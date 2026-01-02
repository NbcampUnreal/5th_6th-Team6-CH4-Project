// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UW_KillCount.generated.h"

class UTextBlock;
/**
 * 
 */
UCLASS()
class TEAM6_MULTIGAME_API UUW_KillCount : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void SetKillCount(int32 NewKillCount);

protected:
	virtual void NativeConstruct() override;

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> KillCountText;
};
