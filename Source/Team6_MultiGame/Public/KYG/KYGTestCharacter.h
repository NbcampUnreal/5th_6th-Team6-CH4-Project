// KYGTestCharacter.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "KYG/LCHealable.h"
#include "KYGTestCharacter.generated.h"

class UInputMappingContext;
class UInputAction;
class AALCGunBase;

UCLASS()
class TEAM6_MULTIGAME_API AKYGTestCharacter : public ACharacter, public ILCHealable
{
	GENERATED_BODY()

public:
	AKYGTestCharacter();

    virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	//==발사 함수==
	void SetEquippedGun_Server(AALCGunBase* NewGun);
	// 테스트용 발사
	UFUNCTION(BlueprintCallable)
	void InputFire();

	virtual void ReceiveHeal_Implementation(float HealAmount) override;

	UFUNCTION(Server, Reliable)
	void ServerEquipGun(AALCGunBase* NewGun);
	void ServerEquipGun_Implementation(AALCGunBase* NewGun);
protected:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Health")
	float MaxHP = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Health")
	float CurrentHP = 100.f;

	//이동 함수
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputMappingContext* IMC_Default;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* IA_Move;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* IA_Look;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* IA_Fire;

	//현재 장착 중인 총(서버에서 클라 복제)
	UPROPERTY(ReplicatedUsing = OnRep_EquippedGun)
	AALCGunBase* EquippedGun;

	UFUNCTION()
	void OnRep_EquippedGun();

	// 이동 처리
	void Move(const FInputActionValue& Value);
	// 마우스 보기 처리
	void Look(const FInputActionValue& Value);
};
