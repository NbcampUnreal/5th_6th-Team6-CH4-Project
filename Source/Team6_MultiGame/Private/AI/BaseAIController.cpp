#include "AI/BaseAIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h" // 누락된 헤더 추가
#include "AI/BaseAICharacter.h" 

ABaseAIController::ABaseAIController()
{
    bReplicates = true;

    AIPerception = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerception"));

    SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    if (SightConfig)
    {
        SightConfig->SightRadius = 500.0f;
        SightConfig->LoseSightRadius = 700.0f;
        SightConfig->PeripheralVisionAngleDegrees = 90.0f;
        SightConfig->DetectionByAffiliation.bDetectEnemies = true;
        SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
        SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
        AIPerception->ConfigureSense(*SightConfig);
    }

    HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
    if (HearingConfig)
    {
        HearingConfig->HearingRange = 1200.0f;
        HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
        HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
        HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;
        AIPerception->ConfigureSense(*HearingConfig);
    }

    if (SightConfig)
    {
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
            AIPerception->OnTargetPerceptionUpdated.AddUniqueDynamic(this, &ABaseAIController::OnTargetDetected);
        }
    }
}

void ABaseAIController::OnTargetDetected(AActor* Actor, FAIStimulus Stimulus)
{
    if (!HasAuthority() || !Actor) return;
    UBlackboardComponent* BBComp = GetBlackboardComponent();
    if (!BBComp) return;

    // [수정] 몬스터끼리 서로 인식하지 않도록 Player 태그 검사 강화
    if (!Actor->ActorHasTag(TEXT("Player"))) return;

    if (Stimulus.WasSuccessfullySensed())
    {
        if (Stimulus.Type == UAISense::GetSenseID<UAISense_Sight>())
        {
            BBComp->SetValueAsObject(TargetActorKeyName, Actor);
            SetFocus(Actor);
        }
        else if (Stimulus.Type == UAISense::GetSenseID<UAISense_Hearing>())
        {
            if (BBComp->GetValueAsObject(TargetActorKeyName) == nullptr)
            {
                BBComp->SetValueAsVector(TargetLocationKeyName, Stimulus.StimulusLocation);
            }
        }
    }
    else // 시야에서 놓쳤을 때
    {
        if (Stimulus.Type == UAISense::GetSenseID<UAISense_Sight>())
        {
            ClearFocus(EAIFocusPriority::Gameplay);
            BBComp->SetValueAsVector(TargetLocationKeyName, Actor->GetActorLocation());
            // 일정 시간 후 타겟을 완전히 지우는 로직은 서비스(BT Service)에서 처리하는 것이 좋습니다.
        }
    }
}

// 피격 시 호출되는 로직
void ABaseAIController::OnDamagedByPlayer(AActor* Attacker)
{
    if (!HasAuthority() || !Attacker) return;

   
    if (!Attacker->ActorHasTag(TEXT("Player"))) return;

    UBlackboardComponent* BBComp = GetBlackboardComponent();
    if (BBComp)
    {
        BBComp->SetValueAsObject(TargetActorKeyName, Attacker);
        SetFocus(Attacker); 
    }

    // 시야 범위 일시적 확장 (경계 태세)
    if (SightConfig && AIPerception)
    {
        SightConfig->SightRadius = 1000.0f;
        SightConfig->LoseSightRadius = 1200.0f;
        AIPerception->ConfigureSense(*SightConfig);
    }
}

void ABaseAIController::OnAICharacterDead()
{
    if (!HasAuthority()) return;

    UBehaviorTreeComponent* BTComp = Cast<UBehaviorTreeComponent>(BrainComponent);
    if (BTComp) BTComp->StopTree(EBTStopMode::Safe);

    UBlackboardComponent* BBComp = GetBlackboardComponent();
    if (BBComp) BBComp->ClearValue(TargetActorKeyName);

    if (AIPerception) AIPerception->Deactivate();

    ClearFocus(EAIFocusPriority::Gameplay);
}