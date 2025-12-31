#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionTypes.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "BaseAIController.generated.h"

UCLASS()
class TEAM6_MULTIGAME_API ABaseAIController : public AAIController
{
    GENERATED_BODY()

public:
    ABaseAIController();

    // 에디터에서 할당할 비헤이비어 트리 에셋
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    class UBehaviorTree* BehaviorTreeAsset;

    // AI가 죽었을 때 호출될 함수 (BaseAICharacter의 Die()에서 호출)
    void OnAICharacterDead();

protected:
    virtual void OnPossess(APawn* InPawn) override;

    // 타겟 감지 시 실행될 콜백 함수
    UFUNCTION()
    void OnTargetDetected(AActor* Actor, FAIStimulus Stimulus);

    // AI 인지 컴포넌트
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    class UAIPerceptionComponent* AIPerception;

    // 시각 설정 컴포넌트 
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    class UAISenseConfig_Sight* SightConfig;

    // 청각 설정 컴포넌트
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    class UAISenseConfig_Hearing* HearingConfig;

    // 블랙보드 키 이름 
    UPROPERTY(EditAnywhere, Category = "AI")
    FName TargetActorKeyName = TEXT("TargetActor");
};