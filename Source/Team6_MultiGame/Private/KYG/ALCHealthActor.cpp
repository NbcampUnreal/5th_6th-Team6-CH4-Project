// ALCHealthActor.cpp

#include "KYG/ALCHealthActor.h"

AALCHealthActor::AALCHealthActor()
{
	CurrentHP = MaxHP;
	PrimaryActorTick.bCanEverTick = false;
}

void AALCHealthActor::BeginPlay()
{
	Super::BeginPlay();
}

void AALCHealthActor::ReceiveDamage_Implementation(float DamageAmount, AActor* DamageCauser)
{
	if (!HasAuthority())
	{
		return;
	}

	CurrentHP -= DamageAmount;

	UE_LOG(LogTemp, Warning, TEXT("[%s] HP: %.1f"), *GetName(), DamageAmount, CurrentHP);

	if (CurrentHP <= 0.f)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] DEAD"), *GetName());

		Destroy();
	}
}


