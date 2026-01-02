#include "CharacterGameMode/CharacterGameState.h"
#include "Net/UnrealNetwork.h"
#include "Character/Controller/MainPlayerController.h"
#include "GameFramework/PlayerState.h"


void ACharacterGameState::AddKillCount(int32 Delta)
{
	if (!HasAuthority())
		return;

	KillCount += Delta;
}

void ACharacterGameState::OnRep_KillCount()
{
	for (APlayerState* PS : PlayerArray)
	{
		if (!PS) continue;

		AController* OwnerController = Cast<AController>(PS->GetOwner());
		if (!OwnerController) continue;

		AMainPlayerController* PC = Cast<AMainPlayerController>(OwnerController);
		if (!PC) continue;

		if (PC->IsLocalController())
		{
			PC->UpdateHUD_KillCount(KillCount);
		}
	}
}

void ACharacterGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ACharacterGameState, KillCount);
}