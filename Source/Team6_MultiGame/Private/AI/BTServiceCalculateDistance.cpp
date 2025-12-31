#include "AI/BTServiceCalculateDistance.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTServiceCalculateDistance::UBTServiceCalculateDistance()
{
    NodeName = TEXT("Calculate Distance");
    Interval = 0.1f; // 0.1초마다 계산 (성능 최적화)
}

void UBTServiceCalculateDistance::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

    APawn* AIPawn = OwnerComp.GetAIOwner()->GetPawn();
    // 블랙보드에서 타겟 플레이어를 가져옴
    AActor* TargetActor = Cast<AActor>(OwnerComp.GetBlackboardComponent()->GetValueAsObject(TEXT("TargetActor")));

    if (AIPawn && TargetActor)
    {
        // 1. 거리 계산
        float Distance = FVector::Dist(AIPawn->GetActorLocation(), TargetActor->GetActorLocation());

        // 2. 블랙보드의 "TargetDistance" 키에 결과값 저장
        OwnerComp.GetBlackboardComponent()->SetValueAsFloat(TEXT("TargetDistance"), Distance);
    }
}