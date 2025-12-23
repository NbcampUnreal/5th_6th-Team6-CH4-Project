
#include "UI/UW_HPBar.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"


void UUW_HPBar::SetHP(float CurrentHP, float MaxHP)
{
	if (!HPProgressBar || !HPText || MaxHP <= 0.f)
	{
		return;
	}

	const float ClampedHP = FMath::Clamp(CurrentHP, 0.f, MaxHP);
	const float Percent = ClampedHP / MaxHP;

	HPProgressBar->SetPercent(Percent);

	HPText->SetText(FText::FromString(FString::Printf(TEXT("%.0f / %.0f"), ClampedHP, MaxHP)));

}
