#include "AI/BTTask_Attack.h"
#include "AIController.h"
#include "AI/BaseAICharacter.h"
#include "BehaviorTree/BlackboardComponent.h" // 추가

UBTTask_Attack::UBTTask_Attack()
{
    NodeName = TEXT("Attack");
    bNotifyTick = true;
}

EBTNodeResult::Type UBTTask_Attack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AAIController* AIController = OwnerComp.GetAIOwner();
    if (!AIController) return EBTNodeResult::Failed;

    ABaseAICharacter* MyAI = Cast<ABaseAICharacter>(AIController->GetPawn());
    if (MyAI && MyAI->HasAuthority())
    {
        MyAI->PlayAttackMontage();

        // 애니메이션 재생 시간 (몽타주 길이에 맞춰 조절)
        float AttackAnimDuration = 1.2f;

        // OwnerComp의 주소를 안전하게 캡처하여 타이머 실행
        TWeakObjectPtr<UBehaviorTreeComponent> MyOwnerComp(&OwnerComp);

        FTimerHandle TimerHandle;
        MyAI->GetWorld()->GetTimerManager().SetTimer(TimerHandle, [MyOwnerComp, this]()
            {
                if (MyOwnerComp.IsValid())
                {
                    
                    MyOwnerComp->OnTaskFinished(this, EBTNodeResult::Succeeded);
                }
            }, AttackAnimDuration, false);

        return EBTNodeResult::InProgress;
    }

    return EBTNodeResult::Failed;
}