

#include "Server/LobbyPlayerState.h"

#include "Net/UnrealNetwork.h"

ALobbyPlayerState::ALobbyPlayerState()
{
	// PlayerState는 보통 기본적으로 복제되지만 명시해도 OK
	bReplicates = true;
}

void ALobbyPlayerState::SetReady(bool bNewReady)
{
	if (bReady == bNewReady)
	{
		return;
	}

	bReady = bNewReady;

	// 서버에서도 즉시 UI 갱신 이벤트가 필요하면 브로드캐스트
	// (클라는 OnRep_Ready에서 브로드캐스트됨)
	OnReadyChanged.Broadcast(this, bReady);
}

void ALobbyPlayerState::OnRep_Ready()
{
	// 클라이언트에서 bReady 복제 반영 시점
	OnReadyChanged.Broadcast(this, bReady);
}

void ALobbyPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ALobbyPlayerState, bReady);
}