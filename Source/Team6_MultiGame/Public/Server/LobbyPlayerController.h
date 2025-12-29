
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "TimerManager.h"
#include "Server/LobbyPlayerState.h"
#include "LobbyPlayerController.generated.h"

class UUserWidget;

UCLASS()
class TEAM6_MULTIGAME_API ALobbyPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
    virtual void BeginPlay() override;

    UFUNCTION(BlueprintCallable, Category = "Lobby")
    void ToggleReady();

protected:
    UFUNCTION(Server, Reliable)
    void ServerToggleReady();

private:
    void TryInitVoiceLobby();

    FTimerHandle VoiceInitTimerHandle;
    bool bVoiceInitDone = false;

    virtual void OnRep_PlayerState() override;

    UPROPERTY()
    TObjectPtr<ALobbyPlayerState> CachedLobbyPS = nullptr;

    bool bTriedVoiceInit = false;

private:
    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<UUserWidget> LobbyWidgetClass;

    UPROPERTY()
    TObjectPtr<UUserWidget> LobbyWidgetInstance;
};
