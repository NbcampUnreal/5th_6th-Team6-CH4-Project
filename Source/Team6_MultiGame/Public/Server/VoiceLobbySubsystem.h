

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "VoiceChat.h"
#include "VoiceLobbySubsystem.generated.h"

class IVoiceChat;
class IVoiceChatUser;

UCLASS()
class TEAM6_MULTIGAME_API UVoiceLobbySubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Voice")
    void EnsureVoiceReady();

    UFUNCTION(BlueprintCallable, Category = "Voice")
    void EnsureVoiceConnected(); // Initialize + Connect + CreateUser

    UFUNCTION(BlueprintCallable, Category = "Voice")
    void EnsureVoiceLoggedIn(const FString& PlayerName, const FString& TokenOrCredentials);

    UFUNCTION(BlueprintCallable, Category = "Voice")
    bool JoinVoiceRoom(const FString& InRoomId, const FString& InTokenOrCredentialsJson);

    UFUNCTION(BlueprintCallable, Category = "Voice")
    void JoinVoiceChannel(const FString& ChannelName, const FString& ChannelCredentialsJson);

    UFUNCTION(BlueprintCallable, Category = "Voice")
    void LeaveVoiceChannel(const FString& ChannelName);

private:

    void OnVoiceConnectComplete(const FVoiceChatResult& Result);
    void OnVoiceLoginComplete(const FString& PlayerName, const FVoiceChatResult& Result);
    void OnVoiceJoinChannelComplete(const FString& ChannelName, const FVoiceChatResult& Result);
    void OnVoiceLeaveChannelComplete(const FString& ChannelName, const FVoiceChatResult& Result);

    FString GetDefaultPlayerNamePUID() const;
    void TryProcessPending(); // Connect/Login 완료 시점에 Pending 처리

private:
    IVoiceChat* VoiceChat = nullptr;
    IVoiceChatUser* VoiceUser = nullptr;

    bool bVoiceReady = false;
    bool bVoiceConnected = false;
    bool bVoiceLoggedIn = false;

    bool bConnectRequested = false;
    bool bLoginRequested = false;

    FString PendingLoginPlayerName;
    FString PendingLoginCredentials;

    FString PendingChannelName;
    FString PendingChannelCreds;
};
