// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Controller/MainPlayerController.h"

#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "Net/UnrealNetwork.h"

#include "Character/Squirrel.h"

#include "UI/UW_KeyGuide.h"


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

	if (IsLocalController() && KeyGuideClass)
	{
		KeyGuideWidget = CreateWidget<UUW_KeyGuide>(this, KeyGuideClass);
		if (KeyGuideWidget)
		{
			KeyGuideWidget->AddToViewport();
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

	// BindAction 결과 로그는 너무 잦을 수 있어, 실패 케이스만 경고
	if (IA_Move)
	{
		EIC->BindAction(IA_Move, ETriggerEvent::Triggered, this, &AMainPlayerController::OnMoveTriggered);
		EIC->BindAction(IA_Move, ETriggerEvent::Completed, this, &AMainPlayerController::OnMoveCompleted);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("MainPlayerController: IA_Move is NULL"));
	}

	if (IA_Look)
	{
		EIC->BindAction(IA_Look, ETriggerEvent::Triggered, this, &AMainPlayerController::OnLookTriggered);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("MainPlayerController: IA_Look is NULL"));
	}


	if (IA_MouseL)
	{
		EIC->BindAction(IA_MouseL, ETriggerEvent::Triggered, this, &AMainPlayerController::OnMouseLTriggered);
		EIC->BindAction(IA_MouseL, ETriggerEvent::Completed, this, &AMainPlayerController::OnMouseLCompleted);
	}

	if (IA_MouseR)
	{
		EIC->BindAction(IA_MouseR, ETriggerEvent::Triggered, this, &AMainPlayerController::OnMouseRTriggered);
		EIC->BindAction(IA_MouseR, ETriggerEvent::Completed, this, &AMainPlayerController::OnMouseRCompleted);
	}

	if (IA_Fire)
	{
		EIC->BindAction(IA_Fire, ETriggerEvent::Started, this, &AMainPlayerController::OnFireStarted);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("MainPlayerController: IA_Fire is NULL"));
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

	if (KeyGuideWidget)
	{
		KeyGuideWidget->ResetAllKeys();
	}
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

	
	// 동일 ViewTarget이면 중복 호출 방지
	if (GetViewTarget() != TargetSquirrel)
	{
		SetViewTargetWithBlend(TargetSquirrel, 0.f, EViewTargetBlendFunction::VTBlend_Linear);
	}

	UE_LOG(LogTemp, Warning,
		TEXT("[ApplyRole] ViewTarget set to %s | Role=%s"),
		*TargetSquirrel->GetName(),
		PlayerRole == EPlayerRole::Camera ? TEXT("Camera") : TEXT("Move"));
}


/* ===================== Input ===================== */

void AMainPlayerController::OnMoveTriggered(const FInputActionValue& Value)
{
	if (PlayerRole != EPlayerRole::Move || !TargetSquirrel)
	{
		return;
	}

	const FVector2D Move = Value.Get<FVector2D>();

	if (Move.IsNearlyZero(0.01f))
		return;

	
	Server_SendMove(Move);

	
	if (KeyGuideWidget)
	{
		KeyGuideWidget->SetKeyPressed("W", Move.X > 0.f);
		KeyGuideWidget->SetKeyPressed("S", Move.X < 0.f);
		KeyGuideWidget->SetKeyPressed("D", Move.Y > 0.f);
		KeyGuideWidget->SetKeyPressed("A", Move.Y < 0.f);
	}
}

void AMainPlayerController::OnMoveCompleted(const FInputActionValue& Value)
{
	if (KeyGuideWidget)
	{
		KeyGuideWidget->ResetAllKeys();
	}
}

void AMainPlayerController::OnMouseLTriggered(const FInputActionValue& Value)
{
	if (KeyGuideWidget)
	{
		KeyGuideWidget->SetKeyPressed("MouseL", true);
	}
}

void AMainPlayerController::OnMouseLCompleted(const FInputActionValue& Value)
{
	if (KeyGuideWidget)
	{
		KeyGuideWidget->SetKeyPressed("MouseL", false);
	}
}

void AMainPlayerController::OnMouseRTriggered(const FInputActionValue& Value)
{
	if (KeyGuideWidget)
	{
		KeyGuideWidget->SetKeyPressed("MouseR", true);
	}
}

void AMainPlayerController::OnMouseRCompleted(const FInputActionValue& Value)
{
	if (KeyGuideWidget)
	{
		KeyGuideWidget->SetKeyPressed("MouseR", false);
	}
}

void AMainPlayerController::OnLookTriggered(const FInputActionValue& Value)
{
	if (PlayerRole != EPlayerRole::Camera)
		return;

	if (!TargetSquirrel)
		return;

	FVector2D Look = Value.Get<FVector2D>();

	// [FIX] 데드존
	if (Look.IsNearlyZero(0.01f))
		return;

	// [NOTE] Pitch 부호 유지 (기존 로직)
	Look.Y = -Look.Y;

	// [ADD] 서버로 Look 델타 전송
	Server_SendLook(Look);
}

void AMainPlayerController::OnFireStarted(const FInputActionValue& Value)
{
	if (PlayerRole != EPlayerRole::Camera)
		return;

	if (!TargetSquirrel)
		return;

	Server_SendFire();
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


void AMainPlayerController::Server_SendFire_Implementation()
{
	// 서버에서도 역할 체크(치트/실수 방지)
	if (PlayerRole != EPlayerRole::Camera)
		return;

	if (TargetSquirrel)
	{
		TargetSquirrel->Fire_ServerAuth();
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