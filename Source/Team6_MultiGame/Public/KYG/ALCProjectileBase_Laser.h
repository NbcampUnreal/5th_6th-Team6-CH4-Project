// ALCProjectileBase_Laser.h

#pragma once

#include "CoreMinimal.h"
#include "KYG/ALCProjectileBase.h"
#include "NiagaraSystem.h"
#include "ALCProjectileBase_Laser.generated.h"

class UNiagaraComponent;

UCLASS()
class TEAM6_MULTIGAME_API AALCProjectileBase_Laser : public AALCProjectileBase
{
	GENERATED_BODY()
	
public:

	AALCProjectileBase_Laser();
	
	// 레이저는 날아가는 탄 이 아니라 발사 순간 처리 //기존 Init를 건베이스에서 추상화 시켜야됨.
	virtual void Init(float InDamage, FVector Direction, float Speed) override;

protected:
	// 데미지 판정에 사용할 트레이스 채널 필요하면 BP에서 바꿀 수 있게
	UPROPERTY(EditDefaultsOnly, Category = "Laser")
	TEnumAsByte<ECollisionChannel> TraceChannel = ECC_Pawn;

	// 레이저가 맞을 수 있는 대상 필터 WorldStatic을 "막는 벽"으로 취급
	UPROPERTY(EditDefaultsOnly, Category = "Laser")
	bool bWorldStaticBlocksLaser = true;

	// 에디터에서 골라줄 VFX
	UPROPERTY(EditDefaultsOnly, Category = "VFX")
	UNiagaraSystem* LaserVFX;

	// 모든 클라에 이펙트만 전파 (데미지는 서버에서 이미 처리)
	UFUNCTION(NetMulticast, Reliable)
	void MulticastPlayLaserFX(const FVector& Start, const FVector& End);
};
