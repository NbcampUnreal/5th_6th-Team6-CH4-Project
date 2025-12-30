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

	// 서버 권위 점프
	void Jump_ServerAuth();
	void StopJump_ServerAuth();
	// 서버 권위 스프린트
	void SetSprinting_ServerAuth(bool bNewSprinting);


	//Sprinting
	UPROPERTY(ReplicatedUsing = OnRep_IsJog, BlueprintReadOnly, Category = "Move")
	bool bIsJog = false;

	

	UFUNCTION()
	void RequestDash_ServerAuth();
	// ===== Dash Replication =====
	UPROPERTY(ReplicatedUsing = OnRep_IsDash, BlueprintReadOnly, Category = "Move")
	bool bIsDash = false;

	UFUNCTION()
	void OnRep_IsDash();

	// 서버용 쿨다운/타이머
	float NextDashAllowedTime = 0.f;

	FTimerHandle DashEndTimerHandle;

	// 대쉬 “활성 유지 시간”(애님 길이에 맞춰 조절)
	UPROPERTY(EditDefaultsOnly, Category = "Dash")
	float DashActiveTime = 0.8f;

	UPROPERTY(EditDefaultsOnly, Category = "Dash")
	float DashCooldownTime = 5.f;

	void EndDash_ServerAuth();
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// 현재 장착한 총(서버가 세팅, 클라는 OnRep에서 부착)
	UPROPERTY(ReplicatedUsing = OnRep_CurrentGun)
	AALCGunBase* CurrentGun = nullptr;

	UFUNCTION()
	void OnRep_CurrentGun(); // [FIX]

	// 속도(걷기 300 / 뛰기 600)
	UPROPERTY(EditDefaultsOnly, Category = "Move")
	float WalkSpeed = 300.f;

	UPROPERTY(EditDefaultsOnly, Category = "Move")
	float SprintSpeed = 600.f;


	// [FIX] Attach는 단일 함수로
	void AttachCurrentGun();

	
	UFUNCTION()
	void OnRep_IsJog();

	void ApplySprintSpeed();
	

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
