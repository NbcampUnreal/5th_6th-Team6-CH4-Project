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
  
    if (MoveInput.IsNearlyZero(0.01f))
        return;

    AddMovementInput(GetActorForwardVector(), MoveInput.Y); // 전후(W/S)
    AddMovementInput(GetActorRightVector(), MoveInput.X); // 좌우(A/D)
}

