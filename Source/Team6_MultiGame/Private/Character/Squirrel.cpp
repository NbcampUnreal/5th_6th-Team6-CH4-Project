// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Squirrel.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/PlayerController.h"
#include "Character/Controller/MainPlayerController.h"
#include "KYG/ALCGunBase.h"
#include "Components/CapsuleComponent.h"

// Sets default values
ASquirrel::ASquirrel()
{
    PrimaryActorTick.bCanEverTick = true;

    bReplicates = true;
    SetReplicateMovement(true);
    // ★ 서버에서 클라로 더 자주 보내게
    NetUpdateFrequency = 100.f;        // 기본보다 크게 (예: 100)
    MinNetUpdateFrequency = 30.f;      // 최소 보장 (예: 30)
    NetPriority = 3.f;                // 우선순위 상승

    // 협동 소규모 게임이면 켜도 됨(멀리 있어도 항상 relevant)
    bAlwaysRelevant = true;

    // (선택) Dormancy 쓰지 않도록
    NetDormancy = DORM_Awake;

    // 이동 방향만 회전에 영향
    GetCharacterMovement()->bOrientRotationToMovement = false;
    GetCharacterMovement()->RotationRate = FRotator(0.f, 720.f, 0.f);

    // ★ 혹시 0으로 초기화돼 있으면 이동 절대 안 됨
    GetCharacterMovement()->MaxWalkSpeed = 600.f;
    /* ===== Camera Setup ===== */

    SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
    SpringArm->SetupAttachment(GetRootComponent());
    SpringArm->TargetArmLength = 300.f;


    SpringArm->bUsePawnControlRotation = false;
    SpringArm->bInheritYaw = false;
    SpringArm->bInheritPitch = false;
    SpringArm->bInheritRoll = false;

    Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    Camera->SetupAttachment(SpringArm);
    Camera->bUsePawnControlRotation = false;
   

    //HP추가
    HP = MaxHP;

    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    GetCapsuleComponent()->SetCollisionObjectType(ECC_Pawn);
    GetCapsuleComponent()->SetCollisionResponseToAllChannels(ECR_Block);
    GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
    GetCapsuleComponent()->SetGenerateOverlapEvents(true);
}

// [ADD] RepNotify: 모든 클라에서 스프링암 회전 반영
void ASquirrel::OnRep_ViewRot() // [ADD]
{
    if (SpringArm)
    {
        SpringArm->SetUsingAbsoluteRotation(true);
        SpringArm->SetWorldRotation(FRotator(RepViewRot.Pitch, GetActorRotation().Yaw, 0.f));
    }
}



// [ADD] 서버 권위로 Look 누적/클램프/적용
void ASquirrel::ApplyLook_ServerAuth(const FVector2D& LookInput) // [ADD]
{
    if (!HasAuthority())
        return;

    // 1) 몸(Yaw) 회전
    const float NewYaw = FMath::UnwindDegrees(GetActorRotation().Yaw + LookInput.X);
    SetActorRotation(FRotator(0.f, NewYaw, 0.f));

    // 2) 카메라 Pitch 누적
    RepViewRot.Pitch = FMath::Clamp(RepViewRot.Pitch + LookInput.Y, -80.f, 80.f);

    // 3) 카메라도 "같이" 돌리기: SpringArm을 월드 회전으로 직접 세팅
    if (SpringArm)
    {
        // AbsoluteRotation이 켜져 있든 말든 결과가 나오게 강제
        SpringArm->SetUsingAbsoluteRotation(true);
        SpringArm->SetWorldRotation(FRotator(RepViewRot.Pitch, NewYaw, 0.f));
    }

    // ★ 너무 자주하면 네트워크 폭증하니 30Hz 정도로 제한
    static float LastForceTime = 0.f;
    const float Now = GetWorld()->TimeSeconds;
    if (Now - LastForceTime >= (1.f / 30.f))
    {
        ForceNetUpdate();
        LastForceTime = Now;
    }
}


// Called when the game starts or when spawned
void ASquirrel::BeginPlay()
{
	Super::BeginPlay();


}

// Called every frame
void ASquirrel::Tick(float DeltaTime)
{

    Super::Tick(DeltaTime);

   
}

// Called to bind functionality to input
void ASquirrel::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}


void ASquirrel::Move(const FVector2D& MoveInput)
{
  
    if (MoveInput.IsNearlyZero(0.01f))
        return;

    AddMovementInput(GetActorForwardVector(), MoveInput.Y); // 전후(W/S)
    AddMovementInput(GetActorRightVector(), MoveInput.X); // 좌우(A/D)
}

void ASquirrel::Fire_ServerAuth()
{
    if (!HasAuthority()) return;
    if (!EquippedGun) return;

    // 너의 기존 방식대로 서버가 가진 조준값으로 AimRot 구성
    const FRotator AimRot(RepViewRot.Pitch, GetActorRotation().Yaw, 0.f);

    // PC 의존 제거 버전
    EquippedGun->HandleFire(AimRot);
}

void ASquirrel::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ASquirrel, EquippedGun);
    DOREPLIFETIME(ASquirrel, RepViewRot);
    DOREPLIFETIME(ASquirrel, HP);   //HP 상태 
    DOREPLIFETIME(ASquirrel, CurrentGun);  //현재 무기 상태 알림
}

void ASquirrel::EquipGun_ServerAuth(TSubclassOf<AALCGunBase> NewGunClass)
{
    if (!HasAuthority()) return;
    if (!NewGunClass) return;

    // 기존 총 정리
    UnequipGun_ServerAuth();

    FActorSpawnParameters Params;
    Params.Owner = this;
    Params.Instigator = this;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    EquippedGun = GetWorld()->SpawnActor<AALCGunBase>(NewGunClass, Params);
    if (EquippedGun)
    {
        // 서버에서도 즉시 부착(서버는 판정/디버그에 필요)
        AttachEquippedGun();

        // 복제 갱신 빠르게
        ForceNetUpdate();
    }
}

void ASquirrel::UnequipGun_ServerAuth()
{
    if (!HasAuthority()) return;

    if (EquippedGun)
    {
        EquippedGun->Destroy();
        EquippedGun = nullptr;
        ForceNetUpdate();
    }
}

void ASquirrel::OnRep_EquippedGun()
{
    // 클라: 장착 총이 갱신되면 부착만 처리
    AttachEquippedGun();
}

void ASquirrel::AttachEquippedGun()
{
    if (!EquippedGun) return;

    USkeletalMeshComponent* MeshComp = GetMesh();
    if (!MeshComp) return;

    if (!MeshComp->DoesSocketExist(WeaponSocketName))
    {
        UE_LOG(LogTemp, Warning, TEXT("[AttachEquippedGun] Socket not found: %s (Mesh=%s)"),
            *WeaponSocketName.ToString(),
            *MeshComp->GetName());
        return;
    }

    EquippedGun->AttachToComponent(
        MeshComp,
        FAttachmentTransformRules::SnapToTargetNotIncludingScale,
        WeaponSocketName
    );
}

//회복 관련 
void ASquirrel::ReceiveHeal_Implementation(float HealAmount)
{
    if (!HasAuthority())
    { return; }

    HP = FMath::Clamp(HP + HealAmount, 0.f, MaxHP);

    UE_LOG(LogTemp, Warning, TEXT("[Squirrel] Healed by %.1f, HP=%.1f"), HealAmount, HP);

    // 여기서 나주에 HUD 업데이트용 멀티캐스트 RPC, 또는 HP를 바인딩한 UMG 등이 있으면 자동으로 반영시킬 수 있음
}

//총기 장착 함수
void ASquirrel::ServerEquipGun_Implementation(AALCGunBase* NewGun)
{
    if (!HasAuthority() || !NewGun)
        return;

    // 기존 총 있으면 제거
    if (CurrentGun && CurrentGun != NewGun)
    {
        CurrentGun->Destroy();
    }

    CurrentGun = NewGun;

    // 캐릭터 메시에 붙이기 (손 소켓 이름은 본인 캐릭터에 맞게)
    if (USkeletalMeshComponent* MeshComp = GetMesh())
    {
        CurrentGun->AttachToComponent(
            MeshComp,
            FAttachmentTransformRules::SnapToTargetNotIncludingScale,
            TEXT("Hand_R_Socket")
        );
    }

    UE_LOG(LogTemp, Warning, TEXT("[Squirrel] Equipped Gun: %s"), *GetNameSafe(CurrentGun));
}