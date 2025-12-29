#include "AI/BTTask_Attack.h"
#include "AIController.h"
#include "AI/BaseAICharacter.h" 

UBTTask_Attack::UBTTask_Attack()
{
    NodeName = TEXT("Attack");
}

EBTNodeResult::Type UBTTask_Attack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AAIController* AIController = OwnerComp.GetAIOwner();
    if (!AIController) return EBTNodeResult::Failed;

    APawn* AIPawn = AIController->GetPawn();
    if (!AIPawn) return EBTNodeResult::Failed;

    // 1.  AI 캐릭터 클래스로 캐스팅
    ABaseAICharacter* MyAI = Cast<ABaseAICharacter>(AIPawn);
    if (MyAI)
    {
        // 2. 서버에서만 실행되도록 보장 
        if (MyAI->HasAuthority())
        {
            // 3. 공격 함수 호출
            MyAI->PlayAttackMontage();

            return EBTNodeResult::Succeeded;
        }
    }

    return EBTNodeResult::Failed;
}