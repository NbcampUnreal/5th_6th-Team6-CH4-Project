

#include "Server/LobbyPlayerController.h"

#include "Blueprint/UserWidget.h"
#include "Server/LobbyPlayerState.h"
#include "Server/TitleGameModeBase.h"
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

//void ALobbyPlayerController::ToggleReady()
//{
//    ServerToggleReady();
//}

//void ALobbyPlayerController::ServerToggleReady_Implementation()
//{
//    ALobbyPlayerState* PS = GetPlayerState<ALobbyPlayerState>();
//    if (!PS) return;
//
//    PS->SetReady(!PS->bReady);
//
//    if (UWorld* World = GetWorld())
//    {
//        if (ALobbyGameModeBase* GM = Cast<ALobbyGameModeBase>(World->GetAuthGameMode()))
//        {
//            GM->TryStartGame();
//        }
//    }
//}