// ALCProjectileBase.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ALCProjectileBase.generated.h"

UCLASS()
class TEAM6_MULTIGAME_API AALCProjectileBase : public AActor
{
	GENERATED_BODY()
	
public:	

	AALCProjectileBase();

	virtual void Init(float InDamage, FVector Direction, float Speed);

protected:
	UPROPERTY(VisibleAnywhere)
	class USphereComponent* CollisionComp;

	UPROPERTY(VisibleAnywhere)
	class UProjectileMovementComponent* MovementComp;

	UPROPERTY(EditDefaultsOnly)
	float Damage = 20.f;

	UPROPERTY(EditDefaultsOnly, Category = "Projectile")
	float MaxRange = 3000.f;

	UFUNCTION()
	void OnHit(
		UPrimitiveComponent* Overlapped,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& Hit
	);
};
