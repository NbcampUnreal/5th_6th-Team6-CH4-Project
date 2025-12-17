// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/SharedCamera.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Net/UnrealNetwork.h"


// Sets default values
ASharedCamera::ASharedCamera()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	
	bReplicates = true;


	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	RootComponent = SpringArm;
	SpringArm->TargetArmLength = 300.f; // 카메라 거리 (등 뒤에서 멀리)
	SpringArm->SocketOffset = FVector(0.f, 0.f, 50.f); // 추가 오프셋 (위로)
	SpringArm->bEnableCameraLag = true; // 부드러운 따라가기
	SpringArm->CameraLagSpeed = 3.f;

	SpringArm->bUsePawnControlRotation = false;


	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm);

	ReplicatedRotation = FRotator::ZeroRotator;
}


void ASharedCamera::Server_AddLook_Implementation(const FVector2D& LookInput)
{
	ReplicatedRotation.Yaw += LookInput.X;
	ReplicatedRotation.Pitch += LookInput.Y;

	ReplicatedRotation.Pitch =
		FMath::Clamp(ReplicatedRotation.Pitch, -80.f, 80.f);

	OnRep_CameraRotation(); // 서버도 즉시 반영
}

void ASharedCamera::OnRep_CameraRotation()
{
	SpringArm->SetRelativeRotation(ReplicatedRotation);
}

void ASharedCamera::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASharedCamera, ReplicatedRotation);
}