// ALCProjectileBase_Laser.h

#pragma once

#include "CoreMinimal.h"
#include "KYG/ALCProjectileBase.h"
#include "ALCProjectileBase_Laser.generated.h"

UCLASS()
class TEAM6_MULTIGAME_API AALCProjectileBase_Laser : public AALCProjectileBase
{
	GENERATED_BODY()
	
public:
	// 레이저는 날아가는 탄 이 아니라 발사 순간 처리 //기존 Init를 건베이스에서 추상화 시켜야됨.
	virtual void Init(float InDamage, FVector Direction, float Speed) override;

protected:
	// 데미지 판정에 사용할 트레이스 채널 필요하면 BP에서 바꿀 수 있게
	UPROPERTY(EditDefaultsOnly, Category = "Laser")
	TEnumAsByte<ECollisionChannel> TraceChannel = ECC_Visibility;

	// 레이저가 맞을 수 있는 대상 필터 WorldStatic을 "막는 벽"으로 취급
	UPROPERTY(EditDefaultsOnly, Category = "Laser")
	bool bWorldStaticBlocksLaser = true;

};
