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
	// AI 컨트롤러와 Pawn(몸) 가져오기
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController) return EBTNodeResult::Failed;

	APawn* AIPawn = AIController->GetPawn();
	if (!AIPawn) return EBTNodeResult::Failed;

	//  현재 AI 위치를 기준으로 설정
	FVector Origin = AIPawn->GetActorLocation();

	// 내비게이션 시스템 가져오기
	UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(GetWorld());
	if (!NavSystem) return EBTNodeResult::Failed;

    // 반경 3000cm 안에서 랜덤 위치 찾기
    FNavLocation RandomLocation;

    // 네비게이션 시스템에서 유효한 포인트를 찾았을 경우
    if (NavSystem->GetRandomReachablePointInRadius(Origin, 3000.0f, RandomLocation))
    {
        // (추가) 성공 시 찾은 위치까지의 거리를 로그로 출력
        UE_LOG(LogTemp, Warning, TEXT("Found Patrol Location. Distance: %f"),
            FVector::Dist(Origin, RandomLocation.Location));

        //  찾은 위치를 블랙보드의 'PatrolLocation' 키에 저장
        OwnerComp.GetBlackboardComponent()->SetValueAsVector(TEXT("PatrolLocation"), RandomLocation.Location);
        return EBTNodeResult::Succeeded;
    }
    // 네비게이션 시스템에서 유효한 포인트를 찾지 못했을 경우 (현재 AI가 멈추는 이유)
    else
    {
        // 실패 시 로그 확인
        UE_LOG(LogTemp, Error, TEXT("--- BTTaskFindRandomLocation FAILED! Check Nav Mesh Volume size. ---"));
        return EBTNodeResult::Failed;
    }
}