#include "AI/BaseAIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "AI/BaseAICharacter.h" 

ABaseAIController::ABaseAIController()
{
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

    if (HasAuthority())
    {
        if (BehaviorTreeAsset)
        {
            RunBehaviorTree(BehaviorTreeAsset);
        }

        if (AIPerception)
        {
            // 중복 바인딩 방지 후 등록
            AIPerception->OnTargetPerceptionUpdated.RemoveDynamic(this, &ABaseAIController::OnTargetDetected);
            AIPerception->OnTargetPerceptionUpdated.AddDynamic(this, &ABaseAIController::OnTargetDetected);
        }
    }
}

void ABaseAIController::OnTargetDetected(AActor* Actor, FAIStimulus Stimulus)
{
    if (!HasAuthority()) return;

    // 조종 중인 캐릭터가 죽었는지 확인
    ABaseAICharacter* MyCharacter = Cast<ABaseAICharacter>(GetPawn());
    if (!MyCharacter || MyCharacter->IsDead()) return;

    UBlackboardComponent* BBComp = GetBlackboardComponent();
    if (!BBComp) return;

    if (Stimulus.WasSuccessfullySensed())
    {
        // 플레이어 태그가 있는 액터만 타겟으로 지정
        if (Actor && Actor->ActorHasTag(TEXT("Player")))
        {
            BBComp->SetValueAsObject(TargetActorKeyName, Actor);
            UE_LOG(LogTemp, Warning, TEXT("[Server] Found Player: %s"), *Actor->GetName());
        }
    }
    else
    {
        // 현재 타겟을 놓쳤을 때만 블랙보드 비우기
        AActor* CurrentTarget = Cast<AActor>(BBComp->GetValueAsObject(TargetActorKeyName));
        if (CurrentTarget == Actor)
        {
            BBComp->ClearValue(TargetActorKeyName);
            UE_LOG(LogTemp, Display, TEXT("[Server] Lost Target: %s"), *Actor->GetName());
        }
    }
}

void ABaseAIController::OnAICharacterDead()
{
    if (!HasAuthority()) return;

    // 1. 비헤이비어 트리 정지
    UBehaviorTreeComponent* BTComp = Cast<UBehaviorTreeComponent>(BrainComponent);
    if (BTComp)
    {
        BTComp->StopTree(EBTStopMode::Safe);
    }

    // 2. 블랙보드 타겟 정보 삭제
    UBlackboardComponent* BBComp = GetBlackboardComponent();
    if (BBComp)
    {
        BBComp->ClearValue(TargetActorKeyName);
    }

    // 3. 인지 시스템 비활성화
    if (AIPerception)
    {
        AIPerception->Deactivate();
    }
}