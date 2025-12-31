// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterGameMode/CharacterGameMode.h"

#include "Character/Controller/MainPlayerController.h"
#include "Character/Squirrel.h"
#include "EngineUtils.h"             // TActorIterator

ACharacterGameMode::ACharacterGameMode()
{
    DefaultPawnClass = nullptr;
    PlayerIndex = 0;
    TargetSquirrel = nullptr;
}


void ACharacterGameMode::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);

    AMainPlayerController* PC = Cast<AMainPlayerController>(NewPlayer);
    if (!PC) return;

    UE_LOG(LogTemp, Warning,
        TEXT("[GM] PostLogin PC=%s RoleBefore=%s"),
        *PC->GetName(),
        PC->GetPawn() ? TEXT("HasPawn") : TEXT("NoPawn")
    );

    // 다람쥐 찾기 (캐싱)
    if (!TargetSquirrel)
    {
        for (TActorIterator<ASquirrel> It(GetWorld()); It; ++It)
        {
            TargetSquirrel = *It;
            break;
        }
        UE_LOG(LogTemp, Warning,
            TEXT("[GM] TargetSquirrel Cached = %s"),
            *GetNameSafe(TargetSquirrel)
        );

    }

    // 다람쥐 있으면 카메라 가져와서 설정
    if (!TargetSquirrel)
    {
        UE_LOG(LogTemp, Error, TEXT("GameMode: TargetSquirrel not found"));
        return;
    }

    // 역할 할당
    if (PlayerIndex == 0)
    {
        PC->SetRole(EPlayerRole::Camera);
      

    }
    else
    {
        PC->SetRole(EPlayerRole::Move);
    }

    


    /* =========================
     * PlayerController에 전달
     * ========================= */
    PC->SetTargetSquirrel(TargetSquirrel);

    PlayerIndex++;
}



void ACharacterGameMode::Logout(AController* Exiting)
{
    AMainPlayerController* PC = Cast<AMainPlayerController>(Exiting);

    const TCHAR* RoleText = TEXT("Unknown");
    if (PC)
    {
        // ★ SetRole 호출 금지! 그냥 현재 값 읽기
        RoleText = (PC->PlayerRole == EPlayerRole::Camera) ? TEXT("Camera") : TEXT("Move");
    }

    UE_LOG(LogTemp, Warning, TEXT("[GM] Logout Role=%s"), RoleText);

    Super::Logout(Exiting);
}


