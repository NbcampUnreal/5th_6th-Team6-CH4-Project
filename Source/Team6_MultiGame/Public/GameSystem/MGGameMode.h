#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MGGameMode.generated.h"

class AMGGameState;

UCLASS()
class TEAM6_MULTIGAME_API AMGGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    AMGGameMode();

protected:
    virtual void BeginPlay() override;

    void StartGame();

    void EndGame();
};
