

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Sound/SoundClass.h"
#include "Sound/SoundMix.h"
#include "VoiceVolumeSubsystem.generated.h"


UCLASS()
class TEAM6_MULTIGAME_API UVoiceVolumeSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
    UVoiceVolumeSubsystem();

    //  상대방 목소리(수신) 볼륨: 0.0 ~ 1.0
    UFUNCTION(BlueprintCallable, Category = "Voice")
    void SetVoiceReceiveVolume(float Volume01);

    UFUNCTION(BlueprintCallable, Category = "Voice")
    float GetVoiceReceiveVolume() const { return VoiceReceiveVolume01; }

protected:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

private:
    // 프로젝트 에셋(필수):
    // - Project Settings > Audio > VOIP Sound Class 에도 같은 SoundClass를 지정해줘야 "보이스만" 분리 조절됨
    UPROPERTY(EditDefaultsOnly, Category = "Voice")
    TObjectPtr<USoundClass> VoiceSoundClass = nullptr;

    UPROPERTY(EditDefaultsOnly, Category = "Voice")
    TObjectPtr<USoundMix> VoiceSoundMix = nullptr;

private:
    float VoiceReceiveVolume01 = 1.0f;
    bool bMixPushed = false;
};
