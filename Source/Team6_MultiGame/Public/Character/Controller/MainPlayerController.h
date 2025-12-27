// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputActionValue.h"
#include "MainPlayerController.generated.h"

class UUW_KeyGuide;
/**
 * 
 */

class ASquirrel;
class UInputMappingContext;
class UInputAction;
class ASquirrelAIController;

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

protected:
	/* ===================== Lifecycle ===================== */
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

	/* ===================== Role ===================== */
	UPROPERTY(ReplicatedUsing = OnRep_PlayerRole)
	EPlayerRole PlayerRole = EPlayerRole::Move;

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

	/* ===================== Server RPC ===================== */
	UFUNCTION(Server, Reliable)
	void Server_SendMove(const FVector2D& MoveInput);

	UFUNCTION(Server, Reliable)                 // [ADD]
	void Server_SendLook(const FVector2D& LookInput); // [ADD]

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

protected:
	/* ===================== Replication ===================== */
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUW_KeyGuide> KeyGuideClass;

	UPROPERTY()
	TObjectPtr<UUW_KeyGuide> KeyGuideWidget;

	void OnMoveCompleted(const FInputActionValue& Value);

	void OnMouseLTriggered(const FInputActionValue& Value);
	void OnMouseLCompleted(const FInputActionValue& Value);

	void OnMouseRTriggered(const FInputActionValue& Value);
	void OnMouseRCompleted(const FInputActionValue& Value);
};