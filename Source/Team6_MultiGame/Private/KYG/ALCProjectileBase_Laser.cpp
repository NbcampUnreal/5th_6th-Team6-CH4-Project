// ALCProjectileBase_Laser.cpp

#include "KYG/ALCProjectileBase_Laser.h"
#include "Kismet/GameplayStatics.h"
#include "KYG/LCDamageable.h"
#include "DrawDebugHelpers.h" //디버그레이저
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"   

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

	//레이저 시작 방향 끝점
	const FVector Start = GetActorLocation();
    const FVector Dir = Direction.GetSafeNormal();	//방향
	const FVector End = Start + Dir * MaxRange;	//최대 거리

	//라인트레잇 세팅
	FCollisionQueryParams Params(SCENE_QUERY_STAT(LaserTrace), false);
	Params.AddIgnoredActor(this);                         // 자기 자신 무시
	if (AActor* OwnerActor = GetOwner())
	{
		Params.AddIgnoredActor(OwnerActor);               // 총 / 캐릭터 등 소유자 무시
	}

	TArray<FHitResult> Hits;
	const bool bHitAny = World->LineTraceMultiByChannel(Hits, Start, End, TraceChannel, Params);


	// 벽막는 오브젝트까지의 최대 거리 계산
    float MaxAllowedDist = MaxRange;

    if (bHitAny)
    {
        for (const FHitResult& HR : Hits)
        {
            if (!HR.bBlockingHit)
            {
                continue;
            }

            const float Dist = (HR.ImpactPoint - Start).Size();

            // 태그로 막는 벽 지정
            if (bWorldStaticBlocksLaser &&
                HR.GetActor() &&
                HR.GetActor()->ActorHasTag(TEXT("LaserBlock")))
            {
                MaxAllowedDist = FMath::Min(MaxAllowedDist, Dist);
            }
            // WorldStatic 이면 막히게
            else if (bWorldStaticBlocksLaser &&
                HR.Component.IsValid() &&
                HR.Component->GetCollisionObjectType() == ECC_WorldStatic)
            {
                MaxAllowedDist = FMath::Min(MaxAllowedDist, Dist);
            }
        }
    }

    const FVector FinalEnd = Start + Dir * MaxAllowedDist;

    // ===== 임시 레이저 디버그 라인 =====
    DrawDebugLine(
        World,
        Start,
        FinalEnd,
        FColor::Red,
        false,
        0.1f,   // 화면에 남는 시간
        0,
        2.f     // 두께
    );

    // ===== 막히기 전에 맞은 데미저블들 처리 =====
    if (bHitAny)
    {
        for (const FHitResult& HR : Hits)
        {
            AActor* Target = HR.GetActor();
            if (!Target)
            {
                continue;
            }

            const float Dist = (HR.ImpactPoint - Start).Size();
            if (Dist > MaxAllowedDist + 1.f)
            {
                // 막히는 지점 뒤에 있는 애들은 무시
                continue;
            }

            if (Target->GetClass()->ImplementsInterface(ULCDamageable::StaticClass()))
            {
                ILCDamageable::Execute_ReceiveDamage(Target, Damage, GetInstigator());
            }
        }
    }
    // 모든 클라에 FX만 보여주기
    MulticastPlayLaserFX(Start, FinalEnd);

    Destroy();
}

void AALCProjectileBase_Laser::MulticastPlayLaserFX_Implementation(const FVector& Start, const FVector& End)
{
    // 전용 서버 월드에서는 이펙트 안 뿌림
    if (GetNetMode() == NM_DedicatedServer) return;
    if (!LaserVFX) return;

    UWorld* World = GetWorld();
    if (!World) return;

    const FVector Dir = End - Start;
    const FRotator Rot = Dir.Rotation();
    const float Length = Dir.Size();

    UNiagaraComponent* Comp =
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            World,
            LaserVFX,
            Start,
            Rot
        );

    if (Comp)
    {
        // 나이아가에서 쓸 길이 파라미터
        Comp->SetFloatParameter(TEXT("LaserLength"), Length);
    }
}
