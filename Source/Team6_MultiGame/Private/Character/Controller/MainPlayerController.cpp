// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Controller/MainPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "Net/UnrealNetwork.h"

#include "Character/SharedCamera.h"
#include "Character/Squirrel.h"

AMainPlayerController::AMainPlayerController()
{
	bReplicates = true;
}

void AMainPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// 입력 매핑은 로컬에서만
	if (IsLocalController())
	{
		if (ULocalPlayer* LP = GetLocalPlayer())
		{
			if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
				ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LP))
			{
				Subsystem->ClearAllMappings();
				if (IMC)
				{
					Subsystem->AddMappingContext(IMC, 0);
				}
			}
		}
	}
}

void AMainPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(InputComponent))
	{
		if (IA_Move)
		{
			EIC->BindAction(
				IA_Move,
				ETriggerEvent::Triggered,
				this,
				&AMainPlayerController::OnMoveTriggered
			);
		}

		if (IA_Look)
		{
			EIC->BindAction(
				IA_Look,
				ETriggerEvent::Triggered,
				this,
				&AMainPlayerController::OnTurnTriggered
			);
		}
	}
}

/* ===================== Role ===================== */

void AMainPlayerController::SetRole(EPlayerRole InRole)
{
	PlayerRole = InRole;
}

/* ===================== Shared Camera ===================== */

void AMainPlayerController::SetSharedCamera(ASharedCamera* InCamera)
{
	// 서버에서만 호출됨
	SharedCamera = InCamera;
}

void AMainPlayerController::OnRep_SharedCamera()
{
	if (IsLocalController() && SharedCamera)
	{
		SetViewTarget(SharedCamera);
	}
}

/* ===================== Target Character ===================== */

void AMainPlayerController::SetTargetSquirrel(ASquirrel* InSquirrel)
{
	TargetSquirrel = InSquirrel;
}

/* ===================== Input Handling ===================== */

void AMainPlayerController::OnMoveTriggered(const FInputActionValue& Value)
{
	if (PlayerRole != EPlayerRole::Move)
		return;

	FVector2D MoveInput = Value.Get<FVector2D>();
	Server_SendMoveInput(MoveInput);
}

void AMainPlayerController::OnTurnTriggered(const FInputActionValue& Value)
{
	if (PlayerRole != EPlayerRole::Camera)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, TEXT("Not Camera Role"));
		return;
	}
	if (!SharedCamera)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, TEXT("SharedCamera NULL!"));
		return;
	}
	FVector2D LookInput = Value.Get<FVector2D>();
	GEngine->AddOnScreenDebugMessage(-1, 0.1f, FColor::Blue, FString::Printf(TEXT("Turn Input: %s"), *LookInput.ToString()));
	Server_TurnCamera(LookInput);
}

/* ===================== Server RPC ===================== */

void AMainPlayerController::Server_SendMoveInput_Implementation(FVector2D MoveInput)
{
	if (!ensure(TargetSquirrel))
	{
		return;
	}

	TargetSquirrel->Move(MoveInput);
}

void AMainPlayerController::Server_TurnCamera_Implementation(FVector2D LookInput)
{
	if (PlayerRole != EPlayerRole::Camera || !SharedCamera)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, TEXT("Server: Invalid Role or No Camera"));
		return;
	}
	SharedCamera->Server_AddLook(LookInput);
}

/* ===================== Replication ===================== */

void AMainPlayerController::GetLifetimeReplicatedProps(
	TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMainPlayerController, PlayerRole);
	DOREPLIFETIME(AMainPlayerController, SharedCamera);
	DOREPLIFETIME(AMainPlayerController, TargetSquirrel);

}