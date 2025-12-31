

#include "Server/LobbyPlayerController.h"

#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Misc/Base64.h"
#include "Misc/ConfigCacheIni.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
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

static FString ExtractIpOnly(const FString& InAddr)
{
    // "1.2.3.4:5678" or "[::1]:1234" 형태에서 IP만 대충 뽑기
    FString S = InAddr;
    S.ReplaceInline(TEXT("["), TEXT(""));
    S.ReplaceInline(TEXT("]"), TEXT(""));
    FString Left, Right;
    if (S.Split(TEXT(":"), &Left, &Right, ESearchCase::IgnoreCase, ESearchDir::FromEnd))
    {
        return Left;
    }
    return S;
}

static bool GetTeam6VoiceServerCreds(FString& OutDeploymentId, FString& OutClientId, FString& OutClientSecret)
{
    return
        GConfig->GetString(TEXT("Team6VoiceToken"), TEXT("DeploymentId"), OutDeploymentId, GEngineIni) &&
        GConfig->GetString(TEXT("Team6VoiceToken"), TEXT("ClientId"), OutClientId, GEngineIni) &&
        GConfig->GetString(TEXT("Team6VoiceToken"), TEXT("ClientSecret"), OutClientSecret, GEngineIni) &&
        !OutDeploymentId.IsEmpty() && !OutClientId.IsEmpty() && !OutClientSecret.IsEmpty();
}

static FString GetRequesterPuidFromPlayerState(APlayerState* PS)
{
    if (!PS) return TEXT("");
    const FUniqueNetIdRepl& Repl = PS->GetUniqueId();
    if (!Repl.IsValid()) return TEXT("");

    FString IdStr = Repl->ToString(); // 예: "EOS:EpicAccountId|ProductUserId" 등
    FString Puid = IdStr;

    // EOSPlus면 "EpicAccountId|ProductUserId"처럼 붙는 경우가 있어 뒤를 PUID로 간주
    IdStr.Split(TEXT("|"), nullptr, &Puid);
    // "EOS:" prefix가 남으면 제거 시도
    Puid.ReplaceInline(TEXT("EOS:"), TEXT(""));
    return Puid;
}

void ALobbyPlayerController::TryInitVoiceLobby()
{
    if (!IsLocalController())
    {
        return;
    }

    if (bVoiceInitDone)
    {
        return;
    }

    UGameInstance* GI = GetGameInstance();
    if (!GI)
    {
        return;
    }

    // 로그인 완료 전에는 VoiceChat Login 불가(너 프로젝트 흐름 기준)
    if (ULoginSubsystem* LoginSS = GI->GetSubsystem<ULoginSubsystem>())
    {
        if (!LoginSS->IsLoggedIn())
        {
            UE_LOG(LogTemp, Warning, TEXT("[LobbyPC] Waiting EOS login for voice..."));
            return;
        }
    }

    ALobbyPlayerState* PS = GetPlayerState<ALobbyPlayerState>();
    if (!PS) return;

    const FString& RoomId = PS->GetVoiceRoomId();
    if (RoomId.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("[LobbyPC] Waiting VoiceRoomId replication..."));
        return;
    }

    // 1) VoiceChat 연결/로그인 준비
    if (UVoiceLobbySubsystem* VoiceSS = GI->GetSubsystem<UVoiceLobbySubsystem>())
    {
        VoiceSS->EnsureVoiceReady(); // 아래에서 만들어줄 함수(Initialize/Connect/CreateUser 등)
    }

    // 2) 토큰 요청 (Trusted Server 방식 자리)
    Server_RequestVoiceJoinToken(RoomId);
}

void ALobbyPlayerController::OnRep_PlayerState()
{
    Super::OnRep_PlayerState();

    if (!IsLocalController() || bTriedVoiceInit)
    {
        return;
    }

    CachedLobbyPS = GetPlayerState<ALobbyPlayerState>();
    if (!CachedLobbyPS)
    {
        return;
    }

    bTriedVoiceInit = true;
    TryInitVoiceLobby();
}

void ALobbyPlayerController::ToggleReady()
{
    ServerToggleReady();
}

void ALobbyPlayerController::Server_RequestVoiceJoinToken_Implementation(const FString& InRoomId)
{
    if (!HasAuthority())
        return;

    FString DeploymentId, ClientId, ClientSecret;
    if (!GetTeam6VoiceServerCreds(DeploymentId, ClientId, ClientSecret))
    {
        UE_LOG(LogTemp, Error, TEXT("[VoiceToken] Missing [Team6VoiceToken] creds on SERVER ini"));
        Client_ReceiveVoiceJoinToken(InRoomId, TEXT("")); // 실패
        return;
    }

    const FString Puid = GetRequesterPuidFromPlayerState(PlayerState);
    if (Puid.IsEmpty())
    {
        UE_LOG(LogTemp, Error, TEXT("[VoiceToken] PUID is empty (PlayerState UniqueId invalid)"));
        Client_ReceiveVoiceJoinToken(InRoomId, TEXT(""));
        return;
    }

    FString ClientIp = TEXT("0.0.0.0");
    if (UNetConnection* Conn = GetNetConnection())
    {
        ClientIp = ExtractIpOnly(Conn->LowLevelGetRemoteAddress(true));
    }

    //  (서버) 토큰 발급 요청 보내기 직전
    UE_LOG(LogTemp, Warning, TEXT("[DiagServer] IssueVoiceToken TargetPUID=%s Room=%s ClientIp=%s DeploymentId=%s"),
        *Puid, *InRoomId, *ClientIp, *DeploymentId);

    // 1) OAuth access token (client_credentials)
    const FString Basic = FBase64::Encode(ClientId + TEXT(":") + ClientSecret);

    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> TokenReq = FHttpModule::Get().CreateRequest();
    TokenReq->SetURL(TEXT("https://api.epicgames.dev/auth/v1/oauth/token"));
    TokenReq->SetVerb(TEXT("POST"));
    TokenReq->SetHeader(TEXT("Authorization"), FString::Printf(TEXT("Basic %s"), *Basic));
    TokenReq->SetHeader(TEXT("Content-Type"), TEXT("application/x-www-form-urlencoded"));

    TokenReq->SetContentAsString(
        FString::Printf(TEXT("grant_type=client_credentials&deployment_id=%s"), *DeploymentId));

    TWeakObjectPtr<ALobbyPlayerController> WeakThis(this);

    TokenReq->OnProcessRequestComplete().BindLambda(
        [WeakThis, InRoomId, DeploymentId, Puid, ClientIp](FHttpRequestPtr Req, FHttpResponsePtr Resp, bool bOk)
        {
            if (!WeakThis.IsValid()) return;

            const int32 Code = Resp.IsValid() ? Resp->GetResponseCode() : -1;
            const FString RespStr = Resp.IsValid() ? Resp->GetContentAsString() : TEXT("<no response>");

            //  OAuth 응답 받은 직후
            UE_LOG(LogTemp, Warning, TEXT("[DiagServer] VoiceToken(OAuth) bWasSuccessful=%d HTTP=%d"),
                bOk ? 1 : 0, Code);
            UE_LOG(LogTemp, Warning, TEXT("[DiagServer] VoiceToken(OAuth) RESP=%s"),
                *RespStr.Left(1000));

            if (!bOk || !Resp.IsValid() || Code / 100 != 2)
            {
                UE_LOG(LogTemp, Error, TEXT("[VoiceToken] OAuth failed. ok=%d code=%d body=%s"),
                    bOk ? 1 : 0,
                    Code,
                    *RespStr
                );
                WeakThis->Client_ReceiveVoiceJoinToken(InRoomId, TEXT(""));
                return;
            }

            FString AccessToken;
            {
                TSharedPtr<FJsonObject> Json;
                const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(RespStr);
                if (!FJsonSerializer::Deserialize(Reader, Json) || !Json.IsValid())
                {
                    UE_LOG(LogTemp, Error, TEXT("[VoiceToken] OAuth JSON parse failed: %s"), *RespStr);
                    WeakThis->Client_ReceiveVoiceJoinToken(InRoomId, TEXT(""));
                    return;
                }
                Json->TryGetStringField(TEXT("access_token"), AccessToken);
            }

            if (AccessToken.IsEmpty())
            {
                UE_LOG(LogTemp, Error, TEXT("[VoiceToken] OAuth access_token empty"));
                WeakThis->Client_ReceiveVoiceJoinToken(InRoomId, TEXT(""));
                return;
            }

            // 2) createRoomToken (Voice Web API / RTC)
            TSharedRef<IHttpRequest, ESPMode::ThreadSafe> VoiceReq = FHttpModule::Get().CreateRequest();
            VoiceReq->SetURL(FString::Printf(TEXT("https://api.epicgames.dev/rtc/v1/%s/room/%s"), *DeploymentId, *InRoomId));
            VoiceReq->SetVerb(TEXT("POST"));
            VoiceReq->SetHeader(TEXT("Authorization"), FString::Printf(TEXT("Bearer %s"), *AccessToken));
            VoiceReq->SetHeader(TEXT("Content-Type"), TEXT("application/json"));

            const FString Body = FString::Printf(
                TEXT("{\"participants\":[{\"puid\":\"%s\",\"clientIp\":\"%s\",\"hardMuted\":false}]}"),
                *Puid, *ClientIp
            );
            VoiceReq->SetContentAsString(Body);

            VoiceReq->OnProcessRequestComplete().BindLambda(
                [WeakThis, InRoomId, Puid, DeploymentId](FHttpRequestPtr Req2, FHttpResponsePtr Resp2, bool bOk2)
                {
                    if (!WeakThis.IsValid()) return;

                    const int32 Code2 = Resp2.IsValid() ? Resp2->GetResponseCode() : -1;
                    const FString RespStr2 = Resp2.IsValid() ? Resp2->GetContentAsString() : TEXT("<no response>");

                    //  createRoomToken 응답 받은 직후
                    UE_LOG(LogTemp, Warning, TEXT("[DiagServer] VoiceToken(Room) bWasSuccessful=%d HTTP=%d"),
                        bOk2 ? 1 : 0, Code2);
                    UE_LOG(LogTemp, Warning, TEXT("[DiagServer] VoiceToken(Room) RESP=%s"),
                        *RespStr2.Left(1000));

                    if (!bOk2 || !Resp2.IsValid() || Code2 / 100 != 2)
                    {
                        UE_LOG(LogTemp, Error, TEXT("[VoiceToken] createRoomToken failed. ok=%d code=%d body=%s"),
                            bOk2 ? 1 : 0,
                            Code2,
                            *RespStr2
                        );
                        WeakThis->Client_ReceiveVoiceJoinToken(InRoomId, TEXT(""));
                        return;
                    }

                    FString ClientBaseUrl;
                    FString ParticipantToken;

                    // 응답 형태는 버전에 따라 다를 수 있어서 여러 케이스 흡수
                    TSharedPtr<FJsonObject> Json;
                    const TSharedRef<TJsonReader<>> Reader2 = TJsonReaderFactory<>::Create(RespStr2);
                    if (FJsonSerializer::Deserialize(Reader2, Json) && Json.IsValid())
                    {
                        // (A) 최상위 필드 케이스: clientBaseUrl / participantToken (주로 소문자)
                        Json->TryGetStringField(TEXT("clientBaseUrl"), ClientBaseUrl);
                        Json->TryGetStringField(TEXT("participantToken"), ParticipantToken);

                        // (B) 혹시 대문자 케이스도 같이
                        if (ClientBaseUrl.IsEmpty()) Json->TryGetStringField(TEXT("ClientBaseUrl"), ClientBaseUrl);
                        if (ParticipantToken.IsEmpty()) Json->TryGetStringField(TEXT("ParticipantToken"), ParticipantToken);

                        // (C) participants 배열 케이스
                        if (ParticipantToken.IsEmpty() && Json->HasTypedField<EJson::Array>(TEXT("participants")))
                        {
                            const TArray<TSharedPtr<FJsonValue>> Arr = Json->GetArrayField(TEXT("participants"));
                            for (const TSharedPtr<FJsonValue>& V : Arr)
                            {
                                const TSharedPtr<FJsonObject> O = V.IsValid() ? V->AsObject() : nullptr;
                                if (!O.IsValid()) continue;

                                FString ThisPuid;
                                O->TryGetStringField(TEXT("puid"), ThisPuid);

                                if (ThisPuid == Puid)
                                {
                                    O->TryGetStringField(TEXT("token"), ParticipantToken);
                                    if (ParticipantToken.IsEmpty()) O->TryGetStringField(TEXT("participantToken"), ParticipantToken);
                                    if (ParticipantToken.IsEmpty()) O->TryGetStringField(TEXT("ParticipantToken"), ParticipantToken);
                                    break;
                                }
                            }
                        }

                        // (D) ClientBaseUrl도 participants나 다른 키에 들어있는 변종 대비
                        if (ClientBaseUrl.IsEmpty())
                        {
                            Json->TryGetStringField(TEXT("clientBaseURL"), ClientBaseUrl);
                            if (ClientBaseUrl.IsEmpty()) Json->TryGetStringField(TEXT("ClientBaseURL"), ClientBaseUrl);
                        }
                    }

                    if (ClientBaseUrl.IsEmpty() || ParticipantToken.IsEmpty())
                    {
                        UE_LOG(LogTemp, Error, TEXT("[VoiceToken] Response missing clientBaseUrl/participantToken: %s"),
                            *RespStr2);
                        WeakThis->Client_ReceiveVoiceJoinToken(InRoomId, TEXT(""));
                        return;
                    }

                    //  (서버) 여기서 클라로 내려줄 creds 만들기 직전/직후 로그
                    UE_LOG(LogTemp, Warning, TEXT("[DiagServer] SendCreds TargetPUID=%s Room=%s CredsLen=%d BaseUrl=%s TokenLen=%d"),
                        *Puid,
                        *InRoomId,
                        0, // 아직 만들기 전이라 0 (아래에서 실제 길이로 다시 찍음)
                        *ClientBaseUrl,
                        ParticipantToken.Len()
                    );

                    const FString CredentialsJson = FString::Printf(
                        TEXT("{\"clientBaseUrl\":\"%s\",\"participantToken\":\"%s\"}"),
                        *ClientBaseUrl, *ParticipantToken
                    );

                    UE_LOG(LogTemp, Warning, TEXT("[DiagServer] SendCreds TargetPUID=%s Room=%s CredsLen=%d BaseUrl=%s TokenLen=%d"),
                        *Puid,
                        *InRoomId,
                        CredentialsJson.Len(),
                        *ClientBaseUrl,
                        ParticipantToken.Len()
                    );

                    UE_LOG(LogTemp, Warning, TEXT("[VoiceToken] Creds len=%d head=%s"),
                        CredentialsJson.Len(),
                        *CredentialsJson.Left(200)
                    );

                    UE_LOG(LogTemp, Warning, TEXT("[LobbyPC] Server providing voice creds Room=%s (len=%d)"),
                        *InRoomId, CredentialsJson.Len());

                    WeakThis->Client_ReceiveVoiceJoinToken(InRoomId, CredentialsJson);
                });

            VoiceReq->ProcessRequest();
        });

    TokenReq->ProcessRequest();

}

void ALobbyPlayerController::Client_ReceiveVoiceJoinToken_Implementation(const FString& InRoomId, const FString& InToken)
{
    if (!IsLocalController()) return;

    if (UGameInstance* GI = GetGameInstance())
    {
        if (UVoiceLobbySubsystem* VoiceSS = GI->GetSubsystem<UVoiceLobbySubsystem>())
        {
            const bool bOk = VoiceSS->JoinVoiceRoom(InRoomId, InToken);
            UE_LOG(LogTemp, Warning, TEXT("[LobbyPC] JoinVoiceRoom(%s) -> %d"), *InRoomId, bOk ? 1 : 0);
            bVoiceInitDone = bOk;
        }
    }
}

void ALobbyPlayerController::ServerToggleReady_Implementation()
{
    if (ALobbyPlayerState* PS = GetPlayerState<ALobbyPlayerState>())
    {
        PS->SetReady(!PS->bReady);
        PS->ForceNetUpdate();
    }

    if (ALobbyGameModeBase* GM = GetWorld()->GetAuthGameMode<ALobbyGameModeBase>())
    {
        GM->TryStartGame(); //  Ready 바뀔 때마다 체크
    }
}