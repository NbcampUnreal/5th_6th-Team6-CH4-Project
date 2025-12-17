// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/UW_Result.h"

#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

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

void UUW_Result::OnRestartClicked()
{
	FString CurrentLevel = UGameplayStatics::GetCurrentLevelName(this, true);
	UGameplayStatics::OpenLevel(this, FName(*CurrentLevel));
}

void UUW_Result::OnExitClicked()
{
	UKismetSystemLibrary::QuitGame(this, nullptr, EQuitPreference::Quit, true);
}
