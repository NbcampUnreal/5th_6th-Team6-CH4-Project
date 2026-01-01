// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UW_Result.generated.h"

class UTextBlock;
class UButton;
class AMainPlayerController;
/**
 * 
 */
UCLASS()
class TEAM6_MULTIGAME_API UUW_Result : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(meta = (BindWidget))
	UTextBlock* ResultText;

	UPROPERTY(meta = (BindWidget))
	UButton* RestartButton;

	UPROPERTY(meta = (BindWidget))
	UButton* ExitButton;

	virtual void NativeConstruct() override;

private:

	UFUNCTION()
	void OnRestartClicked();

	UFUNCTION()
	void OnExitClicked();

	/////////////////////////////////////////////////   수정   /////////////////////////////////////////////////
private:
	TWeakObjectPtr<AMainPlayerController> Player;

public:
	void SetTitleText(bool bClear);

	/////////////////////////////////////////////////   수정   /////////////////////////////////////////////////
};
