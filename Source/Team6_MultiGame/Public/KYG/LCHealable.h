// LCHealable.h

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "LCHealable.generated.h"


UINTERFACE(MinimalAPI)
class ULCHealable : public UInterface
{
	GENERATED_BODY()
};

class TEAM6_MULTIGAME_API ILCHealable
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Heal")
	void ReceiveHeal(float HealAmount);
};
