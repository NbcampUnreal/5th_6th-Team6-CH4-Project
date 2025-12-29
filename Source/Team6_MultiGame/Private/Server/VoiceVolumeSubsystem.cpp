

#include "Server/VoiceVolumeSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"

void UVoiceVolumeSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    // 시작 시 믹스를 올려두면 이후 SetVoiceReceiveVolume이 항상 반영됨(선택)
    if (VoiceSoundMix && !bMixPushed)
    {
        UGameplayStatics::PushSoundMixModifier(this, VoiceSoundMix);
        bMixPushed = true;
    }

    SetVoiceReceiveVolume(VoiceReceiveVolume01);
}

UVoiceVolumeSubsystem::UVoiceVolumeSubsystem()
{
    static ConstructorHelpers::FObjectFinder<USoundClass> SC(TEXT("/Game/Server/Sound/SC_Voice.SC_Voice"));
    if (SC.Succeeded())
    {
        VoiceSoundClass = SC.Object;
    }

    static ConstructorHelpers::FObjectFinder<USoundMix> SM(TEXT("/Game/Server/Sound/SM_VoiceMix.SM_VoiceMix"));
    if (SM.Succeeded())
    {
        VoiceSoundMix = SM.Object;
    }
}

void UVoiceVolumeSubsystem::SetVoiceReceiveVolume(float Volume01)
{
    VoiceReceiveVolume01 = FMath::Clamp(Volume01, 0.f, 1.f);

    if (!VoiceSoundMix || !VoiceSoundClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("[VoiceVolume] VoiceSoundMix or VoiceSoundClass is null. Set assets in subsystem defaults."));
        return;
    }

    if (!bMixPushed)
    {
        UGameplayStatics::PushSoundMixModifier(this, VoiceSoundMix);
        bMixPushed = true;
    }

    // 보이스 SoundClass 볼륨만 오버라이드
    UGameplayStatics::SetSoundMixClassOverride(
        this,
        VoiceSoundMix,
        VoiceSoundClass,
        VoiceReceiveVolume01,
        1.0f,   // Pitch
        0.0f,   // Fade in
        true    // Apply to children
    );

    UE_LOG(LogTemp, Log, TEXT("[VoiceVolume] Voice receive volume: %.2f"), VoiceReceiveVolume01);
}