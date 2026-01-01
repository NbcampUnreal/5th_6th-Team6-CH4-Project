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
	

	// ï¿½Ã·ï¿½ï¿½Ì¾ï¿½ ï¿½ï¿½ï¿½ï¿½ ï¿½ï¿½ È£ï¿½ï¿½
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;   // ¡Ú Ãß°¡


	/////////////////////////////////////////////////   ï¿½ï¿½ï¿½ï¿½   /////////////////////////////////////////////////
	void ClearGame();
	void EndGame();
	void GameOver(bool bClear);
	/////////////////////////////////////////////////   ï¿½ï¿½ï¿½ï¿½   /////////////////////////////////////////////////


protected:
	
	// ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ ï¿½ï¿½Ä¡ï¿½ï¿½ ï¿½Ù¶ï¿½ï¿½ï¿½
	UPROPERTY()
	ASquirrel* TargetSquirrel;

	int32 PlayerIndex = 0;

	
};
