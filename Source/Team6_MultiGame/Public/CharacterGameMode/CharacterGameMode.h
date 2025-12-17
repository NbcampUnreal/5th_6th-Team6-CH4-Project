// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "GameFramework/PlayerController.h"
#include "CharacterGameMode.generated.h"

class AMoveManager;
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

	virtual void BeginPlay() override;
	

	// 플레이어 접속 시 호출
	virtual void PostLogin(APlayerController* NewPlayer) override;

	/** 클라이언트별 PlayerController 클래스 결정 */
	virtual APlayerController* SpawnPlayerController(
		ENetRole InRemoteRole,
		const FString& Options
	) override;


	// ===== BP에서 할당 =====

		// 마우스 전용 컨트롤러
	UPROPERTY(EditDefaultsOnly, Category = "Controller")
	TSubclassOf<APlayerController> MouseControllerClass;

	// 키보드(WASD) 전용 컨트롤러
	UPROPERTY(EditDefaultsOnly, Category = "Controller")
	TSubclassOf<APlayerController> WASDControllerClass;

	UPROPERTY()
	AMoveManager* MoveManager;

	// 다람쥐 자동 할당
	UFUNCTION()
	void AssignSquirrelToController(APlayerController* PC);

	
};
