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

    MaxAmmo = 20;   //일반총 기본값
    CurrentAmmo = 0;
}

void AALCGunBase::BeginPlay()
{
    Super::BeginPlay();

    if (HasAuthority())
    {
        CurrentAmmo = MaxAmmo;

        UE_LOG(LogTemp, Warning,
            TEXT("[Gun] BeginPlay Ammo = %d / %d (%s)"),
            CurrentAmmo, MaxAmmo, *GetName());
    }
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

void AALCGunBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AALCGunBase, CurrentAmmo);
}

//서버에서만 실행되는 로직 , 스폰 , 쿨타임 , 속도 등 핵심 
void AALCGunBase::HandleFire(const FRotator& AimRot)
{
    UWorld* World = GetWorld();
    if (!World || !ProjectileClass)
    {
        UE_LOG(LogTemp, Error, TEXT("[Gun] INVALID World or ProjectileClass"));
        return;
    }

    APawn* OwnerPawn = Cast<APawn>(GetOwner());
    if (!OwnerPawn)
    {
        UE_LOG(LogTemp, Error, TEXT("[Gun] No OwnerPawn"));
        return;
    }

    const float Now = World->GetTimeSeconds();
    if (Now - LastFireTime < FireCooldown)
    {
        // 필요하면 로그
        // UE_LOG(LogTemp, Verbose, TEXT("[Gun] On cooldown. Remain=%.2f"), FireCooldown - (Now - LastFireTime));
        return;
    }

    // 탄약 체크
    if (CurrentAmmo <= 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Gun] No ammo left: %s"), *GetName());

        // 탄 다 쓰면 총 자체를 없애서 손에서 사라지게
        Destroy();
        return;
    }

    // 총구 위치/회전 가져오기 (카메라 AimRot 기준으로 보정)
    FVector MuzzleLoc;
    FRotator MuzzleRot;
    bool bUsedSocket = false;
    GetMuzzleTransform(AimRot, MuzzleLoc, MuzzleRot, bUsedSocket);

    // 실제 발사 방향 (카메라 조준 기준으로)
    const FVector ShootDir = MuzzleRot.Vector();   // 또는 AimRot.Vector();

    // 스폰 위치는 무조건 총구
    const FVector SpawnLoc = MuzzleLoc;

    FActorSpawnParameters Params;
    Params.Owner = OwnerPawn;
    Params.Instigator = OwnerPawn;
    Params.SpawnCollisionHandlingOverride =
        ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    // 발사체를 총구에서, 조준 방향으로 스폰
    AALCProjectileBase* Proj = World->SpawnActor<AALCProjectileBase>(
        ProjectileClass,
        SpawnLoc,
        ShootDir.Rotation(),   // 또는 MuzzleRot
        Params
    );

    if (!Proj)
    {
        UE_LOG(LogTemp, Error,
            TEXT("[Gun] Spawn projectile FAILED. ProjClass=%s Owner=%s"),
            *GetNameSafe(ProjectileClass),
            *GetNameSafe(OwnerPawn));
        return;
    }

    // 발사체에 데미지/방향 넘기기
    Proj->Init(Damage, ShootDir, BulletSpeed);

    if (BulletSpeed > 0.f && MaxRange > 0.f)
    {
        const float LifeTime = MaxRange / BulletSpeed;
        Proj->SetLifeSpan(LifeTime);
    }

    // 탄 1발 소비 + 마지막 발사 시간 갱신
    CurrentAmmo--;
    LastFireTime = Now;

    UE_LOG(LogTemp, Warning,
        TEXT("[Gun] Fired. Ammo=%d/%d Cooldown=%.2f"),
        CurrentAmmo, MaxAmmo, FireCooldown);

    if (CurrentAmmo <= 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Gun] Ammo depleted -> Destroy %s"), *GetName());
        Destroy();
    }
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


