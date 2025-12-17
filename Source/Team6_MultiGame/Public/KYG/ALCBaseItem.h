// ALCBaseItem.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ALCBaseItem.generated.h"

UCLASS()
class TEAM6_MULTIGAME_API AALCBaseItem : public AActor
{
	GENERATED_BODY()
	
public:	

	AALCBaseItem();

protected:
	UPROPERTY(VisibleAnywhere)
	class USphereComponent* CollisionComp;

	virtual void BeginPlay() override;

	UFUNCTION()
	void OnItemOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	virtual void OnPickedUp(class ACharacter* Character);
};
