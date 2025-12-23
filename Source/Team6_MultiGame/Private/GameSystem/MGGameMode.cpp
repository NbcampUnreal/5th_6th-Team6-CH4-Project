#include "GameSystem/MGGameMode.h"
#include "GameSystem/MGGameState.h"
#include "Kismet/GameplayStatics.h"

AMGGameMode::AMGGameMode()
{
    GameStateClass = AMGGameState::StaticClass();
}

void AMGGameMode::BeginPlay()
{
    Super::BeginPlay();

    if (AMGGameState* GS = GetGameState<AMGGameState>())
    {
        GS->SetGamePhase(EMGGamePhase::Warmup);
    }

    FTimerHandle TimerHandle;
    GetWorldTimerManager().SetTimer(
        TimerHandle,
        this,
        &AMGGameMode::StartGame,
        5.0f,
        false
    );
}

void AMGGameMode::StartGame()
{
    if (AMGGameState* GS = GetGameState<AMGGameState>())
    {
        GS->SetGamePhase(EMGGamePhase::InProgress);
    }

    UE_LOG(LogTemp, Warning, TEXT("Game Started"));
}

void AMGGameMode::EndGame()
{
    if (AMGGameState* GS = GetGameState<AMGGameState>())
    {
        GS->SetGamePhase(EMGGamePhase::GameOver);
    }

    UE_LOG(LogTemp, Warning, TEXT("Game Over"));
}
