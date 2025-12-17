//ALCHealingActor.h

#pragma once

#include "CoreMinimal.h"
#include "KYG/ALCBaseItem.h"
#include "ALCHealingActor.generated.h"

UCLASS()
class TEAM6_MULTIGAME_API AALCHealingActor : public AALCBaseItem
{
	GENERATED_BODY()
	
public:	
	AALCHealingActor();

protected:
	UPROPERTY(EditAnywhere, Category = "Heal")
	float HealAmount = 30.f;

	// BaseItem의 가상 함수 오버라이드
	virtual void OnPickedUp(ACharacter* Character) override;

};
