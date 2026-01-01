// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterGameMode/CharacterGameMode.h"

#include "Character/Controller/MainPlayerController.h"
#include "Character/Squirrel.h"
#include "EngineUtils.h"             // TActorIterator

#include "CharacterGameMode/CharacterGameState.h"

namespace
{
    static const TCHAR* RoleToText(EPlayerRole Role)
    {
        switch (Role)
        {
        case EPlayerRole::Camera: return TEXT("Camera");
        case EPlayerRole::Fire:   return TEXT("Fire");
        case EPlayerRole::Move1:  return TEXT("Move1");
        case EPlayerRole::Move2:  return TEXT("Move2");
        case EPlayerRole::None:   return TEXT("None");
        default:                  return TEXT("Unknown");
        }
    }
}

ACharacterGameMode::ACharacterGameMode()
{
    DefaultPawnClass = nullptr;
    TargetSquirrel = nullptr;
}

EPlayerRole ACharacterGameMode::FindFreeRole() const
{
    // °íÁ¤ 4°³ ¿ªÇÒ Ç®(¿øÇÏ´Â ¿ì¼±¼ø¼­·Î Á¤·Ä °¡´É)
    static const EPlayerRole RoleOrder[] =
    {
        EPlayerRole::Camera,
        EPlayerRole::Fire,
        EPlayerRole::Move1,
        EPlayerRole::Move2
    };

    TSet<EPlayerRole> Taken;

    // ÇöÀç ¿ùµåÀÇ ¸ðµç PlayerController¸¦ µ¹¸é¼­ ÀÌ¹Ì »ç¿ë ÁßÀÎ ¿ªÇÒ ¼öÁý
    for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
    {
        const AMainPlayerController* Other = Cast<AMainPlayerController>(It->Get());
        if (!Other) continue;

        switch (Other->PlayerRole)
        {
        case EPlayerRole::Camera:
        case EPlayerRole::Fire:
        case EPlayerRole::Move1:
        case EPlayerRole::Move2:
            Taken.Add(Other->PlayerRole);
            break;
        default:
            // None/Unknown µîÀº Taken¿¡ ³ÖÁö ¾ÊÀ½
            break;
        }
    }

    // ºó ¿ªÇÒ ÇÏ³ª ¹ÝÈ¯
    for (EPlayerRole R : RoleOrder)
    {
        if (!Taken.Contains(R))
            return R;
    }

    // 4°³°¡ ¸ðµÎ Â÷ ÀÖÀ¸¸é None
    return EPlayerRole::None;
}


void ACharacterGameMode::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);

    AMainPlayerController* PC = Cast<AMainPlayerController>(NewPlayer);
    if (!PC) return;

    UE_LOG(LogTemp, Warning,
        TEXT("[GM] PostLogin PC=%s Pawn=%s NumPlayers=%d"),
        *PC->GetName(),
        PC->GetPawn() ? TEXT("HasPawn") : TEXT("NoPawn"),
        NumPlayers
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

    // ===== ºñ¾îÀÖ´Â ¿ªÇÒ ¹èÁ¤(4°³ ´Ù Â÷¸é None) =====
    const EPlayerRole AssignedRole = FindFreeRole();
    PC->SetRole(AssignedRole);
    


    /* =========================
     * PlayerControllerï¿½ï¿½ ï¿½ï¿½ï¿½ï¿½
     * ========================= */
    PC->SetTargetSquirrel(TargetSquirrel);

    UE_LOG(LogTemp, Warning,
        TEXT("[GM] Assigned PC=%s Role=%s"),
        *GetNameSafe(PC),
        RoleToText(AssignedRole)
    );
}

void ACharacterGameMode::ClearGame()
{
    bool bClear = true;

    if (ACharacterGameState* GS = GetGameState<ACharacterGameState>())
    {
        GS->GmaeClear = true;
    }
    GameOver(bClear);
}

void ACharacterGameMode::EndGame()
{
    bool bClear = false;

    if (ACharacterGameState* GS = GetGameState<ACharacterGameState>())
    {
        GS->GmaeClear = false; 
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

void ACharacterGameMode::ReturnToLobby()
{
    if (!HasAuthority()) return;

    const FString LobbyURL = TEXT("/Game/Server/Maps/LobbyMap");
    GetWorld()->ServerTravel(LobbyURL);
}

void ACharacterGameMode::Logout(AController* Exiting)
{
    AMainPlayerController* PC = Cast<AMainPlayerController>(Exiting);

    const TCHAR* RoleText = TEXT("Unknown");
    if (PC)
    {
        RoleText = RoleToText(PC->PlayerRole);
    }

    UE_LOG(LogTemp, Warning, TEXT("[GM] Logout Role=%s"), RoleText);

    EndGame();
    Super::Logout(Exiting);
}


