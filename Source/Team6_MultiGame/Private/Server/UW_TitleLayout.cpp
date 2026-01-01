

#include "Server/UW_TitleLayout.h"

#include "Components/Button.h"
#include "Components/EditableText.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Server/TitlePlayerController.h"
#include "Server/VoiceLobbySubsystem.h"
#include "Server/LoginSubsystem.h"


UUW_TitleLayout::UUW_TitleLayout(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UUW_TitleLayout::NativeConstruct()
{
    LoginButton.Get()->OnClicked.AddDynamic(this, &ThisClass::LoginButtonClicked);
    LobbyButton.Get()->OnClicked.AddDynamic(this, &ThisClass::LobbyButtonClicked);
	ExitButton.Get()->OnClicked.AddDynamic(this, &ThisClass::ExitButtonClicked);
}

void UUW_TitleLayout::LoginButtonClicked()
{
    if (UGameInstance* GI = GetGameInstance())
    {
        auto* LoginSS = GI->GetSubsystem<ULoginSubsystem>();
        if (LoginSS && !LoginSS->IsLoggedIn())
        {
            LoginSS->LoginEOS_AccountPortal(); //  일반 로그인
        }
    }
}

void UUW_TitleLayout::LobbyButtonClicked()
{
	const FString ServerAddr = TEXT("13.209.70.161:7777");
	UE_LOG(LogTemp, Warning, TEXT("Go Lobby: %s"), *ServerAddr);

    ATitlePlayerController* PC = GetOwningPlayer<ATitlePlayerController>();

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

    if (PC)
    {
        PC->JoinServer(ServerAddr);
    }
}

void UUW_TitleLayout::ExitButtonClicked()
{
	UKismetSystemLibrary::QuitGame(this, GetOwningPlayer(), EQuitPreference::Quit, false);
}
