

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

void ALobbyGameModeBase::AssignLeaderIfNeeded()
{
    if (!HasAuthority())
    {
        return;
    }

    AGameStateBase* GS = GameState;
    if (!GS)
    {
        return;
    }

    // 현재 리더가 존재하는지 확인
    ALobbyPlayerState* CurrentLeader = nullptr;
    for (APlayerState* PSBase : GS->PlayerArray)
    {
        if (ALobbyPlayerState* LPS = Cast<ALobbyPlayerState>(PSBase))
        {
            if (LPS->bIsLeader)
            {
                CurrentLeader = LPS;
                break;
            }
        }
    }

    if (CurrentLeader)
    {
        bLeaderAssigned = true;
        return; // 이미 리더 있음
    }

    // 리더가 없으면 첫 번째 플레이어를 리더로 지정
    for (APlayerState* PSBase : GS->PlayerArray)
    {
        if (ALobbyPlayerState* LPS = Cast<ALobbyPlayerState>(PSBase))
        {
            // 안전하게 전체 false 초기화 후 하나만 true
            for (APlayerState* PSBase2 : GS->PlayerArray)
            {
                if (ALobbyPlayerState* LPS2 = Cast<ALobbyPlayerState>(PSBase2))
                {
                    LPS2->SetIsLeader(false);
                    LPS2->ForceNetUpdate();
                }
            }

            LPS->SetIsLeader(true);
            LPS->ForceNetUpdate();

            bLeaderAssigned = true;

            UE_LOG(LogTemp, Warning, TEXT("[LobbyGM] Leader assigned -> %s"), *GetNameSafe(LPS));
            return;
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("[LobbyGM] AssignLeaderIfNeeded: no valid LobbyPlayerState yet"));
}

void ALobbyGameModeBase::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	if (ALobbyPlayerState* PS = NewPlayer ? NewPlayer->GetPlayerState<ALobbyPlayerState>() : nullptr)
	{
		PS->SetReady(false);
        EnsureVoiceRoomId();
        PS->SetVoiceRoomId(VoiceRoomId);
	}

    // 추가: 리더 지정
    AssignLeaderIfNeeded();

	TryStartGame();
}

void ALobbyGameModeBase::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

    //  리더가 나갔을 수도 있으니, 다시 리더 확인/재지정
    bLeaderAssigned = false;
    AssignLeaderIfNeeded();

	TryStartGame();
}

bool ALobbyGameModeBase::AreAllPlayersReady() const
{
	const AGameStateBase* GS = GameState;
    if (!GS)
    {
        return false;
    }

    if (GS->PlayerArray.Num() < MinPlayersToStart)
    {
        return false;
    }

	for (APlayerState* PSBase : GS->PlayerArray)
	{
		const ALobbyPlayerState* LPS = Cast<ALobbyPlayerState>(PSBase);

        if (!LPS)
        {
            return false;
        }

        if (!LPS->bReady)
        {
            return false;
        }
	}

	return true;
}

void ALobbyGameModeBase::TryStartGame()
{
	UE_LOG(LogTemp, Warning, TEXT("[LobbyGM] TryStartGame called. HasAuth=%d"), HasAuthority());

    if (!HasAuthority())
    {
        return;
    }

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
    if (!HasAuthority())
    {
        return;
    }

	const FString TravelURL = TEXT("/Game/Maps/L_WaveMap");
	UE_LOG(LogTemp, Warning, TEXT("[LobbyGM] ServerTravel -> %s"), *TravelURL);

	const bool bOk = GetWorld()->ServerTravel(TravelURL);
	UE_LOG(LogTemp, Warning, TEXT("[LobbyGM] ServerTravel returned %d"), bOk);
}

void ALobbyGameModeBase::EnsureVoiceRoomId()
{
    if (!HasAuthority())
    {
        return;
    }

    if (VoiceRoomId.IsEmpty())
    {
        // 매치마다 고유 RoomId
        VoiceRoomId = FString::Printf(TEXT("TEAM6_%s"), *FGuid::NewGuid().ToString(EGuidFormats::Digits));
        UE_LOG(LogTemp, Warning, TEXT("[LobbyGM] VoiceRoomId created: %s"), *VoiceRoomId);
    }
}