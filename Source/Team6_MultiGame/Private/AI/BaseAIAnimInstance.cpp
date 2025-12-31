#include "AI/BaseAIAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

void UBaseAIAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	// 애니메이션을 소유한 캐릭터 정보를 미리 가져와 저장
	OwnerCharacter = Cast<ACharacter>(TryGetPawnOwner());
	if (OwnerCharacter)
	{
		OwnerMovement = OwnerCharacter->GetCharacterMovement();
	}
}

void UBaseAIAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	// 캐릭터와 무브먼트 컴포넌트가 없으면 중단
	if (!OwnerCharacter || !OwnerMovement) return;

	// 1. 속도 계산 (XY 평면상의 속도만 추출)
	FVector Velocity = OwnerCharacter->GetVelocity();
	GroundSpeed = Velocity.Size2D();

	// 2. 움직임 여부 (속도가 일정 이상)
	bShouldMove = (GroundSpeed > 3.f);

	// 3. 공중 상태 여부
	bIsFalling = OwnerMovement->IsFalling();
}