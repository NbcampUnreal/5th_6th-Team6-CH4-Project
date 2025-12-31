//ALCHealingActor.cpp

#include "KYG/ALCHealingActor.h"
#include "KYG/LCHealable.h"
#include "GameFramework/Character.h"

AALCHealingActor::AALCHealingActor()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AALCHealingActor::OnPickedUp(ACharacter* Character)
{
	if (!HasAuthority() || !Character) 
	{ return; }

	// Healable 인터페이스를 구현한 대상만 회복
	if (Character->GetClass()->ImplementsInterface(ULCHealable::StaticClass()))
	{
		ILCHealable::Execute_ReceiveHeal(Character, HealAmount);

		UE_LOG(LogTemp, Warning, TEXT("[HealItem] %s healed by %.1f"), *Character->GetName(), HealAmount);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[HealItem] %s is not healable"), *Character->GetName());
	}
}
