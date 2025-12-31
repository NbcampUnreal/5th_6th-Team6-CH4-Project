

#include "Server/VoiceLobbySubsystem.h"

#include "VoiceChat.h"
#include "VoiceChatResult.h"
#include "EOSVoiceChatTypes.h"
#include "HAL/PlatformMisc.h"
#include "OnlineSubsystem.h"
#include "Policies/CondensedJsonPrintPolicy.h"
#include "Interfaces/OnlineIdentityInterface.h"

static FString NormalizeClientBaseUrl(const FString& In)
{
    FString Url = In;
    Url.TrimStartAndEndInline();
    Url.ReplaceInline(TEXT("\r"), TEXT(""));
    Url.ReplaceInline(TEXT("\n"), TEXT(""));
    return Url;
}

static FString GetEOSAuthToken(int32 LocalUserNum = 0)
{
    if (IOnlineSubsystem* OSS = IOnlineSubsystem::Get(TEXT("EOS")))
    {
        if (IOnlineIdentityPtr Identity = OSS->GetIdentityInterface())
        {
            return Identity->GetAuthToken(LocalUserNum);
        }
    }
    return TEXT("");
}

static FString GetLocalUniqueIdString_Full()
{
    IOnlineSubsystem* OSS = IOnlineSubsystem::Get(TEXT("EOS"));
    IOnlineIdentityPtr Identity = OSS ? OSS->GetIdentityInterface() : nullptr;
    TSharedPtr<const FUniqueNetId> NetId = Identity.IsValid() ? Identity->GetUniquePlayerId(0) : nullptr;
    return NetId.IsValid() ? NetId->ToString() : TEXT("");
}

static bool RebuildVoiceCredsJson(const FString& InJson, FString& OutJson)
{
    TSharedPtr<FJsonObject> Obj;
    const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(InJson);

    if (!FJsonSerializer::Deserialize(Reader, Obj) || !Obj.IsValid())
        return false;

    FString ClientBaseUrl, ParticipantToken;

    Obj->TryGetStringField(TEXT("ClientBaseUrl"), ClientBaseUrl);
    if (ClientBaseUrl.IsEmpty())
    {
        Obj->TryGetStringField(TEXT("clientBaseUrl"), ClientBaseUrl);
    }

    Obj->TryGetStringField(TEXT("ParticipantToken"), ParticipantToken);
    if (ParticipantToken.IsEmpty())
    {
        Obj->TryGetStringField(TEXT("participantToken"), ParticipantToken);
    }

    ClientBaseUrl = NormalizeClientBaseUrl(ClientBaseUrl);
    ParticipantToken.TrimStartAndEndInline();

    if (ClientBaseUrl.IsEmpty() || ParticipantToken.IsEmpty())
    {
        return false;
    }

    FEOSVoiceChatChannelCredentials Creds;
    Creds.ClientBaseUrl = ClientBaseUrl;
    Creds.ParticipantToken = ParticipantToken;

    OutJson = Creds.ToJson();

    UE_LOG(LogTemp, Warning, TEXT("[Diag] Using FULL base url = %s"), *ClientBaseUrl.Left(200));

    return true;
}

void UVoiceLobbySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    VoiceChat = nullptr;
    VoiceUser = nullptr;

    bVoiceReady = false;
    bVoiceConnected = false;
    bVoiceLoggedIn = false;

    bConnectRequested = false;
    bLoginRequested = false;

    PendingLoginPlayerName.Empty();
    PendingLoginCredentials.Empty();
    PendingChannelName.Empty();
    PendingChannelCreds.Empty();
}

void UVoiceLobbySubsystem::Deinitialize()
{
    VoiceUser = nullptr;
    VoiceChat = nullptr;

    Super::Deinitialize();
}

void UVoiceLobbySubsystem::EnsureVoiceReady()
{
    if (bVoiceReady)
    {
        return;
    }

    VoiceChat = IVoiceChat::Get();
    if (!VoiceChat)
    {
        UE_LOG(LogTemp, Error, TEXT("[Voice] IVoiceChat::Get() failed"));
        return;
    }

    if (!VoiceChat->Initialize())
    {
        UE_LOG(LogTemp, Error, TEXT("[Voice] Initialize failed"));
        return;
    }

    VoiceUser = VoiceChat->CreateUser();
    if (!VoiceUser)
    {
        UE_LOG(LogTemp, Error, TEXT("[Voice] CreateUser failed"));
        return;
    }

    bVoiceReady = true;
    UE_LOG(LogTemp, Warning, TEXT("[Voice] Ready=1"));
}

bool UVoiceLobbySubsystem::JoinVoiceRoom(const FString& InRoomId, const FString& InTokenOrCredentialsJson)
{
    UE_LOG(LogTemp, Warning, TEXT("[VoiceToken] JoinChannel about to call. len=%d head=%s"),
        InTokenOrCredentialsJson.Len(),
        *InTokenOrCredentialsJson.Left(200)
    );

    JoinVoiceChannel(InRoomId, InTokenOrCredentialsJson);
    return bVoiceLoggedIn;
}

void UVoiceLobbySubsystem::EnsureVoiceConnected()
{
    EnsureVoiceReady();
    if (!bVoiceReady || !VoiceChat)
    {
        return;
    }
    if (bVoiceConnected)
    {
        return;
    }
    if (bConnectRequested)
    {
        return;
    }

    bConnectRequested = true;

    VoiceChat->Connect(
        FOnVoiceChatConnectCompleteDelegate::CreateUObject(this, &ThisClass::OnVoiceConnectComplete)
    );
}

FString UVoiceLobbySubsystem::GetDefaultPlayerNamePUID() const
{
    IOnlineSubsystem* OSS = IOnlineSubsystem::Get(TEXT("EOS"));
    IOnlineIdentityPtr Identity = OSS ? OSS->GetIdentityInterface() : nullptr;

    TSharedPtr<const FUniqueNetId> NetId = Identity.IsValid() ? Identity->GetUniquePlayerId(0) : nullptr;
    FString IdStr = NetId.IsValid() ? NetId->ToString() : TEXT("");

    FString ProductUserIdStr = IdStr;
    IdStr.Split(TEXT("|"), nullptr, &ProductUserIdStr);

    return ProductUserIdStr;
}

void UVoiceLobbySubsystem::TryProcessPending()
{
    if (!bVoiceConnected)
    {
        return;
    }

    if (bLoginRequested && !bVoiceLoggedIn && VoiceUser)
    {
        const FPlatformUserId PlatformId = FPlatformMisc::GetPlatformUserForUserIndex(0);

        VoiceUser->Login(
            PlatformId,
            PendingLoginPlayerName,
            PendingLoginCredentials,
            FOnVoiceChatLoginCompleteDelegate::CreateUObject(this, &ThisClass::OnVoiceLoginComplete)
        );

        bLoginRequested = false;
        return;
    }

    if (bVoiceLoggedIn && !PendingChannelName.IsEmpty() && VoiceUser)
    {
        VoiceUser->JoinChannel(
            PendingChannelName,
            PendingChannelCreds,
            EVoiceChatChannelType::NonPositional,
            FOnVoiceChatChannelJoinCompleteDelegate::CreateUObject(this, &ThisClass::OnVoiceJoinChannelComplete)
        );
    }
}

void UVoiceLobbySubsystem::OnVoiceConnectComplete(const FVoiceChatResult& Result)
{
    bVoiceConnected = Result.IsSuccess();
    UE_LOG(LogTemp, Warning, TEXT("[Voice] ConnectComplete success=%d"),
        bVoiceConnected ? 1 : 0);

    TryProcessPending();
}

void UVoiceLobbySubsystem::EnsureVoiceLoggedIn(const FString& PlayerName, const FString& TokenOrCredentials)
{
    EnsureVoiceReady();
    if (!bVoiceReady || !VoiceUser)
    {
        return;
    }

    if (bVoiceLoggedIn)
    {
        return;
    }

    EnsureVoiceConnected();

    FString FinalName = PlayerName;
    if (FinalName.IsEmpty())
    {
        FinalName = GetDefaultPlayerNamePUID();
        UE_LOG(LogTemp, Warning, TEXT("[Voice] PlayerName(PUID)='%s'"), *FinalName);
    }
    if (FinalName.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("[Voice] Login deferred: PUID empty (EOS login not ready)"));
        return;
    }

    FString FinalCreds = TokenOrCredentials;
    if (FinalCreds.IsEmpty())
    {
        FinalCreds = GetEOSAuthToken(0);
        UE_LOG(LogTemp, Warning, TEXT("[Voice] AuthTokenLen=%d"), FinalCreds.Len());
    }

    if (FinalCreds.IsEmpty())
    {
        UE_LOG(LogTemp, Error, TEXT("[Voice] Login deferred: AuthToken empty (EOS Identity not logged in?)"));
        return;
    }

    PendingLoginPlayerName = FinalName;
    PendingLoginCredentials = FinalCreds;
    bLoginRequested = true;

    TryProcessPending();
}

void UVoiceLobbySubsystem::JoinVoiceChannel(const FString& ChannelName, const FString& ChannelCredentialsJson)
{
    EnsureVoiceConnected();
    EnsureVoiceLoggedIn(TEXT(""), TEXT(""));



    FString CleanCreds;
    if (!RebuildVoiceCredsJson(ChannelCredentialsJson, CleanCreds))
    {
        UE_LOG(LogTemp, Error, TEXT("[VoiceToken] RebuildVoiceCredsJson FAILED"));
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("[VoiceToken] CleanCreds len=%d head=%s"),
        CleanCreds.Len(), *CleanCreds.Left(200));

    UE_LOG(LogTemp, Warning, TEXT("[Diag] Has client_base_url=%d participant_token=%d"),
        CleanCreds.Contains(TEXT("\"client_base_url\"")) ? 1 : 0,
        CleanCreds.Contains(TEXT("\"participant_token\"")) ? 1 : 0);

    {
        TSharedPtr<FJsonObject> Obj;
        const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(CleanCreds);
        if (FJsonSerializer::Deserialize(Reader, Obj) && Obj.IsValid())
        {
            FString BaseUrl, Tok, Override;
            Obj->TryGetStringField(TEXT("ClientBaseUrl"), BaseUrl);
            Obj->TryGetStringField(TEXT("ParticipantToken"), Tok);
            Obj->TryGetStringField(TEXT("OverrideUserId"), Override);

            UE_LOG(LogTemp, Warning, TEXT("[Diag] BaseUrl=%s"), *BaseUrl);
            UE_LOG(LogTemp, Warning, TEXT("[Diag] TokenLen=%d"), Tok.Len());
            UE_LOG(LogTemp, Warning, TEXT("[Diag] OverrideUserId=%s"), *Override);
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("[Diag] CleanCreds JSON parse FAILED"));
        }
    }

    if (!VoiceUser || !bVoiceConnected || !bVoiceLoggedIn)
    {
        PendingChannelName = ChannelName;
        PendingChannelCreds = CleanCreds;
        return;
    }

    VoiceUser->JoinChannel(
        ChannelName,
        CleanCreds,
        EVoiceChatChannelType::NonPositional,
        FOnVoiceChatChannelJoinCompleteDelegate::CreateUObject(this, &ThisClass::OnVoiceJoinChannelComplete)
    );
}

void UVoiceLobbySubsystem::LeaveVoiceChannel(const FString& ChannelName)
{
    if (!VoiceUser)
    {
        return;
    }

    VoiceUser->LeaveChannel(
        ChannelName,
        FOnVoiceChatChannelLeaveCompleteDelegate::CreateUObject(this, &ThisClass::OnVoiceLeaveChannelComplete)
    );
}

void UVoiceLobbySubsystem::OnVoiceLoginComplete(const FString& PlayerName, const FVoiceChatResult& Result)
{
    bVoiceLoggedIn = Result.IsSuccess();

    IOnlineSubsystem* OSS = IOnlineSubsystem::Get(TEXT("EOS"));
    IOnlineIdentityPtr Id = OSS ? OSS->GetIdentityInterface() : nullptr;

    FString OSSUnique = TEXT("NONE");
    if (Id.IsValid())
    {
        TSharedPtr<const FUniqueNetId> NetId = Id->GetUniquePlayerId(0);
        if (NetId.IsValid())
        {
            OSSUnique = NetId->ToString();
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("[Diag] VoiceLogin user(PlayerName)=%s success=%d OSSUnique=%s"),
        *PlayerName, Result.IsSuccess() ? 1 : 0, *OSSUnique);

    UE_LOG(LogTemp, Warning, TEXT("[Voice] LoginComplete user=%s success=%d"),
        *PlayerName, bVoiceLoggedIn ? 1 : 0);

    TryProcessPending();
}

void UVoiceLobbySubsystem::OnVoiceJoinChannelComplete(const FString& ChannelName, const FVoiceChatResult& Result)
{
    UE_LOG(LogTemp, Warning, TEXT("[Voice] JoinChannelComplete ch=%s success=%d"),
        *ChannelName, Result.IsSuccess() ? 1 : 0);
}

void UVoiceLobbySubsystem::OnVoiceLeaveChannelComplete(const FString& ChannelName, const FVoiceChatResult& Result)
{
    UE_LOG(LogTemp, Warning, TEXT("[Voice] LeaveChannelComplete ch=%s success=%d"),
        *ChannelName, Result.IsSuccess() ? 1 : 0);
}

static FString GetEOSProductUserIdString(int32 LocalUserNum = 0)
{
    IOnlineSubsystem* OSS = IOnlineSubsystem::Get(TEXT("EOS"));
    IOnlineIdentityPtr Identity = OSS ? OSS->GetIdentityInterface() : nullptr;

    TSharedPtr<const FUniqueNetId> NetId = Identity.IsValid() ? Identity->GetUniquePlayerId(LocalUserNum) : nullptr;
    FString IdStr = NetId.IsValid() ? NetId->ToString() : TEXT("");

    FString ProductUserIdStr = IdStr;
    IdStr.Split(TEXT("|"), nullptr, &ProductUserIdStr);

    return ProductUserIdStr;
}