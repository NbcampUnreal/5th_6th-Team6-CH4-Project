// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputActionValue.h"
#include "MainPlayerController.generated.h"

/**
 * 
 */
class ASharedCamera;
class ASquirrel;
class UInputMappingContext;
class UInputAction;

UENUM(BlueprintType)
enum class EPlayerRole : uint8
{
	Camera,
	Move
};

UCLASS()
class TEAM6_MULTIGAME_API AMainPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AMainPlayerController();

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	virtual void GetLifetimeReplicatedProps(
		TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/* ===================== Role ===================== */

	UPROPERTY(Replicated)
	EPlayerRole PlayerRole;

public:
	void SetRole(EPlayerRole InRole);

	/* ===================== Shared Camera ===================== */

	UPROPERTY(ReplicatedUsing = OnRep_SharedCamera)
	ASharedCamera* SharedCamera;

	UFUNCTION()
	void OnRep_SharedCamera();

	// 서버 전용 세터
	void SetSharedCamera(ASharedCamera* InCamera);

	/* ===================== Target Character ===================== */

	UPROPERTY(Replicated)
	ASquirrel* TargetSquirrel;

	void SetTargetSquirrel(ASquirrel* InSquirrel);

	/* ===================== Input ===================== */

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputMappingContext* IMC;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* IA_Move;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* IA_Look;

	void OnMoveTriggered(const FInputActionValue& Value);
	void OnTurnTriggered(const FInputActionValue& Value);

	/* ===================== Server RPC ===================== */

	UFUNCTION(Server, Reliable)
	void Server_SendMoveInput(FVector2D MoveInput);

	UFUNCTION(Server, Reliable)
	void Server_TurnCamera(FVector2D LookInput);
};