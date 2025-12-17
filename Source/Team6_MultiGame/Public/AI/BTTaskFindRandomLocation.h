#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTaskFindRandomLocation.generated.h"


UCLASS()
class TEAM6_MULTIGAME_API UBTTaskFindRandomLocation : public UBTTaskNode
{
	GENERATED_BODY()


public:
	UBTTaskFindRandomLocation();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	
};
