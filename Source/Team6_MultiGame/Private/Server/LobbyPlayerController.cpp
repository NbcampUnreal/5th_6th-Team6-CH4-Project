

#include "Server/LobbyPlayerController.h"

#include "Blueprint/UserWidget.h"
#include "Server/LobbyPlayerState.h"
#include "Server/LobbyGameModeBase.h"
#include "Engine/World.h"


void ALobbyPlayerController::BeginPlay()
{
    Super::BeginPlay();

    if (!IsLocalController())
        return;

    if (LobbyWidgetClass)
    {
        LobbyWidgetInstance = CreateWidget<UUserWidget>(this, LobbyWidgetClass);
        if (LobbyWidgetInstance)
        {
            LobbyWidgetInstance->AddToViewport();

            FInputModeUIOnly Mode;
            Mode.SetWidgetToFocus(LobbyWidgetInstance->GetCachedWidget());
            SetInputMode(Mode);

            bShowMouseCursor = true;
        }
    }
}

void ALobbyPlayerController::ToggleReady()
{
    ServerToggleReady();
}

void ALobbyPlayerController::ServerToggleReady_Implementation()
{
    UE_LOG(LogTemp, Warning, TEXT("[ServerToggleReady] CALLED. PC=%s"), *GetName());

    ALobbyPlayerState* PS = GetPlayerState<ALobbyPlayerState>();
    UE_LOG(LogTemp, Warning, TEXT("[ServerToggleReady] PS=%s Ready(before)=%d"),
        *GetNameSafe(PS), PS ? PS->bReady : -1);

    if (!PS) return;

    PS->SetReady(!PS->bReady);

    if (UWorld* World = GetWorld())
    {
        if (ALobbyGameModeBase* GM = Cast<ALobbyGameModeBase>(World->GetAuthGameMode()))
        {
            GM->TryStartGame();
        }
    }
}