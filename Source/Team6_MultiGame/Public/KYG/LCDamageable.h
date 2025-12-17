// LCDamageable.h

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "LCDamageable.generated.h"

//리플렉션/블루프린트용 껍데기
UINTERFACE(MinimalAPI)
class ULCDamageable : public UInterface
{
	GENERATED_BODY()
};

//실제 C++에서 구현할 인터페이스 본체
class TEAM6_MULTIGAME_API ILCDamageable
{
	GENERATED_BODY()

public:
	//총알이나 다른 공격에 맞았을 때 호출되는 함수
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Damage")
	void ReceiveDamage(float DamageAmount, AActor* DamageCauser);
};
