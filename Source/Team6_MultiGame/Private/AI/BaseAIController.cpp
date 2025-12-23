#include "AI/BaseAIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"

ABaseAIController::ABaseAIController()
{
    // 멀티플레이어 설정: 컨트롤러 상태를 클라이언트에 복제할 수 있도록 설정
    bReplicates = true;

    AIPerception = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerception"));
    SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));

    if (SightConfig)
    {
        SightConfig->SightRadius = 1000.0f;
        SightConfig->LoseSightRadius = 1200.0f;
        SightConfig->PeripheralVisionAngleDegrees = 90.0f;

        
        SightConfig->DetectionByAffiliation.bDetectEnemies = true;
        SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
        SightConfig->DetectionByAffiliation.bDetectFriendlies = true;

        AIPerception->ConfigureSense(*SightConfig);
        AIPerception->SetDominantSense(SightConfig->GetSenseImplementation());
    }
}

void ABaseAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    // 오직 서버에서만 비헤이비어 트리를 실행
    if (HasAuthority() && BehaviorTreeAsset)
    {
        RunBehaviorTree(BehaviorTreeAsset);
    }

    if (AIPerception)
    {
        AIPerception->OnTargetPerceptionUpdated.AddDynamic(this, &ABaseAIController::OnTargetDetected);
    }
}

void ABaseAIController::OnTargetDetected(AActor* Actor, FAIStimulus Stimulus)
{
    // 서버에서만 블랙보드 값을 수정하도록 제어
    if (!HasAuthority()) return;

    UBlackboardComponent* BlackboardComp = GetBlackboardComponent();
    if (BlackboardComp)
    {
        if (Stimulus.WasSuccessfullySensed())
        {
            // 플레이어 태그가 있는 대상만 추적 
            if (Actor && Actor->ActorHasTag(TEXT("Player")))
            {
                BlackboardComp->SetValueAsObject(TEXT("TargetActor"), Actor);
                UE_LOG(LogTemp, Warning, TEXT("[Server] Player Detected by AI!"));
            }
        }
        else
        {
            // 현재 타겟을 놓쳤을 때만 클리어
            AActor* CurrentTarget = Cast<AActor>(BlackboardComp->GetValueAsObject(TEXT("TargetActor")));
            if (CurrentTarget == Actor)
            {
                BlackboardComp->ClearValue(TEXT("TargetActor"));
            }
        }
    }
}