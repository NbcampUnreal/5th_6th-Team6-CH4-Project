

#include "Server/TitleGameModeBase.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "Server/LobbyPlayerState.h"
#include "Engine/World.h"

ATitleGameModeBase::ATitleGameModeBase()
{
	bUseSeamlessTravel = false;
}

void ATitleGameModeBase::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);


	if (ALobbyPlayerState* PS = NewPlayer ? NewPlayer->GetPlayerState<ALobbyPlayerState>() : nullptr)
	{
		PS->SetReady(false);
	}


	TryStartGame();
}

void ATitleGameModeBase::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	TryStartGame();
}

bool ATitleGameModeBase::AreAllPlayersReady() const
{
	const AGameStateBase* GS = GameState;
	if (!GS)
	{
		return false;
	}

	const int32 NumPlayers = GS->PlayerArray.Num();
	if (NumPlayers < MinPlayersToStart)
	{
		return false;
	}

	for (APlayerState* PSBase : GS->PlayerArray)
	{
		const ALobbyPlayerState* PS = Cast<ALobbyPlayerState>(PSBase);
		if (!PS) return false;

		if (!PS->bReady)
		{
			return false;
		}
	}
	return true;
}

void ATitleGameModeBase::TryStartGame()
{
	if (!HasAuthority())
	{
		return;
	}

	if (bGameStarting)
	{
		return;
	}

	if (AreAllPlayersReady())
	{
		bGameStarting = true;
		StartGame();
	}
}

void ATitleGameModeBase::StartGame()
{
	if (!HasAuthority())
	{
		return;
	}

	const FString TravelURL = MainMapPath;

	UE_LOG(LogTemp, Warning, TEXT("[LobbyGameMode] All Ready. ServerTravel -> %s"), *TravelURL);
	GetWorld()->ServerTravel(TravelURL);
}
