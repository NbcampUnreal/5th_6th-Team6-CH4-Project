

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "LoginSubsystem.generated.h"


UCLASS()
class TEAM6_MULTIGAME_API ULoginSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;


	UFUNCTION(BlueprintCallable, Category = "EOS")
	void LoginEOS_AccountPortal();
	// DevAuthTool로 로그인
	UFUNCTION(BlueprintCallable, Category = "EOS")
	void LoginEOS_DevAuth(const FString& DevAuthId = TEXT("TestUser"));

	UFUNCTION(BlueprintCallable, Category = "EOS")
	bool IsLoggedIn() const { return bLoggedIn; }

private:
	bool bLoggedIn = false;

	FDelegateHandle OnLoginCompleteHandle;

	void OnLoginComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& UserId, const FString& Error);
	IOnlineIdentityPtr GetIdentityInterface() const;

private:
	FString CachedAuthToken;
	FString CachedEasId;
	FString CachedPuid;

public:
	const FString& GetAuthToken() const { return CachedAuthToken; }
	const FString& GetPuid() const { return CachedPuid; }
	const FString& GetEasId() const { return CachedEasId; }
};
