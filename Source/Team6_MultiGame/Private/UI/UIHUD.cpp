#include "UI/UIHUD.h"
#include "UI/UW_HPBar.h"
#include "UI/UW_KeyGuide.h"
#include "Character/Squirrel.h"

void UUIHUD::NativeConstruct()
{
    Super::NativeConstruct();
}

void UUIHUD::UpdateHP(float CurrentHP, float MaxHP)
{
	if (HPBarWidget)
	{
		HPBarWidget->SetHP(CurrentHP, MaxHP);
	}
}

void UUIHUD::SetKeyPressed(FName KeyName, bool bPressed)
{
	if (KeyGuideWidget)
	{
		KeyGuideWidget->SetKeyPressed(KeyName, bPressed);
	}
}

void UUIHUD::ResetAllKeys()
{
	if (KeyGuideWidget)
	{
		KeyGuideWidget->ResetAllKeys();
	}
}