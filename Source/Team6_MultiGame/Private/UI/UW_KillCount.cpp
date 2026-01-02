// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/UW_KillCount.h"
#include "Components/TextBlock.h"

void UUW_KillCount::NativeConstruct()
{
	Super::NativeConstruct();

	if (KillCountText)
	{
		KillCountText->SetText(FText::AsNumber(0));
	}
}

void UUW_KillCount::SetKillCount(int32 NewKillCount)
{
	if (KillCountText)
	{
		KillCountText->SetText(FText::AsNumber(NewKillCount));
	}
}
