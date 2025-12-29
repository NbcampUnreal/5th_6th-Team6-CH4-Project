

#include "Server/LoginSubsystem.h"

#include "OnlineSubsystem.h"
#include "Interfaces/OnlineIdentityInterface.h"

IOnlineIdentityPtr ULoginSubsystem::GetIdentityInterface() const
{
	IOnlineSubsystem* OSS = IOnlineSubsystem::Get(TEXT("EOS"));
	if (!OSS) return nullptr;
	return OSS->GetIdentityInterface();
}

void ULoginSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogTemp, Warning, TEXT("[LoginSubsystem] Initialize"));
}

void ULoginSubsystem::Deinitialize()
{
	if (IOnlineIdentityPtr Identity = GetIdentityInterface())
	{
		if (OnLoginCompleteHandle.IsValid())
		{
			Identity->ClearOnLoginCompleteDelegate_Handle(0, OnLoginCompleteHandle);
		}
	}

	Super::Deinitialize();
}

void ULoginSubsystem::LoginEOS_AccountPortal()
{
	IOnlineIdentityPtr Identity = GetIdentityInterface();
	if (!Identity.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("[LoginSubsystem] IdentityInterface invalid (EOS subsystem?)"));
		return;
	}

	const int32 LocalUserNum = 0;

	if (Identity->GetLoginStatus(LocalUserNum) == ELoginStatus::LoggedIn)
	{
		bLoggedIn = true;
		UE_LOG(LogTemp, Warning, TEXT("[LoginSubsystem] Already logged in"));
		return;
	}

	FOnlineAccountCredentials Creds;
	Creds.Type = TEXT("accountportal"); //  일반 Epic 로그인(브라우저/오버레이)
	Creds.Id = TEXT("");              // 보통 비움
	Creds.Token = TEXT("");              // 보통 비움

	OnLoginCompleteHandle =
		Identity->AddOnLoginCompleteDelegate_Handle(
			LocalUserNum,
			FOnLoginCompleteDelegate::CreateUObject(this, &ThisClass::OnLoginComplete));

	UE_LOG(LogTemp, Warning, TEXT("[LoginSubsystem] LoginEOS AccountPortal"));
	Identity->Login(LocalUserNum, Creds);
}

void ULoginSubsystem::LoginEOS_DevAuth(const FString& DevAuthId)
{
	IOnlineIdentityPtr Identity = GetIdentityInterface();
	if (!Identity.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("[LoginSubsystem] IdentityInterface invalid (EOS subsystem?)"));
		return;
	}

	const int32 LocalUserNum = 0;

	// 이미 로그인 상태면 스킵
	if (Identity->GetLoginStatus(LocalUserNum) == ELoginStatus::LoggedIn)
	{
		bLoggedIn = true;
		UE_LOG(LogTemp, Warning, TEXT("[LoginSubsystem] Already logged in"));
		return;
	}

	FOnlineAccountCredentials Creds;
	Creds.Type = TEXT("developer");   // DevAuthTool
	Creds.Id = TEXT("127.0.0.1:6547");        // DevAuthTool credential name
	Creds.Token = TEXT("TestUser");           // 보통 비움

	OnLoginCompleteHandle =
		Identity->AddOnLoginCompleteDelegate_Handle(
			LocalUserNum,
			FOnLoginCompleteDelegate::CreateUObject(this, &ThisClass::OnLoginComplete));

	UE_LOG(LogTemp, Warning, TEXT("[LoginSubsystem] LoginEOS DevAuthId=%s"), *DevAuthId);
	Identity->Login(LocalUserNum, Creds);
}

void ULoginSubsystem::OnLoginComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& UserId, const FString& Error)
{
	IOnlineIdentityPtr Identity = GetIdentityInterface();
	if (Identity.IsValid() && OnLoginCompleteHandle.IsValid())
	{
		Identity->ClearOnLoginCompleteDelegate_Handle(LocalUserNum, OnLoginCompleteHandle);
	}

	bLoggedIn = bWasSuccessful;

	if (bWasSuccessful)
	{
		UE_LOG(LogTemp, Warning, TEXT("[LoginSubsystem] Login SUCCESS. UserId=%s"), *UserId.ToString());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[LoginSubsystem] Login FAILED: %s"), *Error);
	}
}