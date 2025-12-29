

#include "Server/LobbyPlayerController.h"

#include "Blueprint/UserWidget.h"
#include "Server/LobbyPlayerState.h"
#include "Server/LobbyGameModeBase.h"
#include "Server/VoiceLobbySubsystem.h"
#include "Server/LoginSubsystem.h"
#include "Engine/World.h"


void ALobbyPlayerController::BeginPlay()
{
    Super::BeginPlay();

    if (!IsLocalController())
    {
        return;
    }

    if (LobbyWidgetClass)
    {
        LobbyWidgetInstance = CreateWidget<UUserWidget>(this, LobbyWidgetClass);
        if (LobbyWidgetInstance)
        {
            LobbyWidgetInstance->AddToViewport();

            // UIOnly + Mouse cursor (포커스 강제 지정하면 Non-Focusable 경고가 날 수 있어 생략)
            FInputModeUIOnly Mode;
            SetInputMode(Mode);
            bShowMouseCursor = true;
        }
    }
    // === Voice Lobby init (Leader creates, others find/join) ===
    TryInitVoiceLobby();

    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            VoiceInitTimerHandle,
            this,
            &ThisClass::TryInitVoiceLobby,
            1.0f,
            true
        );
    }
}

void ALobbyPlayerController::TryInitVoiceLobby()
{
    if (UGameInstance* GI = GetGameInstance())
    {
        if (auto* LoginSS = GI->GetSubsystem<ULoginSubsystem>())
        {
            if (!LoginSS->IsLoggedIn())
            {
                UE_LOG(LogTemp, Warning, TEXT("[LobbyPC] Skip voice init: not logged in yet"));
                return;
            }
        }
    }

    if (!IsLocalController())
    {
        return;
    }

    if (bVoiceInitDone)
    {
        if (UWorld* World = GetWorld())
        {
            World->GetTimerManager().ClearTimer(VoiceInitTimerHandle);
        }
        return;
    }

    UGameInstance* GI = GetGameInstance();
    if (!GI)
    {
        return;
    }

    // 로그인 완료 전에는 Voice Lobby를 만들/찾을 수 없음
    if (ULoginSubsystem* LoginSS = GI->GetSubsystem<ULoginSubsystem>())
    {
        if (!LoginSS->IsLoggedIn())
        {
            UE_LOG(LogTemp, Warning, TEXT("[LobbyPC] Waiting EOS login for voice lobby..."));
            return;
        }
    }

    UVoiceLobbySubsystem* VoiceSS = GI->GetSubsystem<UVoiceLobbySubsystem>();
    if (!VoiceSS)
    {
        return;
    }

    // 이미 VOICE_LOBBY 세션이 있으면 완료
    if (VoiceSS->HasVoiceLobbySession())
    {
        bVoiceInitDone = true;
        UE_LOG(LogTemp, Warning, TEXT("[LobbyPC] Voice lobby ready (named session exists)"));
        return;
    }

    ALobbyPlayerState* LPS = GetPlayerState<ALobbyPlayerState>();
    if (!LPS)
    {
        UE_LOG(LogTemp, Warning, TEXT("[LobbyPC] Waiting PlayerState for voice lobby..."));
        return;
    }

    if (LPS->bIsLeader)
    {
        UE_LOG(LogTemp, Warning, TEXT("[LobbyPC] I am LEADER -> CreateVoiceLobby"));
        VoiceSS->CreateVoiceLobby(4);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("[LobbyPC] Not leader -> FindAndJoinVoiceLobby"));
        VoiceSS->FindAndJoinVoiceLobby();
    }
}

void ALobbyPlayerController::OnRep_PlayerState()
{
    Super::OnRep_PlayerState();

    if (!IsLocalController() || bTriedVoiceInit)
        return;

    CachedLobbyPS = GetPlayerState<ALobbyPlayerState>();
    if (!CachedLobbyPS)
        return;

    bTriedVoiceInit = true;

    // 여기서 TryInitVoiceLobby() 한번 호출
    TryInitVoiceLobby();
}

void ALobbyPlayerController::ToggleReady()
{
    ServerToggleReady();
}

void ALobbyPlayerController::ServerToggleReady_Implementation()
{
    UE_LOG(LogTemp, Warning, TEXT("[ServerToggleReady] CALLED. PC=%s"), *GetName());

    ALobbyPlayerState* PS = GetPlayerState<ALobbyPlayerState>();
    UE_LOG(LogTemp, Warning, TEXT("[ServerToggleReady] PS=%s Ready(before)=%d"),
        *GetNameSafe(PS), PS ? PS->bReady : -1);

    if (!PS) return;

    PS->SetReady(!PS->bReady);

    if (UWorld* World = GetWorld())
    {
        if (ALobbyGameModeBase* GM = Cast<ALobbyGameModeBase>(World->GetAuthGameMode()))
        {
            GM->TryStartGame();
        }
    }
}