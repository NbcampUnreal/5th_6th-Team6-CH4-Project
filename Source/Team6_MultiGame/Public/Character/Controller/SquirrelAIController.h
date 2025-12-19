// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "SquirrelAIController.generated.h"

/**
 * 
 */
UCLASS()
class TEAM6_MULTIGAME_API ASquirrelAIController : public AAIController
{
	GENERATED_BODY()
	
public:
	ASquirrelAIController();


protected:
	virtual void OnPossess(APawn* InPawn) override;

public:
	// ★ PlayerController → Server → AIController
	// ★ 카메라 회전 입력 처리 (서버 전용)
	void AddCameraInput(const FVector2D& LookInput);
};
