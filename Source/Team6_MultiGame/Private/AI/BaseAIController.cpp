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

    // 1. 시각 설정 구성
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
    }

    // 2. 청각 설정 구성 
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
            // 중복 바인딩 방지 후 등록
            AIPerception->OnTargetPerceptionUpdated.RemoveDynamic(this, &ABaseAIController::OnTargetDetected);
            AIPerception->OnTargetPerceptionUpdated.AddDynamic(this, &ABaseAIController::OnTargetDetected);
        }
    }
}

void ABaseAIController::OnTargetDetected(AActor* Actor, FAIStimulus Stimulus)
{
    if (!HasAuthority()) return;
    UBlackboardComponent* BBComp = GetBlackboardComponent();
    if (!BBComp) return;

    if (Stimulus.WasSuccessfullySensed())
    {
        // 1. 시각 감지
        if (Stimulus.Type == UAISense::GetSenseID<UAISense_Sight>())
        {
            if (Actor && Actor->ActorHasTag(TEXT("Player")))
            {
                BBComp->SetValueAsObject(TargetActorKeyName, Actor);
                SetFocus(Actor); // 플레이어 주시
                UE_LOG(LogTemp, Warning, TEXT("Player Spotted! Focusing..."));
            }
        }
        // 2. 청각 감지
        else if (Stimulus.Type == UAISense::GetSenseID<UAISense_Hearing>())
        {
            if (BBComp->GetValueAsObject(TargetActorKeyName) == nullptr)
            {
                BBComp->SetValueAsVector(TEXT("TargetLocation"), Stimulus.StimulusLocation);
            }
        }
    }
    else
    {
        // 시야에서 놓쳤을 때 바로 지우지 말고 "마지막 위치"로 기록
        if (Stimulus.Type == UAISense::GetSenseID<UAISense_Sight>())
        {
            ClearFocus(EAIFocusPriority::Gameplay);

           
            BBComp->SetValueAsVector(TEXT("TargetLocation"), Actor->GetActorLocation());

      
            // BBComp->ClearValue(TargetActorKeyName); 
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