#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTServiceCalculateDistance.generated.h"

UCLASS()
class TEAM6_MULTIGAME_API UBTServiceCalculateDistance : public UBTService
{
    GENERATED_BODY()

public:
    UBTServiceCalculateDistance();

protected:
   
    virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
};