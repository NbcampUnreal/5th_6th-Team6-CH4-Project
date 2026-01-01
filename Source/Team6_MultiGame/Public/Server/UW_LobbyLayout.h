
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TimerManager.h"
#include "UW_LobbyLayout.generated.h"

class UButton;
class UTextBlock;

UCLASS()
class TEAM6_MULTIGAME_API UUW_LobbyLayout : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;

	virtual void NativeDestruct() override;

	UFUNCTION()
	void ReadyButtonClicked();

	void RefreshReadyText();

private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ReadyButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ReadyStateText;

	FTimerHandle RefreshTimerHandle;
};
