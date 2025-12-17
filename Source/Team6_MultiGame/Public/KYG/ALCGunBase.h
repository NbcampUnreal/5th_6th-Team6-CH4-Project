// AALCGunBase.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ALCGunBase.generated.h"

UCLASS()
class TEAM6_MULTIGAME_API AALCGunBase : public AActor
{
	GENERATED_BODY()
	
public:	
	AALCGunBase();

	UFUNCTION(BlueprintCallable)
	void Fire();
protected:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class AALCProjectileBase> ProjectileClass;

	UPROPERTY(EditDefaultsOnly)
	float Damage;

	UPROPERTY(EditDefaultsOnly)
	float BulletSpeed;

	UPROPERTY(EditDefaultsOnly)
	float FireCooldown;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* GunMesh;

	float LastFireTime;

	UFUNCTION(Server, Reliable)
	void ServerFire();
	void ServerFire_Implementation();

	void HandleFire();
};
