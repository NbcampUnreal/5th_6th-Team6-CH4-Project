#include "UI/UW_KeyGuide.h"
#include "Components/Border.h"

void UUW_KeyGuide::NativeConstruct()
{
	Super::NativeConstruct();

	KeyBorderMap.Reset();
	KeyBorderMap.Add("W", Border_W);
	KeyBorderMap.Add("A", Border_A);
	KeyBorderMap.Add("S", Border_S);
	KeyBorderMap.Add("D", Border_D);
	KeyBorderMap.Add("MouseL", Border_MouseL);
	KeyBorderMap.Add("MouseR", Border_MouseR);

	ResetAllKeys();
}

void UUW_KeyGuide::SetKeyPressed(FName KeyName, bool bPressed)
{
	if (UBorder** Found = KeyBorderMap.Find(KeyName))
	{
		if (*Found)
		{
			(*Found)->SetBrushColor(bPressed ? PressedColor : DefaultColor);
		}
	}
}

void UUW_KeyGuide::ResetAllKeys()
{
	for (const TPair<FName, UBorder*>& Pair : KeyBorderMap)
	{
		if (Pair.Value)
		{
			Pair.Value->SetBrushColor(DefaultColor);
		}
	}
}


