// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "Character/Squirrel.h"
#include "PlayerController_Mouse.generated.h"

/**
 *
 */
UCLASS()
class TEAM6_MULTIGAME_API APlayerController_Mouse : public APlayerController
{
    GENERATED_BODY()


public:
    virtual void BeginPlay() override;
    // Called every frame

    virtual void SetupInputComponent() override;

    void OnMouseTriggered(const FInputActionValue& Value);

  //다람쥐 빙의 요청
    UFUNCTION(Server, Reliable)
    void Server_RequestPossessSquirrel();
protected:






    // 찾은 다람쥐와 카메라 저장
    ASquirrel* TargetSquirrel = nullptr;
    UCameraComponent* TargetCamera = nullptr;

    // 다람쥐 준비 완료 플래그
    bool bSquirrelReady = false;

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputMappingContext* ClientMouseIMC;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* IA_Mouse;
};

