#include "AI/BTTask_Attack.h"
#include "AIController.h"
#include "AI/BaseAICharacter.h"
#include "Animation/AnimMontage.h" // 몽타주 정보 사용을 위해 포함

UBTTask_Attack::UBTTask_Attack()
{
    NodeName = TEXT("Attack");
}

EBTNodeResult::Type UBTTask_Attack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AAIController* AIController = OwnerComp.GetAIOwner();
    if (!AIController) return EBTNodeResult::Failed;

    ABaseAICharacter* MyAI = Cast<ABaseAICharacter>(AIController->GetPawn());
    if (MyAI && MyAI->HasAuthority())
    {
        // 1. 공격 함수 호출
        MyAI->PlayAttackMontage();

      
        return EBTNodeResult::Succeeded;
    }

    return EBTNodeResult::Failed;
}