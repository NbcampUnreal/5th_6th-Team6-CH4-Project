// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "CharacterGameState.generated.h"

/**
 * 
 */
UCLASS()
class TEAM6_MULTIGAME_API ACharacterGameState : public AGameState
{
	GENERATED_BODY()
	
public:

    UPROPERTY(BlueprintReadOnly)
    bool GmaeClear = false;

	UPROPERTY(ReplicatedUsing = OnRep_KillCount, BlueprintReadOnly)
	int32 KillCount = 0;

	void AddKillCount(int32 Delta = 1);

protected:

	UFUNCTION()
	void OnRep_KillCount();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
