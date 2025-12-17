// Fill out your copyright notice in the Description page of Project Settings.
#include "Character/Controller/PlayerController_Mouse.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "EngineUtils.h"
#include "Character/MoveManager.h"
#include "Engine/Engine.h"
#include "Character/Squirrel.h"
#include "Camera/CameraComponent.h"
#include <CharacterGameMode/CharacterGameMode.h>


void APlayerController_Mouse::BeginPlay()
{
    Super::BeginPlay();
    UE_LOG(LogTemp, Warning, TEXT("PlayerController_Mouse"));

    if (ULocalPlayer* LP = GetLocalPlayer())
    {
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
            ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LP))
        {
            Subsystem->ClearAllMappings();
            Subsystem->AddMappingContext(ClientMouseIMC, 0);
        }
    }


    if (IsLocalController())
    {
        // 폰이 아직 없다면 요청
        if (!GetPawn())
        {
            Server_RequestPossessSquirrel();
        }
    }
}



void APlayerController_Mouse::SetupInputComponent()
{
    Super::SetupInputComponent();

    UE_LOG(LogTemp, Warning, TEXT("SetupInputComponent CALLED"));

    if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(InputComponent))
    {
        UE_LOG(LogTemp, Warning, TEXT("EnhancedInputComponent FOUND"));
        EIC->BindAction(IA_Mouse,
            ETriggerEvent::Triggered,
            this,
            &APlayerController_Mouse::OnMouseTriggered);

        UE_LOG(LogTemp, Warning, TEXT("BindAction DONE"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("EnhancedInputComponent NOT FOUND"));
    }
}

void APlayerController_Mouse::OnMouseTriggered(const FInputActionValue& Value)
{

    if (!IsLocalController()) return;

    // 여기서 Possess 요청하던 코드는 삭제 (BeginPlay에서 이미 했음)

    FVector2D LookInput = Value.Get<FVector2D>();

    if (APawn* P = GetPawn())
    {
        if (ASquirrel* S = Cast<ASquirrel>(P))
        {
            S->Look(LookInput);
        }
    }
}


void APlayerController_Mouse::Server_RequestPossessSquirrel_Implementation()
{
    UE_LOG(LogTemp, Warning, TEXT("Squirrel possessed Try"));
    if (AGameModeBase* GM = GetWorld()->GetAuthGameMode())
    {
        UE_LOG(LogTemp, Warning, TEXT("Squirrel possessed Try GetAuthGameMode()"));
        if (ACharacterGameMode* MyGM = Cast<ACharacterGameMode>(GM))
        {
            UE_LOG(LogTemp, Warning, TEXT("Squirrel possessed Try Cast<ACharacterGameMode>"));
            MyGM->AssignSquirrelToController(this);
        }
    }
}



