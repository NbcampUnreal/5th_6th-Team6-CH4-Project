// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterGameMode/CharacterGameMode.h"

#include "Character/MoveManager.h"
#include "Character/Squirrel.h"

#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"

ACharacterGameMode::ACharacterGameMode()
{
    DefaultPawnClass = nullptr;
    
}

void ACharacterGameMode::BeginPlay()
{
	Super::BeginPlay();

    if (GetWorld())
    {
        MoveManager = GetWorld()->SpawnActor<AMoveManager>();
    }
}

APlayerController* ACharacterGameMode::SpawnPlayerController(
    ENetRole InRemoteRole,
    const FString& Options
)
{
    int32 PlayerIndex = GetNumPlayers();

    if (PlayerIndex == 0 && MouseControllerClass)
    {
        PlayerControllerClass = MouseControllerClass;
    }
    else if (WASDControllerClass)
    {
        PlayerControllerClass = WASDControllerClass;
    }

    UE_LOG(LogTemp, Warning,
        TEXT("Spawning PlayerController for Player %d using %s"),
        PlayerIndex,
        *PlayerControllerClass->GetName()
    );

    return Super::SpawnPlayerController(InRemoteRole, Options);
}


// ★ 2. 접속 완료 후 (컨트롤러 생성 완료됨) -> 다람쥐 빙의 시키기
void ACharacterGameMode::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);

    if (!HasAuthority() || !NewPlayer)
    {
        return;
    }

    // 다람쥐 빙의
    AssignSquirrelToController(NewPlayer);
}




void ACharacterGameMode::AssignSquirrelToController(APlayerController* PC)
{
    if (!PC) return;

    if (APawn* CurrentPawn = PC->GetPawn())
    {
        PC->UnPossess();
    }

    for (TActorIterator<ASquirrel> It(GetWorld()); It; ++It)
    {
        ASquirrel* Squirrel = *It;

        // 주인이 없는 다람쥐 발견
        if (Squirrel && Squirrel->GetController() == nullptr)
        {
            PC->Possess(Squirrel);
            UE_LOG(LogTemp, Warning, TEXT("Success: Squirrel possessed by %s"), *PC->GetName());

            // [중요] 빙의 후 클라이언트에게 카메라 업데이트 등을 알리기 위해 필요시 RPC 호출 가능
            // 하지만 Possess가 되면 자동으로 ViewTarget이 잡히므로 보통은 바로 됩니다.
            return;
        }
    }

    UE_LOG(LogTemp, Error, TEXT("Failed: No free Squirrel found in level!"));
}