// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "Character/Squirrel.h"
#include "PlayerController_WASD.generated.h"
/*

UCLASS()
class TEAM6_MULTIGAME_API APlayerController_WASD : public APlayerController
{
	GENERATED_BODY()


public:
    virtual void BeginPlay() override;
    // Called every frame

    virtual void SetupInputComponent() override;

    void OnMoveForwardTriggered(const FInputActionValue& Value);

protected:


    UFUNCTION(Server, Reliable)
    void Server_SendWInput(FVector2D MoveInput);

    // 타이머 핸들
    FTimerHandle TimerHandle_CheckSquirrel;
    FTimerHandle TimerHandle_UpdateViewTarget;



    // 다람쥐와 카메라 확인
    void CheckSquirrelAndCamera();

    // 다람쥐 시점 업데이트
    void UpdateViewTargetToSquirrel();

    // 찾은 다람쥐와 카메라 저장
    ASquirrel* TargetSquirrel = nullptr;
    UCameraComponent* TargetCamera = nullptr;

    // 다람쥐 준비 완료 플래그
    bool bSquirrelReady = false;

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputMappingContext* ClientIMC;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* IA_MoveForward;
};
*/