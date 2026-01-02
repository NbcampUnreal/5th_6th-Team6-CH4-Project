#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionTypes.h"
#include "BaseAIController.generated.h"

UCLASS()
class TEAM6_MULTIGAME_API ABaseAIController : public AAIController
{
    GENERATED_BODY()

public:
    ABaseAIController();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    class UBehaviorTree* BehaviorTreeAsset;

    void OnAICharacterDead();

    // 피격 시 시야를 넓히고 공격자를 바라보게 하는 함수
    void OnDamagedByPlayer(AActor* Attacker);

protected:
    virtual void OnPossess(APawn* InPawn) override;

    UFUNCTION()
    void OnTargetDetected(AActor* Actor, FAIStimulus Stimulus);

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    class UAIPerceptionComponent* AIPerception;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    class UAISenseConfig_Sight* SightConfig;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    class UAISenseConfig_Hearing* HearingConfig;

    UPROPERTY(EditAnywhere, Category = "AI")
    FName TargetActorKeyName = TEXT("TargetActor");

    UPROPERTY(EditAnywhere, Category = "AI")
    FName TargetLocationKeyName = TEXT("TargetLocation");
};