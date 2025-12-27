

#include "Server/LobbyGameModeBase.h"

#include "Server/LobbyPlayerState.h"
#include "Server/LobbyPlayerController.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "Engine/World.h"

ALobbyGameModeBase::ALobbyGameModeBase()
{
	bUseSeamlessTravel = false;

	PlayerControllerClass = ALobbyPlayerController::StaticClass();
	PlayerStateClass = ALobbyPlayerState::StaticClass();
}

void ALobbyGameModeBase::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	if (ALobbyPlayerState* PS = NewPlayer ? NewPlayer->GetPlayerState<ALobbyPlayerState>() : nullptr)
	{
		PS->SetReady(false);
	}

	TryStartGame();
}

void ALobbyGameModeBase::Logout(AController* Exiting)
{
	Super::Logout(Exiting);
	TryStartGame();
}

bool ALobbyGameModeBase::AreAllPlayersReady() const
{
	const AGameStateBase* GS = GameState;
	if (!GS) return false;

	if (GS->PlayerArray.Num() < MinPlayersToStart)
		return false;

	for (APlayerState* PSBase : GS->PlayerArray)
	{
		const ALobbyPlayerState* LPS = Cast<ALobbyPlayerState>(PSBase);
		if (!LPS) return false;
		if (!LPS->bReady) return false;
	}

	return true;
}

void ALobbyGameModeBase::TryStartGame()
{
	UE_LOG(LogTemp, Warning, TEXT("[LobbyGM] TryStartGame called. HasAuth=%d"), HasAuthority());

	if (!HasAuthority()) return;
	if (bGameStarting)
	{
		UE_LOG(LogTemp, Warning, TEXT("[LobbyGM] Already starting."));
		return;
	}

	const bool bAllReady = AreAllPlayersReady();
	UE_LOG(LogTemp, Warning, TEXT("[LobbyGM] AreAllPlayersReady=%d (MinPlayers=%d)"), bAllReady, MinPlayersToStart);

	if (bAllReady)
	{
		bGameStarting = true;
		StartGame();
	}
}

void ALobbyGameModeBase::StartGame()
{
	if (!HasAuthority()) return;

	const FString TravelURL = TEXT("/Game/Maps/L_WaveMap");
	UE_LOG(LogTemp, Warning, TEXT("[LobbyGM] ServerTravel -> %s"), *TravelURL);

	const bool bOk = GetWorld()->ServerTravel(TravelURL);
	UE_LOG(LogTemp, Warning, TEXT("[LobbyGM] ServerTravel returned %d"), bOk);
}