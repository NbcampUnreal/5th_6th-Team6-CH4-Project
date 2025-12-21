// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Controller/MainPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "Net/UnrealNetwork.h"

#include "Character/Squirrel.h"
#include "Character/Controller/SquirrelAIController.h"

AMainPlayerController::AMainPlayerController()
{
	bReplicates = true;
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
		//////////////////////////////////////////////////// TitlePlayerController ////////////////////////////////////////////////////
		FInputModeGameOnly InputMode;
		SetInputMode(InputMode);
		bShowMouseCursor = false;
		//////////////////////////////////////////////////// TitlePlayerController ////////////////////////////////////////////////////

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
	UE_LOG(LogTemp, Warning,
		TEXT("[OnMoveTriggered]"));
	Server_SendMove(Value.Get<FVector2D>());
	
}

void AMainPlayerController::OnLookTriggered(const FInputActionValue& Value)
{
	if (PlayerRole != EPlayerRole::Camera)
		return;

	if (!TargetSquirrel)
		return;

	const FVector2D Look = Value.Get<FVector2D>();

	// [REMOVE] 로컬 ControlRotation 누적 (이 방식은 다른 클라 동기화 안 됨)
	// AddYawInput(Look.X);
	// AddPitchInput(-Look.Y);

	// [ADD] 서버로 Look 델타 전송
	Server_SendLook(FVector2D(Look.X, -Look.Y)); // [ADD] (Pitch 부호는 기존 로직 유지)
}

/* ===================== Server RPC ===================== */

void AMainPlayerController::Server_SendMove_Implementation(const FVector2D& MoveInput)
{
	if (TargetSquirrel)
	{
		TargetSquirrel->Move(MoveInput);
	}
}
// [ADD] 서버 RPC 구현
void AMainPlayerController::Server_SendLook_Implementation(const FVector2D& LookInput) // [ADD]
{
	if (TargetSquirrel)
	{
		TargetSquirrel->ApplyLook_ServerAuth(LookInput); // [ADD]
	}
}

/* ===================== Replication ===================== */

void AMainPlayerController::GetLifetimeReplicatedProps(
	TArray<FLifetimeProperty>& OutLifetimeProps
) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMainPlayerController, PlayerRole);
	DOREPLIFETIME(AMainPlayerController, TargetSquirrel);

}