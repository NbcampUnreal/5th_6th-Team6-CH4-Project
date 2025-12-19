// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Squirrel.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Net/UnrealNetwork.h"

// Sets default values
ASquirrel::ASquirrel()
{
    PrimaryActorTick.bCanEverTick = true;

    bReplicates = true;

   

    // 이동 방향만 회전에 영향
    GetCharacterMovement()->bOrientRotationToMovement = false;
    GetCharacterMovement()->RotationRate = FRotator(0.f, 720.f, 0.f);

    /* ===== Camera Setup ===== */

    SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
    SpringArm->SetupAttachment(GetRootComponent());
    SpringArm->TargetArmLength = 300.f;


    SpringArm->bUsePawnControlRotation = true;
    SpringArm->bInheritYaw = true;
    SpringArm->bInheritPitch = true;
    SpringArm->bInheritRoll = false;

    Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    Camera->SetupAttachment(SpringArm);
    Camera->bUsePawnControlRotation = false;
   
   
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

    if (!Controller) return;

    const FRotator ActorRot = GetActorRotation();
    const FRotator ControlRot = GetControlRotation();
    const FRotator ControllerRot = Controller->GetControlRotation();
    const FRotator SpringArmRot = SpringArm->GetComponentRotation();

    UE_LOG(LogTemp, Warning,
        TEXT("[Squirrel Tick]"
            "\n  ActorRotation     = %s"
            "\n  Pawn ControlRot   = %s"
            "\n  Controller Rot    = %s"
            "\n  SpringArm Rot     = %s"),
        *ActorRot.ToString(),
        *ControlRot.ToString(),
        *ControllerRot.ToString(),
        *SpringArmRot.ToString()
    );

}

// Called to bind functionality to input
void ASquirrel::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}


void ASquirrel::Move(const FVector2D& MoveInput)
{
    // 컨트롤러가 있어야 방향 계산이 가능

    // Value는 Axis2D로 설정된 IA_Move의 입력값 (WASD)을 담고 있음
// 예) (X=1, Y=0) → 전진 / (X=-1, Y=0) → 후진 / (X=0, Y=1) → 오른쪽 / (X=0, Y=-1) → 왼쪽

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



// Squirrel.cpp
FRotator ASquirrel::GetViewRotation() const
{
    if (GetNetMode() == NM_DedicatedServer)
    {
        return GetActorRotation();
    }

    if (Controller)
    {
        return ReplicatedViewRotation;
    }

    return GetActorRotation();
}

void ASquirrel::GetLifetimeReplicatedProps(
    TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ASquirrel, ReplicatedViewRotation);
}


