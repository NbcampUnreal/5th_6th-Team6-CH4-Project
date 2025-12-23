#pragma once

#include "CoreMinimal.h"
#include "MGGamePhase.generated.h"

UENUM(BlueprintType)
enum class EMGGamePhase : uint8
{
    Warmup     UMETA(DisplayName = "Warmup"),
    InProgress UMETA(DisplayName = "InProgress"),
    GameOver   UMETA(DisplayName = "GameOver")
};
