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

	void HandleFire(const FRotator& AimRot);
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

	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Muzzle")
	FName MuzzleSocketName = TEXT("Muzzle"); // 캐릭터 스켈레탈 메쉬에 만든 소켓 이름

	// 소켓 없을 때 총 액터 기준 폴백 오프셋(총 앞쪽)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Muzzle")
	FVector MuzzleFallbackOffset = FVector(30.f, 0.f, 10.f);

	// 내부 유틸: 스폰 트랜스폼 계산
	void GetMuzzleTransform(const FRotator& AimRot, FVector& OutLoc, FRotator& OutRot, bool& bOutUsedSocket) const;

	float LastFireTime;

	/*UFUNCTION(Server, Reliable)
	void ServerFire();
	void ServerFire_Implementation();*/

	
};
