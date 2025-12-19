// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Controller/MainPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "Net/UnrealNetwork.h"

#include "Character/Squirrel.h"
#include "Character/Controller/SquirrelAIController.h"

AMainPlayerController::AMainPlayerController()
{
	bReplicates = false;
}

void AMainPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// ★ 중요: 입력 매핑은 로컬 컨트롤러에서만
	if (!IsLocalController())
	{
		UE_LOG(LogTemp, Warning, TEXT("[BeginPlay] Not local controller, skipping input setup."));
		return;
	}

	else
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[BeginPlay][LOCAL] Controller=%s Role(Init)=%s"),
			*GetName(),
			PlayerRole == EPlayerRole::Camera ? TEXT("Camera") : TEXT("Move"));
	}

	if (ULocalPlayer* LP = GetLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LP))
		{
			Subsystem->ClearAllMappings();

			if (InputMappingContext)
			{
				Subsystem->AddMappingContext(InputMappingContext, 0);
				UE_LOG(LogTemp, Warning, TEXT("MainPlayerController: InputMappingContext is Succeed"));
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("MainPlayerController: InputMappingContext is NULL"));
			}
		}
	}
}

/* ===================== Input Binding ===================== */
void AMainPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// ★ 서버 컨트롤러는 입력 바인딩 안 함
	if (!IsLocalController())
	{
		UE_LOG(LogTemp, Warning, TEXT("Server controller: skipping input binding."));
		return;
	}

	UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(InputComponent);
	if (!EIC)
	{
		UE_LOG(LogTemp, Error, TEXT("MainPlayerController: EnhancedInputComponent missing"));
		return;
	}

	if (IA_Move)
	{
		EIC->BindAction(
			IA_Move,
			ETriggerEvent::Triggered,
			this,
			&AMainPlayerController::OnMoveTriggered
		);
		UE_LOG(LogTemp, Warning, TEXT("MainPlayerController: IA_Move is Succeed"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("MainPlayerController: IA_Move is NULL"));
	}

	if (IA_Look)
	{
		EIC->BindAction(
			IA_Look,
			ETriggerEvent::Triggered,
			this,
			&AMainPlayerController::OnLookTriggered
		);
		UE_LOG(LogTemp, Warning, TEXT("MainPlayerController: IA_Look is Succeed"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("MainPlayerController: IA_Look is NULL"));
	}
}

/* ===================== Role ===================== */
void AMainPlayerController::SetRole(EPlayerRole NewRole)
{
	if (!HasAuthority())
		return;

	PlayerRole = NewRole;

	UE_LOG(LogTemp, Warning,
		TEXT("[Server][SetRole] %s assigned Role=%s"),
		*GetName(),
		PlayerRole == EPlayerRole::Camera ? TEXT("Camera") : TEXT("Move"));
	
	
}
void AMainPlayerController::OnRep_PlayerRole()
{
	if (IsLocalController())
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[Client][OnRep_PlayerRole] %s Role=%s"),
			*GetName(),
			PlayerRole == EPlayerRole::Camera ? TEXT("Camera") : TEXT("Move"));
	}
	ApplyPlayerRole();
}

void AMainPlayerController::SetTargetSquirrel(ASquirrel* InSquirrel)
{
	if (!HasAuthority()) return;

	TargetSquirrel = InSquirrel;

	UE_LOG(LogTemp, Warning,
		TEXT("[Server] TargetSquirrel set: %s"),
		TargetSquirrel ? *TargetSquirrel->GetName() : TEXT("NULL"));

	// ★ 역할이 이미 정해져 있으면 즉시 반영
	ApplyPlayerRole();
}



void AMainPlayerController::OnRep_TargetSquirrel()
{
	ApplyPlayerRole();
}

void AMainPlayerController::ApplyPlayerRole()
{
	if (!IsLocalController())
		return;

	if (!TargetSquirrel)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[ApplyRole] TargetSquirrel not valid yet"));
		return;
	}

	// AIController 체크 제거
	SetViewTargetWithBlend(
		TargetSquirrel,
		0.f,
		EViewTargetBlendFunction::VTBlend_Linear
	);

	UE_LOG(LogTemp, Warning,
		TEXT("[ApplyRole] ViewTarget set to %s | Role=%s"),
		*TargetSquirrel->GetName(),
		PlayerRole == EPlayerRole::Camera ? TEXT("Camera") : TEXT("Move"));
}


/* ===================== Input ===================== */

void AMainPlayerController::OnMoveTriggered(const FInputActionValue& Value)
{
	if (PlayerRole != EPlayerRole::Move)
	{
		return;
	}

	if (!TargetSquirrel)
	{
		UE_LOG(LogTemp, Warning, TEXT("Move input but TargetSquirrel is NULL"));
		return;
	}

	Server_SendMove(Value.Get<FVector2D>());
}

void AMainPlayerController::OnLookTriggered(const FInputActionValue& Value)
{
	if (PlayerRole != EPlayerRole::Camera)
	{
		return;
	}

	if (!TargetSquirrel)
	{
		UE_LOG(LogTemp, Warning, TEXT("Look input but TargetSquirrel is NULL"));
		return;
	}
	UE_LOG(LogTemp, Warning, TEXT("Clinent Mouse Look"));
	Server_SendLook(Value.Get<FVector2D>());
}

/* ===================== Server RPC ===================== */

void AMainPlayerController::Server_SendMove_Implementation(const FVector2D& MoveInput)
{
	if (TargetSquirrel)
	{
		TargetSquirrel->Move(MoveInput);
	}
}

void AMainPlayerController::Server_SendLook_Implementation(const FVector2D& LookInput)
{
	if (!TargetSquirrel)
	{
		UE_LOG(LogTemp, Error,
			TEXT("[Server_SendLook] TargetSquirrel is NULL | PC=%s"),
			*GetName());
		return;
	}

	UE_LOG(LogTemp, Warning,
		TEXT("[Server_SendLook] TargetSquirrel=%s"),
		*TargetSquirrel->GetName());

	if (ASquirrelAIController* AI =
		Cast<ASquirrelAIController>(TargetSquirrel->GetController()))
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[Server_SendLook] AIController=%s"),
			*AI->GetName());

		AI->AddCameraInput(LookInput);
	}
	else
	{
		UE_LOG(LogTemp, Error,
			TEXT("[Server_SendLook] Controller is not SquirrelAIController | Controller=%s"),
			TargetSquirrel->GetController()
			? *TargetSquirrel->GetController()->GetName()
			: TEXT("NULL"));
	}
}

/* ===================== Replication ===================== */

void AMainPlayerController::GetLifetimeReplicatedProps(
	TArray<FLifetimeProperty>& OutLifetimeProps
) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMainPlayerController, PlayerRole);
	//DOREPLIFETIME(AMainPlayerController, TargetSquirrel);

}