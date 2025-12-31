

#include "Server/VoiceLobbySubsystem.h"

#include "VoiceChat.h"
#include "VoiceChatResult.h"
#include "HAL/PlatformMisc.h"
#include "OnlineSubsystem.h"
#include "Policies/CondensedJsonPrintPolicy.h"
#include "Interfaces/OnlineIdentityInterface.h"

static FString NormalizeClientBaseUrl(const FString& In)
{
    FString Url = In;
    Url.TrimStartAndEndInline();

    // JSON 파싱/로그에서 섞일 수 있는 개행 제거만
    Url.ReplaceInline(TEXT("\r"), TEXT(""));
    Url.ReplaceInline(TEXT("\n"), TEXT(""));

    //  절대 /ws?ms=... 같은 path/query를 잘라내지 마
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

    if (!Obj->TryGetStringField(TEXT("ClientBaseUrl"), ClientBaseUrl))
        Obj->TryGetStringField(TEXT("clientBaseUrl"), ClientBaseUrl);

    if (!Obj->TryGetStringField(TEXT("ParticipantToken"), ParticipantToken))
        Obj->TryGetStringField(TEXT("participantToken"), ParticipantToken);

    ClientBaseUrl.TrimStartAndEndInline();
    ParticipantToken.TrimStartAndEndInline();

    const FString BaseUrlToUse = ClientBaseUrl;

    UE_LOG(LogTemp, Warning, TEXT("[VoiceToken] BaseUrl RAW = %s"), *ClientBaseUrl);
    UE_LOG(LogTemp, Warning, TEXT("[VoiceToken] BaseUrl NORM= %s"), *BaseUrlToUse);

    if (BaseUrlToUse.IsEmpty() || ParticipantToken.IsEmpty())
        return false;

    const FString FullId = GetLocalUniqueIdString_Full();
    UE_LOG(LogTemp, Warning, TEXT("[VoiceToken] OverrideUserId=%s"), *FullId);

    TSharedRef<FJsonObject> Clean = MakeShared<FJsonObject>();
    Clean->SetStringField(TEXT("clientBaseUrl"), BaseUrlToUse);
    Clean->SetStringField(TEXT("participantToken"), ParticipantToken);

    if (!FullId.IsEmpty())
    {
        Clean->SetStringField(TEXT("OverrideUserId"), FullId);
    }
 
    FString Out;
    const TSharedRef<TJsonWriter<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>> Writer =
        TJsonWriterFactory<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>::Create(&Out);
    FJsonSerializer::Serialize(Clean, Writer);

    OutJson = Out;
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
    // 엔진/프로바이더마다 Logout/Disconnect 시그니처가 달라서 여기서는 안전하게 포인터만 정리
    VoiceUser = nullptr;
    VoiceChat = nullptr;
    // 필요 시 Leave/Logout/Disconnect 정리
    Super::Deinitialize();
}

void UVoiceLobbySubsystem::EnsureVoiceReady()
{
    if (bVoiceReady) return;

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
    // 외부(LobbyPC)에서 이미 이 이름으로 부르니까 유지.
    // 내부에서는 “로그인/커넥트 순서 보장 + pending join”만 처리한다.
    JoinVoiceChannel(InRoomId, InTokenOrCredentialsJson);
    return bVoiceLoggedIn; // 의미 있는 true는 “로그인 상태로 진입했는지” 정도로만 사용
}

void UVoiceLobbySubsystem::EnsureVoiceConnected()
{
    EnsureVoiceReady();
    if (!bVoiceReady || !VoiceChat) return;
    if (bVoiceConnected) return;
    if (bConnectRequested) return;

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

    // EOSPlus면 "EpicAccountId|ProductUserId"일 수 있어서 뒤만 사용
    FString ProductUserIdStr = IdStr;
    IdStr.Split(TEXT("|"), nullptr, &ProductUserIdStr);

    return ProductUserIdStr;
}

void UVoiceLobbySubsystem::TryProcessPending()
{
    // Connect 완료 전이면 Login/Join을 뒤로 미룸
    if (!bVoiceConnected)
        return;

    // Login 요청이 있으면 먼저 처리
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

    // 로그인 완료 + Join 대기 채널 있으면 Join 시도
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
    if (!bVoiceReady || !VoiceUser) return;
    if (bVoiceLoggedIn) return;

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
    PendingLoginCredentials = FinalCreds;   //  빈값 금지
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

    UE_LOG(LogTemp, Warning, TEXT("[Diag] Has clientBaseUrl=%d participantToken=%d"),
        CleanCreds.Contains(TEXT("\"clientBaseUrl\"")) ? 1 : 0,
        CleanCreds.Contains(TEXT("\"participantToken\"")) ? 1 : 0);

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
    if (!VoiceUser) return;

    VoiceUser->LeaveChannel(
        ChannelName,
        FOnVoiceChatChannelLeaveCompleteDelegate::CreateUObject(this, &ThisClass::OnVoiceLeaveChannelComplete)
    );
}

void UVoiceLobbySubsystem::OnVoiceLoginComplete(const FString& PlayerName, const FVoiceChatResult& Result)
{
    bVoiceLoggedIn = Result.IsSuccess();

    // === 추가: OSS 쪽 UniqueId 확인 ===
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

    // 기존 로그도 유지하고 싶으면 같이 둬도 됨
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

    // EOSPlus면 "EpicAccountId|ProductUserId" 형태일 수 있으니 뒤쪽만 사용
    FString ProductUserIdStr = IdStr;
    IdStr.Split(TEXT("|"), nullptr, &ProductUserIdStr);

    return ProductUserIdStr;
}