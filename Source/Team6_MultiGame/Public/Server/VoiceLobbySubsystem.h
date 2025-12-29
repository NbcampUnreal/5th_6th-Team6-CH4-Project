// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"
#include "VoiceLobbySubsystem.generated.h"


UCLASS()
class TEAM6_MULTIGAME_API UVoiceLobbySubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	// 보이스 로비 생성 or 참가
	void CreateOrJoinVoiceLobby(int32 MaxPlayers = 4);

	//  리더만 호출: 바로 Create만 수행
	void CreateVoiceLobby(int32 MaxPlayers = 4);

	//  리더 아닌 사람만 호출: Find → 있으면 Join / 없으면 그냥 리턴(절대 Create 안 함)
	void FindAndJoinVoiceLobby();

	//  LobbyPC에서 체크용
	bool HasVoiceLobbySession() const;

	// 종료 시 정리(선택)
	void LeaveVoiceLobby();

private:
	static const FName VOICE_SESSION_NAME;

	TSharedPtr<FOnlineSessionSearch> SessionSearch;

	// delegates
	FDelegateHandle OnCreateSessionCompleteHandle;
	FDelegateHandle OnFindSessionsCompleteHandle;
	FDelegateHandle OnJoinSessionCompleteHandle;
	FDelegateHandle OnDestroySessionCompleteHandle;

	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	void OnFindSessionsComplete(bool bWasSuccessful);
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	void OnDestroySessionComplete(FName SessionName, bool bWasSuccessful);

	IOnlineSessionPtr GetSessionInterface() const;

private:
	bool bFindInProgress = false;

	int32 PendingMaxPlayers = 4;

	// Find 결과가 없을 때 Create할지 여부 (CreateOrJoin만 true)
	bool bCreateIfNotFound = false;

private:
	void StartFind(bool bInCreateIfNotFound);
	void StartCreate();
};
