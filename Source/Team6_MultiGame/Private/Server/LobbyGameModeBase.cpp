

#include "Server/LobbyGameModeBase.h"

#include "Server/LobbyPlayerState.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "Engine/World.h"

ALobbyGameModeBase::ALobbyGameModeBase()
{
	bUseSeamlessTravel = false;

	// 로비에서 PlayerStateClass를 확실히 로비용으로
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
	if (!HasAuthority()) return;
	if (bGameStarting) return;

	if (AreAllPlayersReady())
	{
		bGameStarting = true;
		StartGame();
	}
}

void ALobbyGameModeBase::StartGame()
{
	if (!HasAuthority()) return;

	UE_LOG(LogTemp, Warning, TEXT("[LobbyGM] All Ready. ServerTravel -> %s"), *MainMapPath);
	GetWorld()->ServerTravel(MainMapPath);
}