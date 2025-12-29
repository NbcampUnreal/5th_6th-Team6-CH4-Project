#include "AI/BTTaskFindRandomLocation.h"
#include "AIController.h"
#include "NavigationSystem.h" 
#include "BehaviorTree/BlackboardComponent.h"

UBTTaskFindRandomLocation::UBTTaskFindRandomLocation()
{
    NodeName = TEXT("Find Random Location");
}

EBTNodeResult::Type UBTTaskFindRandomLocation::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AAIController* AIController = OwnerComp.GetAIOwner();
    if (!AIController) return EBTNodeResult::Failed;

    APawn* AIPawn = AIController->GetPawn();
    if (!AIPawn) return EBTNodeResult::Failed;

    // 서버 권한 확인 
    if (!AIPawn->HasAuthority()) return EBTNodeResult::Failed;

    FVector Origin = AIPawn->GetActorLocation();
    UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(GetWorld());
    if (!NavSystem) return EBTNodeResult::Failed;

    FNavLocation RandomLocation;

    // 반경 내 이동 가능한 점 찾기
    if (NavSystem->GetRandomReachablePointInRadius(Origin, 3000.0f, RandomLocation))
    {
        // 블랙보드에 위치 저장 
        OwnerComp.GetBlackboardComponent()->SetValueAsVector(TEXT("PatrolLocation"), RandomLocation.Location);
        return EBTNodeResult::Succeeded;
    }

    return EBTNodeResult::Failed;
}