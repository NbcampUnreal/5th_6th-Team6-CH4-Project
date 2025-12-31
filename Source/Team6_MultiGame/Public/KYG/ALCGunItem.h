// ALCGunItem.h

#pragma once

#include "CoreMinimal.h"
#include "KYG/ALCBaseItem.h"
#include "ALCGunItem.generated.h"

UCLASS()
class TEAM6_MULTIGAME_API AALCGunItem : public AALCBaseItem
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly, Category = "Gun")
	TSubclassOf<class AALCGunBase> GunClass;

protected:
	virtual void OnPickedUp(class ACharacter* Character) override;
};
