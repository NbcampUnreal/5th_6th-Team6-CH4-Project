// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UW_HPBar.generated.h"

class UProgressBar;
class UTextBlock;

/**
 * 
 */
UCLASS()
class TEAM6_MULTIGAME_API UUW_HPBar : public UUserWidget
{
	GENERATED_BODY()
	
public:

	UFUNCTION(BlueprintCallable)
	void SetHP(float CurrentHP, float MaxHP);

protected:

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> HPProgressBar;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> HPText;


};
