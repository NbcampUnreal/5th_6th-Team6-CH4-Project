// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputActionValue.h"
#include "MainPlayerController.generated.h"

class ASquirrel;
class UInputMappingContext;
class UInputAction;
class ASquirrelAIController;
class UUIHUD;

UENUM(BlueprintType)
enum class EPlayerRole : uint8
{
	Move ,
	Camera  
};

UCLASS()
class TEAM6_MULTIGAME_API AMainPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AMainPlayerController();

	/* ===== Server Only ===== */
	void SetRole(EPlayerRole NewRole);
	void SetTargetSquirrel(ASquirrel* InSquirrel);

	UPROPERTY(ReplicatedUsing = OnRep_PlayerRole)
	EPlayerRole PlayerRole = EPlayerRole::Move;
protected:
	/* ===================== Lifecycle ===================== */
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

	/* ===================== Role ===================== */


	UFUNCTION()
	void OnRep_PlayerRole();

	void ApplyPlayerRole();
	FTimerHandle ApplyRoleTimerHandle;

	/* ===================== Target ===================== */
	UPROPERTY(ReplicatedUsing = OnRep_TargetSquirrel)
	ASquirrel* TargetSquirrel;

	UFUNCTION()
	void OnRep_TargetSquirrel();

	/* ===================== Input ===================== */
	void OnMoveTriggered(const FInputActionValue& Value);
	void OnLookTriggered(const FInputActionValue& Value);
	void OnFireStarted(const FInputActionValue& Value);
	// ===== Handlers =====
	void OnJumpStarted(const FInputActionValue& Value);
	void OnJumpCompleted(const FInputActionValue& Value);

	void OnSprintStarted(const FInputActionValue& Value);
	void OnSprintCompleted(const FInputActionValue& Value);

	// 입력 콜백
	void OnDashStarted(const FInputActionValue& Value);
	/* ===================== Server RPC ===================== */
	UFUNCTION(Server, Reliable)
	void Server_SendMove(const FVector2D& MoveInput);

	UFUNCTION(Server, Reliable)                 // [ADD]
	void Server_SendLook(const FVector2D& LookInput); // [ADD]

	UFUNCTION(Server, Reliable)
	void Server_SendFire();

	UFUNCTION(Server, Reliable)
	void Server_SendJump(bool bPressed);

	UFUNCTION(Server, Reliable)
	void Server_SendSprint(bool bSprinting);

	UFUNCTION(Server, Reliable)
	void Server_SendDash();


public:
	/* ===================== Input Assets ===================== */
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputMappingContext* InputMappingContext;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* IA_Move;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* IA_Look;

	UPROPERTY(EditDefaultsOnly, Category = "Input")

	UInputAction* IA_MouseL;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* IA_MouseR;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* IA_Fire;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* IA_Jump = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* IA_Sprint = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* IA_Dash;
protected:
	/* ===================== Replication ===================== */
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUIHUD> UIHUDClass;

	UPROPERTY()
	TObjectPtr<UUIHUD> UIHUD;

	void OnMoveCompleted(const FInputActionValue&);

	void OnMouseLTriggered(const FInputActionValue&);
	void OnMouseLCompleted(const FInputActionValue&);

	void OnMouseRTriggered(const FInputActionValue&);
	void OnMouseRCompleted(const FInputActionValue&);

	virtual void OnPossess(APawn* InPawn) override;

public:

	void UpdateHUD_HP(float CurHP, float MaxHP);

};