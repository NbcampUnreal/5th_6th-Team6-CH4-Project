// Fill out your copyright notice in the Description page of Project Settings.


#include "Server/TitleGameModeBase.h"
#include "GameFramework/GameStateBase.h"   //  AGameStateBase 정의 + PlayerArray 접근 가능
#include "GameFramework/PlayerState.h"     //  APlayerState 정의
#include "Server/LobbyPlayerState.h"       //  ALobbyPlayerState 정의 (Cast 대상)
#include "Engine/World.h"

ATitleGameModeBase::ATitleGameModeBase()
{
	// 로비용 클래스 지정(원하면 여기서)
	// PlayerControllerClass = ALobbyPlayerController::StaticClass();
	// PlayerStateClass     = ALobbyPlayerState::StaticClass();

	// “맵 이동 시 PC도 바뀌길 원하면” SeamlessTravel은 꺼두는 쪽이 단순함
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

	// 누가 나가서 “남은 사람 전원 ready”가 될 수도 있으니 다시 체크
	TryStartGame();
}

bool ATitleGameModeBase::AreAllPlayersReady() const
{
	const AGameStateBase* GS = GameState;
	if (!GS) return false;

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
	if (!HasAuthority()) return;
	if (bGameStarting) return;

	if (AreAllPlayersReady())
	{
		bGameStarting = true;
		StartGame();
	}
}

void ATitleGameModeBase::StartGame()
{
	if (!HasAuthority()) return;

	// 데디 서버라면 보통 ?listen 필요 없음
	const FString TravelURL = MainMapPath;

	UE_LOG(LogTemp, Warning, TEXT("[LobbyGameMode] All Ready. ServerTravel -> %s"), *TravelURL);
	GetWorld()->ServerTravel(TravelURL);
}
