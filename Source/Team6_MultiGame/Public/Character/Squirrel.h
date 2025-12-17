// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Squirrel.generated.h"

class ASharedCamera;

UCLASS()
class TEAM6_MULTIGAME_API ASquirrel : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ASquirrel();

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

	UPROPERTY(ReplicatedUsing = OnRep_SharedCamera)
	class ASharedCamera* SharedCamera;

	UFUNCTION(BlueprintCallable)
	class ASharedCamera* GetSharedCamera() const { return SharedCamera; }

	UFUNCTION()
	void OnRep_SharedCamera();
	virtual void GetLifetimeReplicatedProps(
		TArray<FLifetimeProperty>& OutLifetimeProps) const override;

};
