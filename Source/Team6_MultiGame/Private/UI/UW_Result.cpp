// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/UW_Result.h"

#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Character/Controller/MainPlayerController.h"
#include "CharacterGameMode/CharacterGameState.h"
#include "Server/TitlePlayerController.h"
#include "GameFramework/PlayerController.h"



void UUW_Result::NativeConstruct()
{
	Super::NativeConstruct();

	if (RestartButton)
	{
		RestartButton->OnClicked.AddDynamic(this, &ThisClass::OnRestartClicked);
	}

	if (ExitButton)
	{
		ExitButton->OnClicked.AddDynamic(this, &ThisClass::OnExitClicked);
	}
}

/////////////////////////////////////////////////   수정   /////////////////////////////////////////////////
void UUW_Result::OnRestartClicked()
{
	if (AMainPlayerController* PC = GetOwningPlayer<AMainPlayerController>())
	{
		PC->SetPause(false);
		PC->Server_RequestReturnToLobby(); // 서버에게 "로비로 보내줘" 요청
	}
}
/////////////////////////////////////////////////   수정   /////////////////////////////////////////////////

void UUW_Result::OnExitClicked()
{
	UKismetSystemLibrary::QuitGame(this, nullptr, EQuitPreference::Quit, true);
}

/////////////////////////////////////////////////   수정   /////////////////////////////////////////////////
void UUW_Result::SetTitleText(bool bClear)
{
	if (!ResultText) return;

	ResultText->SetText(
		bClear
		? FText::FromString(TEXT("!! Game Clear !!"))
		: FText::FromString(TEXT("Game Over"))
	);
}
/////////////////////////////////////////////////   수정   /////////////////////////////////////////////////