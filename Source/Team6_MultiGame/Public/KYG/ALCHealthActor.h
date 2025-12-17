// ALCHealthActor.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "KYG/LCDamageable.h"				//대미지 코드 
#include "ALCHealthActor.generated.h"

UCLASS()
class TEAM6_MULTIGAME_API AALCHealthActor : public AActor, public ILCDamageable
{
	GENERATED_BODY()
	
public:	
	
	AALCHealthActor();

protected:

	virtual void BeginPlay() override;

	//체력
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
	float MaxHP = 100.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Health")
	float CurrentHP;

public:	

	virtual void ReceiveDamage_Implementation(float DamageAmount,AActor* DamageCauser)override;
};
