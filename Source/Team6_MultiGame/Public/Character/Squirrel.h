// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "KYG/LCHealable.h"		//회복아이템을 위해 추가
#include "Squirrel.generated.h"

class USpringArmComponent;
class UCameraComponent;
class AALCGunBase;

UCLASS()
class TEAM6_MULTIGAME_API ASquirrel : public ACharacter, public ILCHealable	//HP관련 처리 추가
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ASquirrel();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override; 

	// === Look 적용(서버 권위) ===
	void ApplyLook_ServerAuth(const FVector2D& LookInput); 

	void Fire_ServerAuth(); // 서버에서만 호출될 발사



protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// 현재 장착한 총(서버가 세팅, 클라는 OnRep에서 부착)
	UPROPERTY(ReplicatedUsing = OnRep_CurrentGun)
	AALCGunBase* CurrentGun = nullptr;

	UFUNCTION()
	void OnRep_CurrentGun(); // [FIX]

	// [FIX] Attach는 단일 함수로
	void AttachCurrentGun();


	//======== HP구현 ==========
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Status")
	float HP;

	UPROPERTY(EditDefaultsOnly, Category = "Status")
	float MaxHP = 100.f;



public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION()
	void Move(const FVector2D& Value);

	/* ===== Camera ===== */

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class USpringArmComponent* SpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class UCameraComponent* Camera;

	// === Replicated View Rotation (카메라 회전 진실값) ===
	UPROPERTY(ReplicatedUsing = OnRep_ViewRot, BlueprintReadOnly, Category = "View") // [ADD]
		FRotator RepViewRot;                    // [ADD]

	UFUNCTION()                              // [ADD]
		void OnRep_ViewRot();                    // [ADD]

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	FName WeaponSocketName = TEXT("Hand_R_Socket");


	// Healable 인터페이스 구현
	virtual void ReceiveHeal_Implementation(float HealAmount) override;

	//=====총기 장착 함수 ======
	UFUNCTION(Server, Reliable)
	void ServerEquipGun(AALCGunBase* NewGun);

	
};
