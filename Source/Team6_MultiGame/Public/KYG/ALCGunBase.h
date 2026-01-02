// AALCGunBase.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ALCGunBase.generated.h"

class UStaticMeshComponent;
class AALCProjectileBase;

UCLASS()
class TEAM6_MULTIGAME_API AALCGunBase : public AActor
{
	GENERATED_BODY()
	
public:	
	AALCGunBase();

	/*UFUNCTION(BlueprintCallable)
	void Fire();*/
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void BeginPlay() override;

	void HandleFire(const FVector& AimRot);


protected:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class AALCProjectileBase> ProjectileClass;

	UPROPERTY(EditDefaultsOnly)
	float Damage;

	UPROPERTY(EditDefaultsOnly)
	float BulletSpeed;

	UPROPERTY(EditDefaultsOnly)
	float FireCooldown;

	//메쉬 총구 위치.
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* GunMesh;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Muzzle")
	FName MuzzleSocketName = TEXT("Muzzle"); // 캐릭터 스켈레탈 메쉬에 만든 소켓 이름

	// 소켓 없을 때 총 액터 기준 폴백 오프셋(총 앞쪽)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Muzzle")
	FVector MuzzleFallbackOffset = FVector(30.f, 0.f, 10.f);


	float LastFireTime;

	/*UFUNCTION(Server, Reliable)
	void ServerFire();
	void ServerFire_Implementation();*/
	//최대 탄 수 일반총 20
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Ammo")
	int32 MaxAmmo = 20;
	
	// 현재 남은 탄 수 Replicate 해서 HUD 등에 쓸 수 있게
	UPROPERTY(Replicated, VisibleAnywhere, Category = "Weapon|Ammo")
	int32 CurrentAmmo = 0;

	//최대사거리
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Range")
	float MaxRange = 3000.f;
};
