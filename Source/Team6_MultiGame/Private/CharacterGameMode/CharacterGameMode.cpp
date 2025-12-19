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

//void ACharacterGameMode::PostLogin(APlayerController* NewPlayer)
//{
//    Super::PostLogin(NewPlayer);
//
//    AMainPlayerController* PC = Cast<AMainPlayerController>(NewPlayer);
//    if (!PC) return;
//
//
//    if (PlayerIndex == 0)
//    {
//        PC->SetRole(EPlayerRole::Camera);
//    }
//    else
//    {
//        PC->SetRole(EPlayerRole::Move);
//    }
//
//    // ?? 항상 호출 (중복 안전)
//    PC->SetSharedCamera(SharedCamera);
//
//    // 레벨에 배치된 Squirrel 가져오기
//    if (!TargetSquirrel)
//    {
//        for (TActorIterator<ASquirrel> It(GetWorld()); It; ++It)
//        {
//            TargetSquirrel = *It;
//            break; // 첫 번째 다람쥐만 사용
//        }
//    }
//
//    // PlayerController에 TargetSquirrel 주입
//    PC->SetTargetSquirrel(TargetSquirrel);
//    PlayerIndex++;
//}

void ACharacterGameMode::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);
    AMainPlayerController* PC = Cast<AMainPlayerController>(NewPlayer);
    if (!PC) return;

    // 역할 할당
    if (PlayerIndex == 0)
    {
        PC->SetRole(EPlayerRole::Camera);
    }
    else
    {
        PC->SetRole(EPlayerRole::Move);
    }

    // 다람쥐 찾기 (캐싱)
    if (!TargetSquirrel)
    {
        for (TActorIterator<ASquirrel> It(GetWorld()); It; ++It)
        {
            TargetSquirrel = *It;
            break;
        }
    }

    // 다람쥐 있으면 카메라 가져와서 설정
    if (!TargetSquirrel)
    {
        UE_LOG(LogTemp, Error, TEXT("GameMode: TargetSquirrel not found"));
        return;
    }

    /* =========================
     * PlayerController에 전달
     * ========================= */
    PC->SetTargetSquirrel(TargetSquirrel);

    PlayerIndex++;
}






