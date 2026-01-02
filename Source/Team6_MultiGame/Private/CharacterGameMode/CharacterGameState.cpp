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
    // 클라이언트에서 "자기 로컬 PC" 찾아 HUD 갱신
    UWorld* World = GetWorld();
    if (!World) return;

    for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
    {
        APlayerController* PC = It->Get();
        if (!PC || !PC->IsLocalController()) continue;

        if (AMainPlayerController* MPC = Cast<AMainPlayerController>(PC))
        {
            MPC->UpdateHUD_KillCount(KillCount);
        }
    }
}

void ACharacterGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ACharacterGameState, KillCount);
}