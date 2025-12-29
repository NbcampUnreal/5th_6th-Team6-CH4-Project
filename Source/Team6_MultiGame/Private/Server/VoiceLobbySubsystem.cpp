

#include "Server/VoiceLobbySubsystem.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystemUtils.h"


const FName UVoiceLobbySubsystem::VOICE_SESSION_NAME(TEXT("VOICE_LOBBY"));

IOnlineSessionPtr UVoiceLobbySubsystem::GetSessionInterface() const
{
	IOnlineSubsystem* OSS = IOnlineSubsystem::Get(TEXT("EOS")); // EOS를 명시(권장)
	if (!OSS) return nullptr;
	return OSS->GetSessionInterface();
}

bool UVoiceLobbySubsystem::HasVoiceLobbySession() const
{
	IOnlineSessionPtr Session = GetSessionInterface();
	if (!Session.IsValid()) return false;
	return (Session->GetNamedSession(VOICE_SESSION_NAME) != nullptr);
}


void UVoiceLobbySubsystem::CreateOrJoinVoiceLobby(int32 MaxPlayers)
{
	PendingMaxPlayers = MaxPlayers;

	IOnlineSessionPtr Session = GetSessionInterface();
	if (!Session.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("[VoiceLobby] SessionInterface invalid (EOS not ready?)"));
		return;
	}

	if (Session->GetNamedSession(VOICE_SESSION_NAME))
	{
		UE_LOG(LogTemp, Warning, TEXT("[VoiceLobby] Already have VOICE session"));
		return;
	}

	StartFind(/*bInCreateIfNotFound=*/true);
}

void UVoiceLobbySubsystem::CreateVoiceLobby(int32 MaxPlayers)
{
	PendingMaxPlayers = MaxPlayers;

	IOnlineSessionPtr Session = GetSessionInterface();
	if (!Session.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("[VoiceLobby] SessionInterface invalid (EOS not ready?)"));
		return;
	}

	if (Session->GetNamedSession(VOICE_SESSION_NAME))
	{
		UE_LOG(LogTemp, Warning, TEXT("[VoiceLobby] Already have VOICE session"));
		return;
	}

	bCreateIfNotFound = false;
	StartCreate();
}

void UVoiceLobbySubsystem::FindAndJoinVoiceLobby()
{
	IOnlineSessionPtr Session = GetSessionInterface();
	if (!Session.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("[VoiceLobby] SessionInterface invalid (EOS not ready?)"));
		return;
	}

	if (Session->GetNamedSession(VOICE_SESSION_NAME))
	{
		UE_LOG(LogTemp, Warning, TEXT("[VoiceLobby] Already have VOICE session"));
		return;
	}

	StartFind(/*bInCreateIfNotFound=*/false);
}

void UVoiceLobbySubsystem::StartFind(bool bInCreateIfNotFound)
{
	if (bFindInProgress)
	{
		UE_LOG(LogTemp, VeryVerbose, TEXT("[VoiceLobby] StartFind ignored: already in progress"));
		return;
	}
	bFindInProgress = true;;

	bCreateIfNotFound = bInCreateIfNotFound;

	IOnlineSessionPtr Session = GetSessionInterface();
	if (!Session.IsValid())
	{
		bFindInProgress = false; //  실패하면 반드시 풀어주기
		UE_LOG(LogTemp, Warning, TEXT("[VoiceLobby] StartFind failed: Session invalid"));
		return;
	}

	SessionSearch = MakeShared<FOnlineSessionSearch>();
	SessionSearch->bIsLanQuery = false;
	SessionSearch->MaxSearchResults = 50;

	//  VOICE_ONLY 세션만 찾도록 필터(중요)
	SessionSearch->QuerySettings.Set(FName("VOICE_ONLY"), true, EOnlineComparisonOp::Equals);

	//  PRESENCE 기반은 혼선(EOS_InvalidUser) 유발할 때가 많아서 제거 권장
	// SessionSearch->QuerySettings.Set(FName(TEXT("PRESENCE")), true, EOnlineComparisonOp::Equals);

	OnFindSessionsCompleteHandle =
		Session->AddOnFindSessionsCompleteDelegate_Handle(
			FOnFindSessionsCompleteDelegate::CreateUObject(this, &ThisClass::OnFindSessionsComplete));

	UE_LOG(LogTemp, Warning, TEXT("[VoiceLobby] FindSessions... (CreateIfNotFound=%d)"), bCreateIfNotFound ? 1 : 0);
	Session->FindSessions(/*LocalUserNum=*/0, SessionSearch.ToSharedRef());
}

void UVoiceLobbySubsystem::OnFindSessionsComplete(bool bWasSuccessful)
{
	bFindInProgress = false;

	IOnlineSessionPtr Session = GetSessionInterface();
	if (!Session.IsValid()) return;

	UE_LOG(LogTemp, Warning, TEXT("[VoiceLobby] FindDone: Success=%d Results=%d"),
		bWasSuccessful ? 1 : 0,
		SessionSearch.IsValid() ? SessionSearch->SearchResults.Num() : -1);

	Session->ClearOnFindSessionsCompleteDelegate_Handle(OnFindSessionsCompleteHandle);

	if (bWasSuccessful && SessionSearch.IsValid())
	{
		for (const FOnlineSessionSearchResult& R : SessionSearch->SearchResults)
		{
			bool bIsVoiceLobby = false;
			R.Session.SessionSettings.Get(FName("VOICE_ONLY"), bIsVoiceLobby);

			if (bIsVoiceLobby)
			{
				UE_LOG(LogTemp, Warning, TEXT("[VoiceLobby] Found voice lobby -> Join"));

				OnJoinSessionCompleteHandle =
					Session->AddOnJoinSessionCompleteDelegate_Handle(
						FOnJoinSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnJoinSessionComplete));

				Session->JoinSession(0, VOICE_SESSION_NAME, R);
				return;
			}
		}
	}

	//  여기서 갈림: CreateOrJoin만 Create, FindAndJoin은 그냥 리턴
	if (!bCreateIfNotFound)
	{
		UE_LOG(LogTemp, Warning, TEXT("[VoiceLobby] No voice lobby found (join-only mode). Will retry later."));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("[VoiceLobby] No voice lobby found -> Create (leader mode)"));
	StartCreate();
}

void UVoiceLobbySubsystem::StartCreate()
{
	IOnlineSessionPtr Session = GetSessionInterface();
	if (!Session.IsValid()) return;

	FOnlineSessionSettings Settings;
	Settings.bIsLANMatch = false;
	Settings.bShouldAdvertise = true;
	Settings.bAllowJoinInProgress = true;
	Settings.NumPublicConnections = PendingMaxPlayers;

	Settings.bUseLobbiesIfAvailable = true;
	Settings.bUseLobbiesVoiceChatIfAvailable = true;

	//  필요 없으면 끄는 게 안전 (원하면 true로 되돌려도 됨)
	Settings.bUsesPresence = false;

	Settings.Set(FName("VOICE_ONLY"), true, EOnlineDataAdvertisementType::ViaOnlineService);

	OnCreateSessionCompleteHandle =
		Session->AddOnCreateSessionCompleteDelegate_Handle(
			FOnCreateSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnCreateSessionComplete));

	Session->CreateSession(0, VOICE_SESSION_NAME, Settings);

	if (IOnlineSubsystem* OSS = IOnlineSubsystem::Get(TEXT("EOS")))
	{
		if (IOnlineIdentityPtr Identity = OSS->GetIdentityInterface(); Identity.IsValid())
		{
			if (TSharedPtr<const FUniqueNetId> MyId = Identity->GetUniquePlayerId(0); MyId.IsValid())
			{
				Settings.MemberSettings.FindOrAdd(MyId.ToSharedRef());
			}
		}
	}
}

void UVoiceLobbySubsystem::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	IOnlineSessionPtr Session = GetSessionInterface();
	if (!Session.IsValid()) return;

	Session->ClearOnCreateSessionCompleteDelegate_Handle(OnCreateSessionCompleteHandle);

	UE_LOG(LogTemp, Warning, TEXT("[VoiceLobby] CreateSession(%s) = %d"), *SessionName.ToString(), bWasSuccessful);
}

void UVoiceLobbySubsystem::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	IOnlineSessionPtr Session = GetSessionInterface();
	if (!Session.IsValid()) return;

	Session->ClearOnJoinSessionCompleteDelegate_Handle(OnJoinSessionCompleteHandle);

	UE_LOG(LogTemp, Warning, TEXT("[VoiceLobby] JoinSession(%s) Result=%d"), *SessionName.ToString(), (int32)Result);
}

void UVoiceLobbySubsystem::LeaveVoiceLobby()
{
	IOnlineSessionPtr Session = GetSessionInterface();
	if (!Session.IsValid()) return;

	if (!Session->GetNamedSession(VOICE_SESSION_NAME))
		return;

	OnDestroySessionCompleteHandle =
		Session->AddOnDestroySessionCompleteDelegate_Handle(
			FOnDestroySessionCompleteDelegate::CreateUObject(this, &ThisClass::OnDestroySessionComplete));

	Session->DestroySession(VOICE_SESSION_NAME);
}

void UVoiceLobbySubsystem::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
	IOnlineSessionPtr Session = GetSessionInterface();
	if (!Session.IsValid()) return;

	Session->ClearOnDestroySessionCompleteDelegate_Handle(OnDestroySessionCompleteHandle);
	UE_LOG(LogTemp, Warning, TEXT("[VoiceLobby] DestroySession(%s) = %d"), *SessionName.ToString(), bWasSuccessful);
}