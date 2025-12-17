// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/MoveManager.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "Engine/Engine.h"
#include "Character/Squirrel.h"
#include "EngineUtils.h"

AMoveManager::AMoveManager()
{

    bReplicates = true;

    Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    RootComponent = Root;
}

void AMoveManager::OnClientWInput(APlayerController* PC, const FVector2D& MoveInput)
{
    // 클라이언트 정보

    int32 PlayerId = -1;

    if (PC && PC->PlayerState)
    {
        PlayerId = PC->PlayerState->GetPlayerId(); // 클라이언트 번호
    }

    // MoveInput 값 확인
    FString Msg = FString::Printf(
        TEXT("[MoveManager] Player: %d / | X=%.2f, Y=%.2f"),
        PlayerId,        // int32 → %d
        MoveInput.X,
        MoveInput.Y
    );

    // 서버 화면 디버그 출력
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Yellow, Msg);
        UE_LOG(LogTemp, Warning, TEXT("%s"), *Msg);
    }

    // 다람쥐들에게 Move 호출
    if (HasAuthority()) // 서버에서만
    {
        for (TActorIterator<ASquirrel> It(GetWorld()); It; ++It)
        {
            ASquirrel* Squirrel = *It;
            if (Squirrel)
            {
                Squirrel->Move(MoveInput);
            }
        }
    }
    // 클라이언트 화면에도 멀티캐스트
    Multicast_DebugWInput(Msg);
}

void AMoveManager::Multicast_DebugWInput_Implementation(const FString& Msg)
{
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Cyan, FString::Printf(TEXT("[Client] %s"), *Msg));
    }
}

