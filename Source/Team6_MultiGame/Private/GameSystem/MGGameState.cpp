#include "GameSystem/MGGameState.h"
#include "Net/UnrealNetwork.h"


AMGGameState::AMGGameState()
{
    CurrentPhase = EMGGamePhase::Warmup;
}

void AMGGameState::SetGamePhase(EMGGamePhase NewPhase)
{
    if (HasAuthority())
    {
        CurrentPhase = NewPhase;
        OnRep_GamePhase();
    }
}

void AMGGameState::OnRep_GamePhase()
{
    UE_LOG(LogTemp, Warning, TEXT("Game Phase Changed: %d"), (int32)CurrentPhase);
}

void AMGGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AMGGameState, CurrentPhase);
}
