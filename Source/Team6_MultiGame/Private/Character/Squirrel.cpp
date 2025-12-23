// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Squirrel.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
//UI
#include "Components/WidgetComponent.h"
#include "UI/UW_HPBar.h"

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


    //UI
    HPWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("HPWidgetComponent"));
    HPWidgetComponent->SetupAttachment(GetMesh());
    HPWidgetComponent->SetWidgetSpace(EWidgetSpace::World);
    HPWidgetComponent->SetDrawSize(FVector2D(160.f, 24.f));
    HPWidgetComponent->SetRelativeLocation(FVector(0.f, 0.f, 200.f));
    HPWidgetComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    CurrentHP = MaxHP;

}

// Called when the game starts or when spawned
void ASquirrel::BeginPlay()
{
	Super::BeginPlay();
	
    CurrentHP = MaxHP;
    UpdateHPUI();

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

void ASquirrel::Look(const FVector2D& value)
{

    // X는 좌우 회전 (Yaw), Y는 상하 회전 (Pitch)
    // 좌우 회전
    AddControllerYawInput(value.X);
    // 상하 회전
    AddControllerPitchInput(value.Y);
}

void ASquirrel::UpdateHPUI()
{
    if (!HPWidgetComponent)
    {
        //로그용 나중에 지워도 상관없음
        UE_LOG(LogTemp, Warning, TEXT("HPWidgetComponent is null"));
        return;
    }

    if (UUW_HPBar* HPBar = Cast<UUW_HPBar>(HPWidgetComponent->GetUserWidgetObject()))
    {
        HPBar->SetHP(CurrentHP, MaxHP);
    }
}
