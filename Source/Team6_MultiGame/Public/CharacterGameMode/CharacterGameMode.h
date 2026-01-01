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
	

	// �÷��̾� ���� �� ȣ��
	virtual void PostLogin(APlayerController* NewPlayer) override;

	/////////////////////////////////////////////////   ����   /////////////////////////////////////////////////
	void ClearGame();
	void EndGame();
	void GameOver(bool bClear);
	/////////////////////////////////////////////////   ����   /////////////////////////////////////////////////

protected:
	
	// ������ ��ġ�� �ٶ���
	UPROPERTY()
	ASquirrel* TargetSquirrel;

	int32 PlayerIndex = 0;

	
};
