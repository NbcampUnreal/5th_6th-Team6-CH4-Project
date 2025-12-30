#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BaseAICharacter.generated.h"

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
    class UAnimMontage* DeathMontage = nullptr;
};

UCLASS()
class TEAM6_MULTIGAME_API ABaseAICharacter : public ACharacter
{
    GENERATED_BODY()

public:
    ABaseAICharacter();

    // 외형 설정
    UPROPERTY(EditAnywhere, Category = "AI|Appearance")
    TArray<FAIAppearanceSet> AppearancePresets;

    UPROPERTY(ReplicatedUsing = OnRep_SelectedAppearanceIndex)
    int32 SelectedAppearanceIndex = -1;

    UFUNCTION()
    void OnRep_SelectedAppearanceIndex();

    // 공격 로직
    UFUNCTION(BlueprintCallable, Category = "AI|Combat") 
        void PlayAttackMontage();

    UFUNCTION(BlueprintCallable, Category = "AI|Combat") 
        void OnAttackHitCheck();

    UFUNCTION(NetMulticast, Reliable)
    void MulticastPlayAttackMontage();

    // 데미지 수신
    virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

    // AIController에서 죽었는지 확인할 수 있는 함수 
    FORCEINLINE bool IsDead() const { return bIsDead; }

protected:
    virtual void BeginPlay() override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    void ApplyAppearance();
    virtual void PostNetInit() override;

    // 전투 및 스탯 설정
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
};