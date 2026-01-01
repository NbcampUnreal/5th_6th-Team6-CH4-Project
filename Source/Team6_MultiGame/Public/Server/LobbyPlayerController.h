
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "TimerManager.h"
#include "LobbyPlayerController.generated.h"

class UUserWidget;
class ALobbyPlayerState;


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

    UFUNCTION(Server, Reliable)
    void Server_RequestVoiceJoinToken(const FString& InRoomId);

    UFUNCTION(Client, Reliable)
    void Client_ReceiveVoiceJoinToken(const FString& InRoomId, const FString& InToken);

    virtual void OnRep_PlayerState() override;

private:
    void TryInitVoiceLobby();

    FTimerHandle VoiceInitTimerHandle;
    bool bVoiceInitDone = false;
    bool bTriedVoiceInit = false;

    UPROPERTY()
    TObjectPtr<ALobbyPlayerState> CachedLobbyPS = nullptr;

private:
    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<UUserWidget> LobbyWidgetClass;

    UPROPERTY()
    TObjectPtr<UUserWidget> LobbyWidgetInstance;
};
