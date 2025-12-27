#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UW_KeyGuide.generated.h"

class UBorder;

/**
 * 
 */
UCLASS()
class TEAM6_MULTIGAME_API UUW_KeyGuide : public UUserWidget
{
	GENERATED_BODY()
public:
	
	void SetKeyPressed(FName KeyName, bool bPressed);
	void ResetAllKeys();

protected:
	virtual void NativeConstruct() override;

protected:
	
	UPROPERTY(meta = (BindWidget)) UBorder* Border_W;
	UPROPERTY(meta = (BindWidget)) UBorder* Border_A;
	UPROPERTY(meta = (BindWidget)) UBorder* Border_S;
	UPROPERTY(meta = (BindWidget)) UBorder* Border_D;

	UPROPERTY(meta = (BindWidgetOptional)) UBorder* Border_MouseL;
	UPROPERTY(meta = (BindWidgetOptional)) UBorder* Border_MouseR;

private:
	
	TMap<FName, UBorder*> KeyBorderMap;


	FLinearColor DefaultColor = FLinearColor(0.15f, 0.15f, 0.15f, 0.8f);
	FLinearColor PressedColor = FLinearColor(0.2f, 0.6f, 1.f, 1.f);
};
