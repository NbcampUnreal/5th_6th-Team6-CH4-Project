// ALCProjectileBase.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NiagaraSystem.h"
#include "ALCProjectileBase.generated.h"


class USphereComponent;
class UProjectileMovementComponent;
class UNiagaraSystem;
class UNiagaraComponent;


UCLASS()
class TEAM6_MULTIGAME_API AALCProjectileBase : public AActor
{
	GENERATED_BODY()
	
public:	

	AALCProjectileBase();

	virtual void Init(float InDamage, FVector Direction, float Speed);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class USphereComponent* CollisionComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UStaticMeshComponent* MeshComp;

	UPROPERTY(VisibleAnywhere)
	class UProjectileMovementComponent* MovementComp;

	UPROPERTY(EditDefaultsOnly, Category = "Projectile")
	float Damage = 20.f;

	UPROPERTY(EditDefaultsOnly, Category = "Projectile")
	float MaxRange = 3000.f;

	UPROPERTY(VisibleAnywhere, Category = "Projectile")
	FVector SpawnLocation;


	// 에디터에서 골라줄 VFX
	UPROPERTY(EditDefaultsOnly, Category = "VFX")
	UNiagaraSystem* LaserVFX;

	virtual void BeginPlay() override;

	UFUNCTION()
	void OnHit(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);
};
