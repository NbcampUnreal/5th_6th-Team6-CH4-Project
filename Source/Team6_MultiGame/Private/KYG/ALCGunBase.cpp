// AALCGunBase.cpp

#include "KYG/ALCGunBase.h"
#include "KYG/ALCProjectileBase.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"

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

//서버에서만 실행되는 로직 , 스폰 , 쿨타임 , 속도 등 핵심 
void AALCGunBase::HandleFire(const FRotator& AimRot)
{	


    // ★ 서버에서만 총알 스폰
    if (!HasAuthority())
        return;

    UWorld* World = GetWorld();
    if (!World || !ProjectileClass)
        return;

    APawn* OwnerPawn = Cast<APawn>(GetOwner());
    if (!OwnerPawn)
        return;

    // 쿨타임
    const float Now = World->GetTimeSeconds();
    if (Now - LastFireTime < FireCooldown)
        return;
    LastFireTime = Now;

    // AimRot 기반 방향
    const FVector ShootDir = AimRot.Vector();

    // 스폰 위치(기존 로직 유지)
    const FVector SpawnLoc =
        OwnerPawn->GetActorLocation()
        + ShootDir * 100.f
        + FVector(0, 0, 50.f);

    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = OwnerPawn;
    SpawnParams.Instigator = OwnerPawn;
    SpawnParams.SpawnCollisionHandlingOverride =
        ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    AALCProjectileBase* Bullet = World->SpawnActor<AALCProjectileBase>(
        ProjectileClass,
        SpawnLoc,
        AimRot,
        SpawnParams
    );

    if (Bullet)
    {
        Bullet->Init(Damage, ShootDir, BulletSpeed);
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
}
