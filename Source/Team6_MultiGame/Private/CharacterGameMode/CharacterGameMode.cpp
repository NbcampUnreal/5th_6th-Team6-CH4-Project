// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterGameMode/CharacterGameMode.h"

#include "Character/Controller/MainPlayerController.h"
#include "Character/Squirrel.h"
#include "EngineUtils.h"             // TActorIterator

#include "CharacterGameMode/CharacterGameState.h"


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

    // �ٶ��� ã�� (ĳ��)
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

    // �ٶ��� ������ ī�޶� �����ͼ� ����
    if (!TargetSquirrel)
    {
        UE_LOG(LogTemp, Error, TEXT("GameMode: TargetSquirrel not found"));
        return;
    }

    // ���� �Ҵ�
    if (PlayerIndex == 0)
    {
        PC->SetRole(EPlayerRole::Camera);
      

    }
    else
    {
        PC->SetRole(EPlayerRole::Move);
    }

    


    /* =========================
     * PlayerController�� ����
     * ========================= */
    PC->SetTargetSquirrel(TargetSquirrel);

    PlayerIndex++;
}

/////////////////////////////////////////////////   ����   /////////////////////////////////////////////////
void ACharacterGameMode::ClearGame()
{
    bool bClear = true;

    if (ACharacterGameState* GS = GetGameState<ACharacterGameState>())
    {
        GS->GmaeClear = true;  //Ŭ���� ��
    }
    GameOver(bClear);
}

void ACharacterGameMode::EndGame()
{
    bool bClear = false;

    if (ACharacterGameState* GS = GetGameState<ACharacterGameState>())
    {
        GS->GmaeClear = false;  //Ŭ���� ����
    }
    GameOver(bClear);
}

void ACharacterGameMode::GameOver(bool bClear)
{
    for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
    {
        if (AMainPlayerController* PC = Cast<AMainPlayerController>(It->Get()))
        {
            PC->Client_ShowResult(true, bClear);
        }
    }
}
/////////////////////////////////////////////////   ����   /////////////////////////////////////////////////




