#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "MGGamePhase.h"          
#include "MGGameState.generated.h"

UCLASS()
class TEAM6_MULTIGAME_API AMGGameState : public AGameStateBase
{
    GENERATED_BODY()

public:
    AMGGameState();

    UPROPERTY(ReplicatedUsing = OnRep_GamePhase, BlueprintReadOnly, Category = "Game")
    EMGGamePhase CurrentPhase;

    void SetGamePhase(EMGGamePhase NewPhase);

protected:
    UFUNCTION()
    void OnRep_GamePhase();

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
