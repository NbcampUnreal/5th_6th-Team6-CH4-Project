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
	

	// 플레이어 접속 시 호출
	virtual void PostLogin(APlayerController* NewPlayer) override;

	/////////////////////////////////////////////////   수정   /////////////////////////////////////////////////
	void ClearGmae();
	void EndGame();
	void GameOver(bool bClear);
	/////////////////////////////////////////////////   수정   /////////////////////////////////////////////////

protected:
	
	// 레벨에 배치된 다람쥐
	UPROPERTY()
	ASquirrel* TargetSquirrel;

	int32 PlayerIndex = 0;

	
};
