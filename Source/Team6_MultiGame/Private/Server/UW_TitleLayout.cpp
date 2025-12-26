

#include "Server/UW_TitleLayout.h"

#include "Components/Button.h"
#include "Components/EditableText.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Server/TitlePlayerController.h"

UUW_TitleLayout::UUW_TitleLayout(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UUW_TitleLayout::NativeConstruct()
{
	PlayButton.Get()->OnClicked.AddDynamic(this, &ThisClass::LobbyButtonClicked);
	ExitButton.Get()->OnClicked.AddDynamic(this, &ThisClass::ExitButtonClicked);
}

void UUW_TitleLayout::LobbyButtonClicked()
{
	const FString ServerAddr = TEXT("13.209.70.161:7777");
	UE_LOG(LogTemp, Warning, TEXT("Go Lobby: %s"), *ServerAddr);

    // OwningPlayer 우선
    ATitlePlayerController* PC = GetOwningPlayer<ATitlePlayerController>();

    // 혹시 OwningPlayer가 비는 상황 대비 (안전장치)
    if (!PC)
    {
        if (UWorld* World = GetWorld())
        {
            PC = Cast<ATitlePlayerController>(World->GetFirstPlayerController());
        }
    }

    if (!PC)
    {
        UE_LOG(LogTemp, Error, TEXT("[Title] No valid TitlePlayerController."));
        return;
    }

    PC->JoinServer(ServerAddr);
}

void UUW_TitleLayout::ExitButtonClicked()
{
	UKismetSystemLibrary::QuitGame(this, GetOwningPlayer(), EQuitPreference::Quit, false);
}
