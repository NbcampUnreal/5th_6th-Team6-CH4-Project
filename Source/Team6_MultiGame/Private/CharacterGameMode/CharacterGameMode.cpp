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
        TEXT("[GM] PostLogin PC=%s Pawn=%s PlayerIndex=%d"),
        *PC->GetName(),
        PC->GetPawn() ? TEXT("HasPawn") : TEXT("NoPawn"),
        PlayerIndex
    );

    // ===== TargetSquirrel Ä³½Ì(Ã³À½ 1È¸) =====
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

    // ï¿½Ù¶ï¿½ï¿½ï¿½ ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ Ä«ï¿½Þ¶ï¿½ ï¿½ï¿½ï¿½ï¿½ï¿½Í¼ï¿½ ï¿½ï¿½ï¿½ï¿½
    if (!TargetSquirrel)
    {
        UE_LOG(LogTemp, Error, TEXT("GameMode: TargetSquirrel not found"));
        return;
    }

    // ===== ¿ªÇÒ ¹èÁ¤: Á¤È®È÷ 3¸í °íÁ¤ =====
    if (PlayerIndex == 0)
    {
        PC->SetRole(EPlayerRole::Camera);
    }
    else if (PlayerIndex == 1)
    {
        PC->SetRole(EPlayerRole::Move1);
    }
    else // PlayerIndex == 2
    {
        PC->SetRole(EPlayerRole::Move2);
    }

    


    /* =========================
     * PlayerControllerï¿½ï¿½ ï¿½ï¿½ï¿½ï¿½
     * ========================= */
    PC->SetTargetSquirrel(TargetSquirrel);

    UE_LOG(LogTemp, Warning,
        TEXT("[GM] Assigned PC=%s Role=%s"),
        *GetNameSafe(PC),
        (PC->PlayerRole == EPlayerRole::Camera) ? TEXT("Camera")
        : (PC->PlayerRole == EPlayerRole::Move1) ? TEXT("Move1")
        : (PC->PlayerRole == EPlayerRole::Move2) ? TEXT("Move2")
        : TEXT("Unknown")
    );

    PlayerIndex++;
}

/////////////////////////////////////////////////   ï¿½ï¿½ï¿½ï¿½   /////////////////////////////////////////////////
void ACharacterGameMode::ClearGame()
{
    bool bClear = true;

    if (ACharacterGameState* GS = GetGameState<ACharacterGameState>())
    {
        GS->GmaeClear = true;  //Å¬ï¿½ï¿½ï¿½ï¿½ ï¿½ï¿½
    }
    GameOver(bClear);
}

void ACharacterGameMode::EndGame()
{
    bool bClear = false;

    if (ACharacterGameState* GS = GetGameState<ACharacterGameState>())
    {
        GS->GmaeClear = false;  //Å¬ï¿½ï¿½ï¿½ï¿½ ï¿½ï¿½ï¿½ï¿½
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
/////////////////////////////////////////////////   ï¿½ï¿½ï¿½ï¿½   /////////////////////////////////////////////////

void ACharacterGameMode::Logout(AController* Exiting)
{
    AMainPlayerController* PC = Cast<AMainPlayerController>(Exiting);

    const TCHAR* RoleText = TEXT("Unknown");
    if (PC)
    {
        // ÇöÀç °ª¸¸ ÀÐ±â(SetRole È£Ãâ ±ÝÁö)
        RoleText =
            (PC->PlayerRole == EPlayerRole::Camera) ? TEXT("Camera")
            : (PC->PlayerRole == EPlayerRole::Move1) ? TEXT("Move1")
            : (PC->PlayerRole == EPlayerRole::Move2) ? TEXT("Move2")
            : TEXT("Unknown");
    }

    UE_LOG(LogTemp, Warning, TEXT("[GM] Logout Role=%s"), RoleText);
    EndGame();
    Super::Logout(Exiting);
}


