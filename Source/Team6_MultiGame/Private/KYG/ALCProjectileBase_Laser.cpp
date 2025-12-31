// ALCProjectileBase_Laser.cpp

#include "KYG/ALCProjectileBase_Laser.h"
#include "Kismet/GameplayStatics.h"
//#include "KYG/LCDamageable.h"
#include "DrawDebugHelpers.h" //디버그레이저
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"   
#include "GameFramework/ProjectileMovementComponent.h" 
#include "Components/SphereComponent.h"       

AALCProjectileBase_Laser::AALCProjectileBase_Laser()
{
    if (MovementComp)
    {//날아가는 발사체가 아니라서 이동은 쓰지 않게 설정.
        MovementComp->InitialSpeed = 0.f;
        MovementComp->MaxSpeed = 0.f;
        MovementComp->ProjectileGravityScale = 0.f;
        MovementComp->bAutoActivate = false;
    }
    //레이저는 오버렙충돌을 안 써도 됨.
    if (CollisionComp)
    {
        CollisionComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }
}

void AALCProjectileBase_Laser::Init(float InDamage, FVector Direction, float Speed)
{
    //대미지 설정, 오너, 사용자 무시 설정.
    Super::Init(InDamage, Direction, 0.f);

	Damage = InDamage;

	// 레이저는 서버에서만 판정
	if (!HasAuthority())
	{
		//Destroy();
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
            if (!HR.bBlockingHit) { continue; }

            const float Dist = (HR.ImpactPoint - Start).Size();

            //태그로 막는 오브젝트
            if (bWorldStaticBlocksLaser &&
                HR.GetActor() &&
                HR.GetActor()->ActorHasTag(TEXT("LaserBlock")))
            {
                MaxAllowedDist = FMath::Min(MaxAllowedDist, Dist);
            }
            //월드 스태틱 충돌도 벽으로 취급
            else if (bWorldStaticBlocksLaser &&
                HR.Component.IsValid() &&
                HR.Component->GetCollisionObjectType() == ECC_WorldStatic)
            {
                MaxAllowedDist = FMath::Min(MaxAllowedDist, Dist);
            }
        }
    }

    const FVector FinalEnd = Start + Dir * MaxAllowedDist;

    //  임시 레이저 디버그 라인 서버만
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

    // 막히기 전에 맞은 데미저블들 처리 
    if (bHitAny)
    {
        for (const FHitResult& HR : Hits)
        {
            AActor* Target = HR.GetActor();
            if (!Target || Target == this)
            {continue;}

            const float Dist = (HR.ImpactPoint - Start).Size();
            if (Dist > MaxAllowedDist + 1.f)
            {
                // 레이저가 막힌 지점 뒤에 있는 애들은 무시
                continue;
            }

            // ★ 여기서 엔진 Damage 시스템 사용
            UGameplayStatics::ApplyDamage(
                Target,
                Damage,
                GetInstigator() ? GetInstigator()->GetController() : nullptr,
                this,                         // DamageCauser = 이 레이저 액터
                UDamageType::StaticClass()
            );
        }
    }
    // 모든 클라에 FX만 보여주기
    MulticastPlayLaserFX(Start, FinalEnd);

    //0.2초후 자동 제거
    SetLifeSpan(0.2f);
    //Destroy();
}

void AALCProjectileBase_Laser::MulticastPlayLaserFX_Implementation(const FVector& Start, const FVector& End)
{
    //UE_LOG(LogTemp, Warning,
    //    TEXT("[LaserFX] MulticastPlayLaserFX Start=%s End=%s NetMode=%d"),
    //    *Start.ToString(),
    //    *End.ToString(),
    //    (int32)GetNetMode());

    // 전용 서버 월드에서는 이펙트 안 뿌림
    if (GetNetMode() == NM_DedicatedServer)
    {
        UE_LOG(LogTemp, Warning, TEXT("[LaserFX] DedicatedServer → FX 생략"));
        return;
    }
    if (!LaserVFX)
    {
        UE_LOG(LogTemp, Error, TEXT("[LaserFX] LaserVFX is NULL!"));
        return;
    }

    UWorld* World = GetWorld();
    if (!World) 
    { 
        UE_LOG(LogTemp, Error, TEXT("[LaserFX] World is NULL!"));
        return;
    }

    // 시작 위치 확인용 디버그 스피어
    //DrawDebugSphere(
    //    World,
    //    Start,
    //    20.f,
    //    12,
    //    FColor::Green,
    //    false,
    //    1.5f
    //);

    const FVector Dir = End - Start;
    const FRotator Rot = Dir.Rotation();
    const float Length = Dir.Size();

    UNiagaraComponent* Comp =
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            World,
            LaserVFX,
            Start,
            Rot,
            FVector(1.f, 1.f, 1.f),
            true,
            true
        );

    if (Comp)
    {
        // 나이아가에서 쓸 길이 파라미터
        Comp->SetFloatParameter(TEXT("LaserLength"), Length);

        //시스템이  끝나면 자동 삭제
        Comp->SetAutoDestroy(true);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("[LaserFX] SpawnSystemAtLocation returned NULL"));
    }
}
