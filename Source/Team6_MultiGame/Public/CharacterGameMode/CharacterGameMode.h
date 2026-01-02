// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "CharacterGameMode.generated.h"

class ASquirrel;
/**
 * 
 */
UCLASS()
class TEAM6_MULTIGAME_API ACharacterGameMode : public AGameMode
{
	GENERATED_BODY()
	

public:
	ACharacterGameMode();

	//virtual void BeginPlay() override;
	

	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;   //  Ãß°¡

	void ClearGame();
	void EndGame();
	void GameOver(bool bClear);

	UFUNCTION()
	void ReturnToLobby();

protected:

	UPROPERTY()
	ASquirrel* TargetSquirrel;

	int32 PlayerIndex = 0;
};
