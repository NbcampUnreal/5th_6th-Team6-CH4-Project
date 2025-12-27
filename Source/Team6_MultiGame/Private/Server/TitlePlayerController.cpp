
#include "Server/TitlePlayerController.h"
#include "GameFramework/PlayerState.h" //  추가
#include "Server/LobbyPlayerState.h"
#include "Server/TitleGameModeBase.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"




void ATitlePlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (!IsLocalController())
		return;

	if (UIWidgetClass)
	{
		UIWidgetInstance = CreateWidget<UUserWidget>(this, UIWidgetClass);
		if (UIWidgetInstance)
		{
			UIWidgetInstance->AddToViewport();

			FInputModeUIOnly Mode;
			Mode.SetWidgetToFocus(UIWidgetInstance->GetCachedWidget());
			SetInputMode(Mode);

			bShowMouseCursor = true;
		}
	}
}

void ATitlePlayerController::JoinServer(const FString& InAddress)
{
	FString Addr = InAddress;
	Addr.TrimStartAndEndInline();

	if (Addr.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("[TitlePC] JoinServer failed: empty address"));
		return;
	}

	// 타이틀 UI가 남아있지 않게 정리(권장)
	if (UIWidgetInstance)
	{
		UIWidgetInstance->RemoveFromParent();
		UIWidgetInstance = nullptr;
	}

	UE_LOG(LogTemp, Warning, TEXT("[TitlePC] ClientTravel -> %s"), *Addr);
	ClientTravel(Addr, ETravelType::TRAVEL_Absolute);
}