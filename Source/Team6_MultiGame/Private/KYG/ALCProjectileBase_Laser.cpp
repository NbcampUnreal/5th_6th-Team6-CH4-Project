// ALCProjectileBase_Laser.cpp

#include "KYG/ALCProjectileBase_Laser.h"
#include "Kismet/GameplayStatics.h"
#include "KYG/LCDamageable.h"
#include "DrawDebugHelpers.h" //디버그레이저

void AALCProjectileBase_Laser::Init(float InDamage, FVector Direction, float Speed)
{
	Damage = InDamage;

	// 레이저는 서버에서만 판정
	if (!HasAuthority())
	{
		Destroy();
		return;
	}

	UWorld* World = GetWorld();
	if (!World) { Destroy(); return; }

	const FVector Start = GetActorLocation();
	const FVector End = Start + Direction.GetSafeNormal() * MaxRange;

	FCollisionQueryParams Params(SCENE_QUERY_STAT(LaserTrace), false);
	Params.AddIgnoredActor(this);
	if (GetOwner()) Params.AddIgnoredActor(GetOwner());

	TArray<FHitResult> Hits;
	const bool bHitAny = World->LineTraceMultiByChannel(Hits, Start, End, TraceChannel, Params);

	if (!bHitAny)
	{
		// 그냥 최대 사거리까지 레이저 선만 그리기
		DrawDebugLine(
			World,
			Start,
			End,
			FColor::Red,
			false,
			0.1f, // 화면에 남아있는 시간(초)
			0,
			2.f   // 두께
		);
		Destroy();
		return;
	}

	// 1) 벽막는 오브젝트까지의 최대 거리 계산
	float MaxAllowedDist = MaxRange;

	for (const FHitResult& HR : Hits)
	{
		if (!HR.bBlockingHit) continue;

		if (bWorldStaticBlocksLaser && HR.GetActor() && HR.GetActor()->ActorHasTag(TEXT("LaserBlock")))
		{
			// 태그 기반으로 막는 벽을 지정하고 싶을때
			MaxAllowedDist = FMath::Min(MaxAllowedDist, (HR.ImpactPoint - Start).Size());
		}
		else if (bWorldStaticBlocksLaser && HR.Component.IsValid() && HR.Component->GetCollisionObjectType() == ECC_WorldStatic)
		{
			// WorldStatic이면 막힘 처리
			MaxAllowedDist = FMath::Min(MaxAllowedDist, (HR.ImpactPoint - Start).Size());
		}
	}

	const FVector FinalEnd = Start + Direction.GetSafeNormal() * MaxAllowedDist;

	// 임시 레이저 이펙트 (디버그 라인)
	DrawDebugLine(
		World,
		Start,
		FinalEnd,
		FColor::Red,
		false,
		0.1f, // 0.05~0.2 정도가 무난
		0,
		2.f
	);

	// 막히기 전 구간 안에 있는 Damageable들을 데미지 처리
	for (const FHitResult& HR : Hits)
	{
		AActor* Target = HR.GetActor();
		if (!Target) continue;

		const float Dist = (HR.ImpactPoint - Start).Size();
		if (Dist > MaxAllowedDist + 1.f) continue;

		if (Target->GetClass()->ImplementsInterface(ULCDamageable::StaticClass()))
		{
			ILCDamageable::Execute_ReceiveDamage(Target, Damage, GetInstigator());
		}
	}

	Destroy();
}
