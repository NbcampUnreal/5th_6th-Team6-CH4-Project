#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UIHUD.generated.h"

class UUW_HPBar;
class UUW_KeyGuide;

UCLASS()
class TEAM6_MULTIGAME_API UUIHUD : public UUserWidget
{
	GENERATED_BODY()
	

protected:
	virtual void NativeConstruct() override;

public:

	UFUNCTION(BlueprintCallable)
	void UpdateHP(float CurrentHP, float MaxHP);

	UFUNCTION(BlueprintCallable)
	void SetKeyPressed(FName KeyName, bool bPressed);

	UFUNCTION(BlueprintCallable)
	void ResetAllKeys();

protected:

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UUW_HPBar> HPBarWidget;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UUW_KeyGuide> KeyGuideWidget;


};
