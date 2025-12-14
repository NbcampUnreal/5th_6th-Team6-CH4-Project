// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Squirrel.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"

// Sets default values
ASquirrel::ASquirrel()
{
    PrimaryActorTick.bCanEverTick = true;

    // ★★★★★ 이 두 줄이 문제의 90% ★★★★★
    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = true;   // 캐릭터가 컨트롤러 yaw 따라 돎 (필수)
    bUseControllerRotationRoll = false;

    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(RootComponent);
    CameraBoom->TargetArmLength = 400.f;
    CameraBoom->bUsePawnControlRotation = true;   // ← Pawn 회전 사용
    CameraBoom->bInheritPitch = true;
    CameraBoom->bInheritYaw = true;
    CameraBoom->bInheritRoll = true;

    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
    FollowCamera->bUsePawnControlRotation = false;  // ← 여기 false!

    // ★★★★★ 클라이언트가 카메라 시뮬레이션 하도록 ★★★★★


    bReplicates = true;

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

