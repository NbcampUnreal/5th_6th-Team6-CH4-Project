

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Misc/Guid.h"
#include "LobbyGameModeBase.generated.h"


UCLASS()
class TEAM6_MULTIGAME_API ALobbyGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	ALobbyGameModeBase();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lobby")
	int32 MinPlayersToStart = 3;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lobby")
	FString MainMapPath = TEXT("/Game/Maps/MainMap");

	bool AreAllPlayersReady() const;

	void TryStartGame();

	void EnsureVoiceRoomId();

protected:
	bool bGameStarting = false;
	// Voice Lobby 리더가 이미 정해졌는지
	bool bLeaderAssigned = false;

	//  리더 지정 헬퍼
	void AssignLeaderIfNeeded();
	void StartGame();

	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;

private:
	FString VoiceRoomId;
};
