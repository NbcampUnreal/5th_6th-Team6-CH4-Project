

#include "Server/UW_LobbyLayout.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Server/LobbyPlayerState.h"

void UUW_LobbyLayout::NativeConstruct()
{
	Super::NativeConstruct();

	if (ReadyButton)
	{
		ReadyButton->OnClicked.AddDynamic(this, &ThisClass::ReadyButtonClicked);
	}

	RefreshReadyText();
}

void UUW_LobbyLayout::ReadyButtonClicked()
{

}

void UUW_LobbyLayout::RefreshReadyText()
{
	if (!ReadyStateText) return;

	if (const ALobbyPlayerState* PS = GetOwningPlayerState<ALobbyPlayerState>())
	{
		ReadyStateText->SetText(PS->bReady ? FText::FromString(TEXT("READY")) : FText::FromString(TEXT("NOT READY")));
	}
	else
	{
		ReadyStateText->SetText(FText::FromString(TEXT("NO PLAYERSTATE")));
	}
}
