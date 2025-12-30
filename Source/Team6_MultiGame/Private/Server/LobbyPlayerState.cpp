

#include "Server/LobbyPlayerState.h"

#include "Net/UnrealNetwork.h"

ALobbyPlayerState::ALobbyPlayerState()
{
	// PlayerState는 보통 기본적으로 복제되지만 명시해도 OK
	bReplicates = true;
}

void ALobbyPlayerState::SetReady(bool bNewReady)
{
	if (!HasAuthority())
	{
		return;
	}

	if (bReady == bNewReady)
	{
		return;
	}

	bReady = bNewReady;
	OnReadyChanged.Broadcast(this, bReady);
}

void ALobbyPlayerState::SetIsLeader(bool bNewLeader)
{
	if (!HasAuthority())
	{
		return;
	}

	if (bIsLeader == bNewLeader)
	{
		return;
	}

	bIsLeader = bNewLeader;
	UE_LOG(LogTemp, Warning, TEXT("[LobbyPS] %s IsLeader=%d"), *GetName(), bIsLeader);
}

void ALobbyPlayerState::OnRep_Ready()
{
	UE_LOG(LogTemp, Warning, TEXT("[OnRep_Ready] %s Ready=%d"), *GetName(), bReady);
	// 클라이언트에서 bReady 복제 반영 시점
	OnReadyChanged.Broadcast(this, bReady);
}

void ALobbyPlayerState::OnRep_IsLeader()
{
	UE_LOG(LogTemp, Warning, TEXT("[OnRep_IsLeader] %s IsLeader=%d"), *GetName(), bIsLeader);
}

void ALobbyPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ALobbyPlayerState, bReady);
	DOREPLIFETIME(ALobbyPlayerState, bIsLeader);
	DOREPLIFETIME(ALobbyPlayerState, VoiceRoomId); //  추가
}

void ALobbyPlayerState::SetVoiceRoomId(const FString& InRoomId)
{
	if (!HasAuthority()) return;

	VoiceRoomId = InRoomId;
	ForceNetUpdate();
}

void ALobbyPlayerState::OnRep_VoiceRoomId()
{
	UE_LOG(LogTemp, Warning, TEXT("[OnRep_VoiceRoomId] %s Room=%s"), *GetName(), *VoiceRoomId);
	OnVoiceRoomIdChanged.Broadcast(this, VoiceRoomId);
}