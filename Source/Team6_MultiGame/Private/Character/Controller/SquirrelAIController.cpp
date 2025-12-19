// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Controller/SquirrelAIController.h"
#include "Character/Squirrel.h"
#include "Net/UnrealNetwork.h"

ASquirrelAIController::ASquirrelAIController()
{
    bReplicates = false;
	UE_LOG(LogTemp, Log,
		TEXT("[SquirrelAIController] Constructor | %s"),
		*GetName());

}

void ASquirrelAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);
	
	UE_LOG(LogTemp, Warning,
		TEXT("[SquirrelAI][OnPossess] Controller=%s Pawn=%s Authority=%s"),
		*GetName(),
		InPawn ? *InPawn->GetName() : TEXT("NULL"),
		HasAuthority() ? TEXT("Server") : TEXT("Client"));
	// ★ 초기 회전값 동기화
	 //    (AIController의 내부 회전 캐시)
	if (ASquirrel* Squirrel = Cast<ASquirrel>(InPawn))
	{
		const FRotator InitRot = InPawn->GetActorRotation();
		SetControlRotation(InitRot);
		Squirrel->ReplicatedViewRotation = InitRot;
	}
}

void ASquirrelAIController::AddCameraInput(const FVector2D& LookInput)
{
	// ★ 서버 권위 보장
	if (!HasAuthority())
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[SquirrelAI][AddCameraInput] Rejected (Not Authority) | Controller=%s"),
			*GetName());

		return;
	}


	/* ================================
	 * 1. 회전 계산 (AIController 내부)
	 * ================================ */

	FRotator NewRotation = GetControlRotation();


	NewRotation.Yaw += LookInput.X;
	NewRotation.Pitch = FMath::Clamp(
		NewRotation.Pitch + LookInput.Y,
		-80.f,
		80.f
	);

	UE_LOG(LogTemp, Log,
		TEXT("[SquirrelAI][AddCameraInput] LookInput=(X=%.2f, Y=%.2f)"),
		LookInput.X, LookInput.Y);

	UE_LOG(LogTemp, Warning,
		TEXT("[SquirrelAI][AddCameraInput] New ControlRotation = %s"),
		*NewRotation.ToString());
	// ★ ControlRotation은 "계산 캐시"로만 유지
	SetControlRotation(NewRotation);

	if (ASquirrel* Squirrel = Cast<ASquirrel>(GetPawn()))
	{
		Squirrel->ReplicatedViewRotation = NewRotation;
	}
}
