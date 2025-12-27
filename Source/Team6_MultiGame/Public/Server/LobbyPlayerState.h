
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

	UPROPERTY(BlueprintAssignable, Category = "Lobby")
	FOnReadyChanged OnReadyChanged;

	// 서버에서만 호출하는 걸 권장(클라에서 호출해도 값은 서버가 아니면 의미 없음)
	void SetReady(bool bNewReady);

protected:
	UFUNCTION()
	void OnRep_Ready();

public:
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
