// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Squirrel.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/PlayerController.h"
#include "Character/Controller/MainPlayerController.h"

// Sets default values
ASquirrel::ASquirrel()
{
    PrimaryActorTick.bCanEverTick = true;

  

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
   
    bReplicates = true;
    SetReplicateMovement(true);
}

// [ADD] RepNotify: 모든 클라에서 스프링암 회전 반영
void ASquirrel::OnRep_ViewRot() // [ADD]
{
    if (SpringArm)
    {
        SpringArm->SetRelativeRotation(RepViewRot); // [ADD]
    }
}

// [ADD] 서버 권위로 Look 누적/클램프/적용
void ASquirrel::ApplyLook_ServerAuth(const FVector2D& LookInput) // [ADD]
{
    if (!HasAuthority())
        return;

    // 누적
    RepViewRot.Yaw += LookInput.X; // [ADD]
    RepViewRot.Pitch = FMath::Clamp(RepViewRot.Pitch + LookInput.Y, -80.f, 80.f); // [ADD]

    // 서버 즉시 반영
    if (SpringArm)
    {
        SpringArm->SetRelativeRotation(RepViewRot); // [ADD]
    }

    ForceNetUpdate(); // [ADD] (즉시 전파를 조금 더 촉진)
}

// [ADD] Replication 등록
void ASquirrel::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const // [ADD]
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ASquirrel, RepViewRot); // [ADD]
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
  
    if (!FMath::IsNearlyZero(MoveInput.X))
    {
        // 캐릭터가 바라보는 방향(정면)으로 X축 이동
        AddMovementInput(GetActorForwardVector(), MoveInput.X);
    }

    if (!FMath::IsNearlyZero(MoveInput.Y))
    {
        // 캐릭터의 오른쪽 방향으로 Y축 이동
        AddMovementInput(GetActorRightVector(), MoveInput.Y);
    }
}

