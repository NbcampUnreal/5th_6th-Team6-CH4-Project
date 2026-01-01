// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "CharacterGameMode.generated.h"

class ASquirrel;
class AMainPlayerController;

/**
 * 
 */
enum class EPlayerRole : uint8;
UCLASS()
class TEAM6_MULTIGAME_API ACharacterGameMode : public AGameMode
{
	GENERATED_BODY()
	

public:
	ACharacterGameMode();

	//virtual void BeginPlay() override;
	

	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;   //  추가

	void ClearGame();
	void EndGame();
	void GameOver(bool bClear);

	UFUNCTION()
	void ReturnToLobby();

protected:
	
	//UPROPERTY()
	//ASquirrel* TargetSquirrel;

	
	// 현재 접속 중인 컨트롤러들의 역할을 보고, 비어있는 역할 1개 반환
// 4개가 다 차 있으면 None 반환
	EPlayerRole FindFreeRole() const;

	UPROPERTY()
	TObjectPtr<ASquirrel> TargetSquirrel;


};
