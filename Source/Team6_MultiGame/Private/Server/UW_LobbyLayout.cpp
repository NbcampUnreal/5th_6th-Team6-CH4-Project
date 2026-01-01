

#include "Server/UW_LobbyLayout.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Server/LobbyPlayerController.h"
#include "Server/LobbyPlayerState.h" 


void UUW_LobbyLayout::NativeConstruct()
{
	Super::NativeConstruct();

	if (ReadyButton)
	{
		ReadyButton->OnClicked.AddDynamic(this, &ThisClass::ReadyButtonClicked);
		ReadyButton->SetIsEnabled(false);
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(RefreshTimerHandle, this, &ThisClass::RefreshReadyText, 0.1f, true);
	}

	RefreshReadyText();
}

void UUW_LobbyLayout::NativeDestruct()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(RefreshTimerHandle);
	}
	Super::NativeDestruct();
}

void UUW_LobbyLayout::ReadyButtonClicked()
{
	UE_LOG(LogTemp, Warning, TEXT("[LobbyUI] Ready clicked"));

	if (ALobbyPlayerController* PC = GetOwningPlayer<ALobbyPlayerController>())
	{
		UE_LOG(LogTemp, Warning, TEXT("[LobbyUI] OwningPC=%s"), *GetNameSafe(PC));

		PC->ToggleReady();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[LobbyUI] OwningPlayer is not ALobbyPlayerController (null or wrong class)."));
	}

	RefreshReadyText();
}

void UUW_LobbyLayout::RefreshReadyText()
{
	if (!ReadyStateText)
	{
		return;
	}


	APlayerController* PC = GetOwningPlayer();
	APlayerState* BasePS = GetOwningPlayerState();
	UE_LOG(LogTemp, Warning, TEXT("[LobbyUI] PC=%s (%s) / PS=%s (%s)"),
		*GetNameSafe(PC), PC ? *GetNameSafe(PC->GetClass()) : TEXT("None"),
		*GetNameSafe(BasePS), BasePS ? *GetNameSafe(BasePS->GetClass()) : TEXT("None"));


	if (const ALobbyPlayerState* PS = Cast<ALobbyPlayerState>(BasePS))
	{
		if (ReadyButton) ReadyButton->SetIsEnabled(true);

		ReadyStateText->SetText(PS->bReady
			? FText::FromString(TEXT("READY"))
			: FText::FromString(TEXT("NOT READY")));

		//GetWorld()->GetTimerManager().ClearTimer(RefreshTimerHandle);
	}
	else
	{
		ReadyStateText->SetText(FText::FromString(TEXT("SYNCING...")));
	}
}
