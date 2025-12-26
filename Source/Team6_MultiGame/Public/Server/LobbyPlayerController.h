
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "LobbyPlayerController.generated.h"


UCLASS()
class TEAM6_MULTIGAME_API ALobbyPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
    virtual void BeginPlay() override;

    //UFUNCTION(BlueprintCallable, Category = "Lobby")
    //void ToggleReady();

private:
    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<UUserWidget> LobbyWidgetClass;

    UPROPERTY()
    TObjectPtr<UUserWidget> LobbyWidgetInstance;

protected:
    //UFUNCTION(Server, Reliable)
    //void ServerToggleReady();
};
