// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Controller/MainPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "Net/UnrealNetwork.h"
#include "Character/Squirrel.h"
#include "UI/UIHUD.h"
#include "UI/UW_Result.h"
#include "CharacterGameMode/CharacterGameMode.h"



AMainPlayerController::AMainPlayerController()
{
	bReplicates = true;
}

void AMainPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// 중요: 입력 매핑은 로컬 컨트롤러에서만
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


	UIHUD = CreateWidget<UUIHUD>(this, UIHUDClass);
	if (UIHUD)
	{
		UIHUD->AddToViewport();
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
	
	// ===== Jump =====
	if (IA_Jump)
	{
		EIC->BindAction(IA_Jump, ETriggerEvent::Started, this, &AMainPlayerController::OnJumpStarted);
		EIC->BindAction(IA_Jump, ETriggerEvent::Completed, this, &AMainPlayerController::OnJumpCompleted);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("MainPlayerController: IA_Jump is NULL"));
	}

	// ===== Sprint (Shift) =====
	if (IA_Sprint)
	{
		EIC->BindAction(IA_Sprint, ETriggerEvent::Started, this, &AMainPlayerController::OnSprintStarted);
		EIC->BindAction(IA_Sprint, ETriggerEvent::Completed, this, &AMainPlayerController::OnSprintCompleted);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("MainPlayerController: IA_Sprint is NULL"));
	}

	// ===== Dash =====
	if (IA_Dash)
	{
		EIC->BindAction(IA_Dash, ETriggerEvent::Started, this, &AMainPlayerController::OnDashStarted);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("MainPlayerController: IA_Dash is NULL"));
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

	if (UIHUD)
	{
		UIHUD->ResetAllKeys();
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
		return;

	const FVector2D Move = Value.Get<FVector2D>();
	if (Move.IsNearlyZero())
		return;


	Server_SendMove(Move);

	if (UIHUD)
	{
		UIHUD->SetKeyPressed("W", Move.Y > 0.f);
		UIHUD->SetKeyPressed("S", Move.Y < 0.f);
		UIHUD->SetKeyPressed("D", Move.X > 0.f);
		UIHUD->SetKeyPressed("A", Move.X < 0.f);
	}
}

void AMainPlayerController::OnMoveCompleted(const FInputActionValue&)
{
	if (UIHUD)
	{
		UIHUD->ResetAllKeys();
	}
}

void AMainPlayerController::OnMouseLTriggered(const FInputActionValue&)
{
	if (UIHUD)
	{
		UIHUD->SetKeyPressed("MouseL", true);
	}
}

void AMainPlayerController::OnMouseLCompleted(const FInputActionValue&)
{
	if (UIHUD)
	{
		UIHUD->SetKeyPressed("MouseL", false);
	}
}

void AMainPlayerController::OnMouseRTriggered(const FInputActionValue&)
{
	if (UIHUD)
	{
		UIHUD->SetKeyPressed("MouseR", true);
	}
}

void AMainPlayerController::OnMouseRCompleted(const FInputActionValue&)
{
	if (UIHUD)
	{
		UIHUD->SetKeyPressed("MouseR", false);
	}
}

void AMainPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (!IsLocalController())
		return;

	if (!UIHUD)
		return;

}

void AMainPlayerController::UpdateHUD_HP(float CurHP, float MaxHP)
{
	// 로컬 컨트롤러만 UI 갱신
	if (!IsLocalController())
		return;

	if (UIHUD)
	{
		UIHUD->UpdateHP(CurHP, MaxHP);
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

void AMainPlayerController::OnJumpStarted(const FInputActionValue& Value)
{
	if (PlayerRole != EPlayerRole::Move || !TargetSquirrel)
		return;

	Server_SendJump(true);

}

void AMainPlayerController::OnJumpCompleted(const FInputActionValue& Value)
{
	if (PlayerRole != EPlayerRole::Move || !TargetSquirrel)
		return;

	Server_SendJump(false);

}

void AMainPlayerController::OnSprintStarted(const FInputActionValue& Value)
{
	if (PlayerRole != EPlayerRole::Move || !TargetSquirrel)
		return;

	Server_SendSprint(true);

	
}

void AMainPlayerController::OnSprintCompleted(const FInputActionValue& Value)
{
	if (PlayerRole != EPlayerRole::Move || !TargetSquirrel)
		return;

	Server_SendSprint(false);

}

void AMainPlayerController::OnDashStarted(const FInputActionValue& Value)
{
	if (PlayerRole != EPlayerRole::Move || !TargetSquirrel)
		return;

	// 클라 -> 서버로 대쉬 요청
	Server_SendDash();
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
		UE_LOG(LogTemp, Warning, TEXT("[Camera] Fire:Fire_ServerAuth()"));
	}
}

void AMainPlayerController::Server_SendJump_Implementation(bool bPressed)
{
	if (!TargetSquirrel) return;

	if (bPressed) TargetSquirrel->Jump_ServerAuth();
	else          TargetSquirrel->StopJump_ServerAuth();
}

void AMainPlayerController::Server_SendSprint_Implementation(bool bSprinting)
{
	if (!TargetSquirrel) return;

	TargetSquirrel->SetSprinting_ServerAuth(bSprinting);
}

void AMainPlayerController::Server_SendDash_Implementation()
{
	// 서버에서도 역할 체크
	if (PlayerRole != EPlayerRole::Move || !TargetSquirrel)
		return;

	TargetSquirrel->RequestDash_ServerAuth(); // 아래 2)에서 구현
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

/////////////////////////////////////////////////   수정   /////////////////////////////////////////////////
void AMainPlayerController::Client_ShowResult_Implementation(bool bIsRestart, bool bClear)
{
	ShowResult(bIsRestart, bClear);
}

void AMainPlayerController::ShowResult(bool bIsRestart, bool bClear)
{
	if (UIHUD)
	{
		UIHUD->RemoveFromParent();
		UIHUD = nullptr;
	}
	
	if (ResultWidget)
	{
		ResultWidget->RemoveFromParent();
		ResultWidget = nullptr;
	}

	if (ResultWidgetClass)
	{
		UUW_Result* ResultUI = CreateWidget<UUW_Result>(this, ResultWidgetClass);
		if (ResultUI)
		{
			ResultUI->AddToViewport();
			SetInputMode(FInputModeUIOnly());
			ResultUI->SetTitleText(bClear);
			ResultWidget = ResultUI;

			bShowMouseCursor = true;
			bEnableClickEvents = true;
			bEnableMouseOverEvents = true;

			FInputModeGameAndUI Mode;
			Mode.SetWidgetToFocus(ResultUI->TakeWidget());                 // 포커스 지정
			Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);// 마우스 락 해제
			Mode.SetHideCursorDuringCapture(false);                        // 캡처 중 커서 숨김 X
			SetInputMode(Mode);

			SetIgnoreMoveInput(true);
			SetIgnoreLookInput(true);
		}
	}
}

void AMainPlayerController::Server_RequestReturnToLobby_Implementation()
{
	if (ACharacterGameMode* GM = GetWorld()->GetAuthGameMode<ACharacterGameMode>())
	{
		GM->ReturnToLobby();
	}
}
/////////////////////////////////////////////////   수정   /////////////////////////////////////////////////