// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Squirrel.generated.h"

class USpringArmComponent;
class UCameraComponent;

UCLASS()
class TEAM6_MULTIGAME_API ASquirrel : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ASquirrel();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override; 

	// === Look 적용(서버 권위) ===
	void ApplyLook_ServerAuth(const FVector2D& LookInput); 

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

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
	UPROPERTY(ReplicatedUsing = OnRep_ViewRot) // [ADD]
		FRotator RepViewRot;                    // [ADD]

	UFUNCTION()                              // [ADD]
		void OnRep_ViewRot();                    // [ADD]

};
