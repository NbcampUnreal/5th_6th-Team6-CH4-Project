
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "Delegates/DelegateCombinations.h"
#include "LobbyPlayerState.generated.h"

class ALobbyPlayerState;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
    FOnReadyChanged,
    ALobbyPlayerState*, PlayerState,
    bool, bReady
);

UCLASS()
class TEAM6_MULTIGAME_API ALobbyPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	ALobbyPlayerState();

	UPROPERTY(ReplicatedUsing = OnRep_Ready, BlueprintReadOnly, Category = "Lobby")
	bool bReady = false;

	// 첫 접속자(리더)만 true: Voice Lobby 생성 책임
	UPROPERTY(ReplicatedUsing = OnRep_IsLeader, BlueprintReadOnly, Category = "Lobby")
	bool bIsLeader = false;

	UPROPERTY(BlueprintAssignable, Category = "Lobby")
	FOnReadyChanged OnReadyChanged;

	UFUNCTION(BlueprintPure, Category = "Lobby")
	bool IsLeader() const { return bIsLeader; }

	// 서버에서만 호출하는 걸 권장(클라에서 호출해도 값은 서버가 아니면 의미 없음)
	void SetReady(bool bNewReady);

	// 서버에서만 호출(게임모드에서 리더 지정)
	void SetIsLeader(bool bNewLeader);

protected:
	UFUNCTION()
	void OnRep_Ready();

	UFUNCTION()
	void OnRep_IsLeader();

public:
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
