#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "BaseAIAnimInstance.generated.h"

UCLASS()
class TEAM6_MULTIGAME_API UBaseAIAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	// 매 프레임 데이터를 업데이트하는 함수 
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	// 초기화 함수 
	virtual void NativeInitializeAnimation() override;

protected:
	
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	float GroundSpeed;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	bool bShouldMove;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	bool bIsFalling;

	// 캐릭터 참조 (매번 캐스팅하지 않도록 미리 저장)
	UPROPERTY()
	class ACharacter* OwnerCharacter;

	UPROPERTY()
	class UCharacterMovementComponent* OwnerMovement;
};