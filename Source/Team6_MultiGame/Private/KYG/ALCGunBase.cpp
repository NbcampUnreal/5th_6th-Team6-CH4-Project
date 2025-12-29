// AALCGunBase.cpp

#include "KYG/ALCGunBase.h"
#include "KYG/ALCProjectileBase.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"

#include "GameFramework/Character.h"

#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"

AALCGunBase::AALCGunBase()
{
	bReplicates = true; //총기 클래스도 네트워크에서 복제 Owner가 들고 있으면 모든 클라에서 보임

	//총기 메쉬
	GunMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GunMesh"));
	RootComponent = GunMesh;

	GunMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GunMesh->SetGenerateOverlapEvents(false);

	//총 기본 설정
	Damage = 35.f;
	BulletSpeed = 3000.f;
	FireCooldown = 1.0f;
	LastFireTime = -999.f;	//게임 시작 시 첫 발을 즉시 쏠 수있도록 발사 가능 상태로 만들기 위한 초기값
}

//사격 함수
//void AALCGunBase::Fire()
//{
//    if (HasAuthority())
//    {
//    }//HandleFire(); 	//서버는 직접 발사 처리
//	else
//	{ ServerFire(); }	//클라이언트는 서버에 발사 RPC 요청
//}

//클라에서 호출했을 경우 서버에서 처리되는 함수
//void AALCGunBase::ServerFire_Implementation()
//{
//	HandleFire();	//서버에서 실제 발사 처리
//}

void AALCGunBase::GetMuzzleTransform(const FRotator& AimRot, FVector& OutLoc, FRotator& OutRot, bool& bOutUsedSocket) const
{
    bOutUsedSocket = false;

    // 기본 폴백: 총 액터 위치 + 조준방향 기준 오프셋
    OutRot = AimRot;
    OutLoc = GetActorLocation() + AimRot.RotateVector(MuzzleFallbackOffset);

    if (!GunMesh)
        return;

    if (!GunMesh->DoesSocketExist(MuzzleSocketName))
        return;

    // StaticMesh 소켓 월드 위치
    OutLoc = GunMesh->GetSocketLocation(MuzzleSocketName);

    // 회전은 조준을 쓰는 게 보통 정답(카메라/조준과 일치)
    // "총구의 로컬 방향"을 정확히 쓰고 싶으면 아래로 바꾸면 됨.
    // OutRot = GunMesh->GetSocketRotation(MuzzleSocketName);

    bOutUsedSocket = true;
}

//서버에서만 실행되는 로직 , 스폰 , 쿨타임 , 속도 등 핵심 
void AALCGunBase::HandleFire(const FRotator& AimRot)
{	

    // 서버에서만 총알 스폰
    if (!HasAuthority())
        return;

    UWorld* World = GetWorld();
    if (!World)
        return;

    if (!ProjectileClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Gun] ProjectileClass is NULL (%s)"), *GetName());
        return;
    }

    APawn* OwnerPawn = Cast<APawn>(GetOwner());
    if (!OwnerPawn)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Gun] OwnerPawn is NULL. Did you SetOwner(Squirrel) on pickup? Gun=%s"), *GetName());
        return;
    }

    // 쿨타임
    const float Now = World->GetTimeSeconds();
    if (Now - LastFireTime < FireCooldown)
        return;
    LastFireTime = Now;

    if (AimRot.ContainsNaN())
        return;

    const FVector ShootDir = AimRot.Vector();
    if (ShootDir.ContainsNaN() || ShootDir.IsNearlyZero())
        return;

    // 스폰 트랜스폼
    FVector SpawnLoc;
    FRotator SpawnRot;
    bool bUsedSocket = false;
    GetMuzzleTransform(AimRot, SpawnLoc, SpawnRot, bUsedSocket);

    // Spawn
    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = OwnerPawn;
    SpawnParams.Instigator = OwnerPawn;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    AALCProjectileBase* Bullet = World->SpawnActor<AALCProjectileBase>(
        ProjectileClass,
        SpawnLoc,
        SpawnRot,
        SpawnParams
    );

    if (Bullet)
    {
        Bullet->Init(Damage, ShootDir, BulletSpeed);

        // (선택) 스폰 직후 오너랑 겹쳐서 즉시 OnHit/Destroy 되는 것 방지
        if (UPrimitiveComponent* RootPrim = Cast<UPrimitiveComponent>(Bullet->GetRootComponent()))
        {
            RootPrim->IgnoreActorWhenMoving(OwnerPawn, true);
        }
    }

    UE_LOG(LogTemp, Warning,
        TEXT("[Gun] Fire UsedSocket=%d Socket=%s SpawnLoc=%s Rot=%s Owner=%s GunMesh=%s StaticMesh=%s"),
        bUsedSocket ? 1 : 0,
        *MuzzleSocketName.ToString(),
        *SpawnLoc.ToString(),
        *SpawnRot.ToString(),
        *GetNameSafe(OwnerPawn),
        *GetNameSafe(GunMesh),
        *GetNameSafe(GunMesh ? GunMesh->GetStaticMesh() : nullptr));
}
   // UWorld* World = GetWorld();
   // if (!World || !ProjectileClass) { return; }

   // APawn* OwnerPawn = Cast<APawn>(GetOwner());
   // if (!OwnerPawn) { return; }

   // APlayerController* PC = Cast<APlayerController>(OwnerPawn->GetController());
   // if (!PC) { return; }

   // //카메라 기준 시점
   // FVector CamLoc;
   // FRotator CamRot;
   // PC->GetPlayerViewPoint(CamLoc, CamRot);

   // //총알 방향 (마우스 보는 방향)
   // FVector ShootDir = CamRot.Vector();

   // //총구 위치 (총에서 나오게)
   //// FVector MuzzleLoc = GetActorLocation() + ShootDir * 50.f;
   //
   ////스폰 위치 캐릭터 앞으로
   // FVector SpawnLoc =
   //     OwnerPawn->GetActorLocation()
   //     + ShootDir * 100.f
   //     + FVector(0, 0, 50.f); // 가슴 높이 보정

   // // 쿨타임
   // float Now = World->GetTimeSeconds();
   // if (Now - LastFireTime < FireCooldown) return;
   // LastFireTime = Now;

   // FActorSpawnParameters SpawnParams;
   // SpawnParams.Owner = OwnerPawn;
   // SpawnParams.Instigator = OwnerPawn;
   // SpawnParams.SpawnCollisionHandlingOverride =
   //     ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

   // AALCProjectileBase* Bullet = World->SpawnActor<AALCProjectileBase>(
   //     ProjectileClass,
   //     SpawnLoc,
   //     CamRot,
   //     SpawnParams
   // );

   // if (Bullet)
   // {
   //     Bullet->Init(Damage, ShootDir, BulletSpeed);
   // }


