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
        SightConfig->SightRadius = 1200.0f;
        SightConfig->LoseSightRadius = 1500.0f;
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

    if (!Actor->ActorHasTag(TEXT("Player"))) return;

    if (Stimulus.WasSuccessfullySensed())
    {
        // 플레이어를 감지했을 때
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
    else
    {
		//  플레이어를 놓쳤을 때
        if (Stimulus.Type == UAISense::GetSenseID<UAISense_Sight>())
        {
            ClearFocus(EAIFocusPriority::Gameplay);

			//  마지막 위치 저장
            BBComp->SetValueAsVector(TargetLocationKeyName, Actor->GetActorLocation());
			// 타겟 초기화
            BBComp->ClearValue(TargetActorKeyName);
        }
    }
}

// 피격 시 호출되는 로직
void ABaseAIController::OnDamagedByPlayer(AActor* Attacker)
{
    if (!HasAuthority() || !Attacker || !GetPawn()) return;

   
    StopMovement();

    UBlackboardComponent* BBComp = GetBlackboardComponent();
    if (BBComp)
    {
        BBComp->SetValueAsObject(TargetActorKeyName, Attacker);

       
    }

    if (AIPerception)
    {
        AIPerception->RequestStimuliListenerUpdate();
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