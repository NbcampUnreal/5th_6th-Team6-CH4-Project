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
#include "TimerManager.h"
#include "Components/SkeletalMeshComponent.h"

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
    GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
    /* ===== Camera Setup ===== */

    SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
    SpringArm->SetupAttachment(GetRootComponent());
    // SpringArm->TargetArmLength = 300.f;


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


    // RootMotion Dash를 Dedicated Server에서 쓸 거면(권장)
    if (GetMesh())
    {
        GetMesh()->VisibilityBasedAnimTickOption =
            EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
        GetMesh()->bEnableUpdateRateOptimizations = false;
    }
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



void ASquirrel::Jump_ServerAuth()
{

    if (!HasAuthority()) return;

    Jump();

}

void ASquirrel::StopJump_ServerAuth()
{
    if (!HasAuthority()) return;

    StopJumping();
}



void ASquirrel::ApplySprintSpeed()
{
    if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
    {
        MoveComp->MaxWalkSpeed = bIsJog ? SprintSpeed : WalkSpeed;
    }
}

void ASquirrel::SetSprinting_ServerAuth(bool bNewSprinting)
{
    if (!HasAuthority()) return;

    bIsJog = bNewSprinting;
    ApplySprintSpeed();
    ForceNetUpdate();
}

void ASquirrel::OnRep_IsJog()
{
    ApplySprintSpeed();
}



void ASquirrel::Move(const FVector2D& MoveInput)
{
  
    if (MoveInput.IsNearlyZero(0.01f))
    {
        UE_LOG(LogTemp, Warning, TEXT("[Move] INPUT ZERO Auth=%d Controller=%s Vel=%.1f"),
            HasAuthority(), *GetNameSafe(GetController()), GetVelocity().Size());
        return;
    }
    AddMovementInput(GetActorForwardVector(), MoveInput.Y); // 전후(W/S)
    AddMovementInput(GetActorRightVector(), MoveInput.X); // 좌우(A/D)

    UE_LOG(LogTemp, Warning, TEXT("[Move] Auth=%d Controller=%s Vel=%.1f"),
        HasAuthority(),
        *GetNameSafe(GetController()),
        GetVelocity().Size());

}

void ASquirrel::Fire_ServerAuth()
{
    if (!HasAuthority()) return;
    UE_LOG(LogTemp, Warning, TEXT("Fire_ServerAuth()->HasAuthority()"));


    //if (!EquippedGun) return;
    if (!CurrentGun) return;
   
    UE_LOG(LogTemp, Warning, TEXT("Fire_ServerAuth()->EquippedGun"));
    // 너의 기존 방식대로 서버가 가진 조준값으로 AimRot 구성
    const FRotator AimRot(RepViewRot.Pitch, GetActorRotation().Yaw, 0.f);

    // PC 의존 제거 버전
    //EquippedGun->HandleFire(AimRot);

    CurrentGun->HandleFire(AimRot);
    UE_LOG(LogTemp, Warning, TEXT("[Camera] Fire:Fire_ServerAuth()->WeaponFire"));
}

void ASquirrel::RequestDash_ServerAuth()
{
    if (!HasAuthority())
        return;

    const float Now = GetWorld()->GetTimeSeconds();

    // 1) 쿨다운 잠금
    if (Now < NextDashAllowedTime)
        return;

    // 2) 이미 대쉬 중이면 중복 시작 방지(원하면 허용도 가능)
    if (bIsDash)
        return;

    // 3) 대쉬 시작
    bIsDash = true;
    ForceNetUpdate();

    // 4) 5초 잠금 시작
    NextDashAllowedTime = Now + DashCooldownTime;

    // 5) 일정 시간 후 자동 종료(중요: true를 “한 프레임”만 주면 복제에서 놓칠 수 있음)
    GetWorldTimerManager().ClearTimer(DashEndTimerHandle);
    GetWorldTimerManager().SetTimer(
        DashEndTimerHandle,
        this,
        &ASquirrel::EndDash_ServerAuth,
        DashActiveTime,
        false
    );
}

void ASquirrel::OnRep_IsDash()
{
    UE_LOG(LogTemp, Warning, TEXT("[OnRep_IsDash] %s bIsDash=%d"), *GetName(), bIsDash);
}

void ASquirrel::EndDash_ServerAuth()
{
    if (!HasAuthority())
        return;

    bIsDash = false;
    ForceNetUpdate();
}

// (선택) 로그/디버그용
void ASquirrel::OnRep_HP()
{
    // 여기서 HUD 갱신, 피격 UI, 사운드 등을 처리 가능(클라에서 호출됨)
    UE_LOG(LogTemp, Log, TEXT("[OnRep_HP] %s HP=%.1f"), *GetName(), HP);
}

float ASquirrel::TakeDamage(
    float DamageAmount,
    FDamageEvent const& DamageEvent,
    AController* EventInstigator,
    AActor* DamageCauser
)
{
    // 서버에서만 HP를 깎는다(진실은 서버)
    if (!HasAuthority())
    {
        return 0.f;
    }

    if (DamageAmount <= 0.f || HP <= 0.f)
    {
        return 0.f;
    }

    const float Applied = FMath::Min(DamageAmount, HP);
    HP = FMath::Clamp(HP - Applied, 0.f, MaxHP);

    UE_LOG(LogTemp, Warning, TEXT("[Damage] %s took %.1f (HP=%.1f) causer=%s instigator=%s"),
        *GetName(),
        Applied,
        HP,
        *GetNameSafe(DamageCauser),
        *GetNameSafe(EventInstigator)
    );


    // HP 변경을 더 빨리 보내고 싶으면
    ForceNetUpdate();

    if (HP <= 0.f)
    {
        Die(EventInstigator, DamageCauser);
    }

    return Applied;
}

void ASquirrel::Die(AController* Killer, AActor* DamageCauser)
{
    if (!HasAuthority()) return;
    if (bIsDead) return; // 중복 사망 방지

    bIsDead = true;
    ForceNetUpdate();

    // 서버도 즉시 레그돌 적용
    OnRep_IsDead();

    SetLifeSpan(8.f);
}

void ASquirrel::OnRep_IsDead()
{
    if (!bIsDead) return;

    // 이동/충돌 정지(선택)
    if (UCharacterMovementComponent* Move = GetCharacterMovement())
    {
        Move->StopMovementImmediately();
        Move->DisableMovement();
    }

    if (UCapsuleComponent* Capsule = GetCapsuleComponent())
    {
        Capsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }

    // 레그돌
    if (USkeletalMeshComponent* MeshComp = GetMesh())
    {
        MeshComp->SetCollisionProfileName(TEXT("Ragdoll"));
        MeshComp->SetSimulatePhysics(true);
        MeshComp->bBlendPhysics = true; // 권장
    }
}

void ASquirrel::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

   
    DOREPLIFETIME(ASquirrel, RepViewRot);
    DOREPLIFETIME(ASquirrel, HP);   //HP 상태 
    DOREPLIFETIME(ASquirrel, CurrentGun);  //현재 무기 상태 알림
    DOREPLIFETIME(ASquirrel, bIsJog);
    DOREPLIFETIME(ASquirrel, bIsDash);
    DOREPLIFETIME(ASquirrel, bIsDead);

}


void ASquirrel::AttachCurrentGun()
{
    if (!CurrentGun) return;

    USkeletalMeshComponent* MeshComp = GetMesh();
    if (!MeshComp) return;

    // [FIX] 소켓 이름도 한 곳에서만 관리
    const FName HandSocket(TEXT("Hand_R_Socket"));

    if (!MeshComp->DoesSocketExist(HandSocket))
    {
        UE_LOG(LogTemp, Warning, TEXT("[AttachCurrentGun] Socket not found: %s (Mesh=%s)"),
            *HandSocket.ToString(),
            *MeshComp->GetName());
        return;
    }

    // [FIX] 이미 붙어있으면 재부착 스킵(네트워크/RepNotify 중복 호출 방어)
    if (CurrentGun->GetAttachParentActor() == this)
    {
        return;
    }

    CurrentGun->AttachToComponent(
        MeshComp,
        FAttachmentTransformRules::SnapToTargetNotIncludingScale,
        HandSocket
    );
}

// [FIX] CurrentGun RepNotify (헤더에 UFUNCTION() void OnRep_CurrentGun(); 필요)
void ASquirrel::OnRep_CurrentGun()
{
    // 클라: CurrentGun 갱신되면 여기서만 Attach
    AttachCurrentGun(); // [FIX]
}

//총기 장착 함수(서버에서만 진실값 세팅)
void ASquirrel::ServerEquipGun_Implementation(AALCGunBase* NewGun)
{
    if (!HasAuthority() || !NewGun)
        return;

    // [FIX] 같은 총이면 끝(중복 픽업 방어)
    if (CurrentGun == NewGun)
    {
        AttachCurrentGun();
        return;
    }

    //기존 총 없애기
    if (CurrentGun)
    {
        CurrentGun->Destroy();
        CurrentGun = nullptr;
        //CurrentGun->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
        // CurrentGun->SetOwner(nullptr);
    }

    CurrentGun = NewGun;
    CurrentGun->SetOwner(this);
    CurrentGun->SetInstigator(this);

    // [FIX] 픽업 즉시 재오버랩/재픽업 방지: 총 쪽 충돌/물리 꺼주기
    if (UPrimitiveComponent* RootPrim = Cast<UPrimitiveComponent>(CurrentGun->GetRootComponent()))
    {
        RootPrim->SetSimulatePhysics(false);
        RootPrim->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }
    // [FIX] 총 액터 전체 충돌을 확실히 끄고 싶으면(루트 말고 다른 컴포넌트가 오버랩 만들 때)
    CurrentGun->SetActorEnableCollision(false);

    // 서버도 즉시 Attach
    AttachCurrentGun();

    // [FIX] (권장) 총이 월드에 놓여있던 순간의 RepMovement가 남아 떨리는 걸 막고 싶으면
    // CurrentGun->SetReplicateMovement(false);

    ForceNetUpdate();

    UE_LOG(LogTemp, Warning, TEXT("[Squirrel] Equipped Gun(CurrentGun): %s"), *GetNameSafe(CurrentGun));
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