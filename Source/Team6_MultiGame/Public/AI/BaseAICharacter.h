#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BaseAICharacter.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAICharacterDeadSignature, AActor*, DeadActor);

// AI 외형 설정 구조체
USTRUCT(BlueprintType)
struct FAIAppearanceSet
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    class USkeletalMesh* Mesh = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSubclassOf<UAnimInstance> AnimBlueprint = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    class UAnimMontage* AttackMontage = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UAnimMontage* HitMontage  = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    class UAnimMontage* DeathMontage = nullptr;
};

UCLASS()
class TEAM6_MULTIGAME_API ABaseAICharacter : public ACharacter
{
    GENERATED_BODY()

public:
    ABaseAICharacter();

    // [추가] SpawnVolume이 이 델리게이트를 보고 죽음을 감지
    UPROPERTY(BlueprintAssignable, Category = "AI|Events")
    FOnAICharacterDeadSignature OnAICharacterDeadDelegate;

    UPROPERTY(EditAnywhere, Category = "AI|Appearance")
    TArray<FAIAppearanceSet> AppearancePresets;

    // [중요] 여기에 한 번만 선언하면 됩니다!
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Effects")
    class UMaterialInterface* DissolveMasterMaterial;

    UPROPERTY(ReplicatedUsing = OnRep_SelectedAppearanceIndex)
    int32 SelectedAppearanceIndex = -1;

    UFUNCTION()
    void OnRep_SelectedAppearanceIndex();

    UFUNCTION(BlueprintCallable, Category = "AI|Combat")
    void PlayAttackMontage();

    UFUNCTION(NetMulticast, Unreliable)
    void MulticastPlayHitMontage();

    UFUNCTION(BlueprintCallable, Category = "AI|Combat")
    void OnAttackHitCheck();

    UFUNCTION(NetMulticast, Reliable)
    void MulticastPlayAttackMontage();

    virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

    FORCEINLINE bool IsDead() const { return bIsDead; }

    UFUNCTION(BlueprintCallable, Category = "AI|Death")
    void FinishDying();

    UFUNCTION(BlueprintCallable, Category = "AI|Death")
    void TriggerDissolveEffect();

protected:
    virtual void BeginPlay() override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    void ApplyAppearance();
    virtual void PostNetInit() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Combat")
    float AttackRange = 150.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Combat")
    float AttackDamage = 10.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Stat")
    float MaxHP = 100.f;

    UPROPERTY(ReplicatedUsing = OnRep_CurrentHP)
    float CurrentHP;

    UFUNCTION()
    void OnRep_CurrentHP();

    UPROPERTY(ReplicatedUsing = OnRep_IsDead)
    bool bIsDead = false;

    UFUNCTION()
    void OnRep_IsDead();

    UFUNCTION(NetMulticast, Reliable)
    void MulticastPlayDeath();

    void Die();

    UPROPERTY()
    class UMaterialInstanceDynamic* DynamicDissolveMaterial;

    UFUNCTION(BlueprintImplementableEvent, Category = "AI|Effects")
    void BP_StartDissolveEffect();

    UFUNCTION(BlueprintCallable, Category = "AI|Effects")
    void UpdateDissolveParameter(float DissolveValue);

    void StartDissolveAfterAnim(UAnimMontage* Montage, bool bInterrupted);

};