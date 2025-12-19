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


	UPROPERTY(Replicated)
	FRotator ReplicatedViewRotation;

	virtual FRotator GetViewRotation() const override;

	virtual void GetLifetimeReplicatedProps(
		TArray<FLifetimeProperty>& OutLifetimeProps
	) const override;

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



};
