// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "SharedCamera.generated.h"

class USpringArmComponent;
class UCameraComponent;

UCLASS()
class TEAM6_MULTIGAME_API ASharedCamera : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASharedCamera();

	// 서버 전용 호출
	UFUNCTION(Server, Reliable)
	void Server_AddLook(const FVector2D& LookInput);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION()
	void OnRep_CameraRotation();

	UPROPERTY(VisibleAnywhere)
	USpringArmComponent* SpringArm;

	UPROPERTY(VisibleAnywhere)
	UCameraComponent* Camera;

	// ? 핵심: 카메라 회전 상태
	UPROPERTY(ReplicatedUsing = OnRep_CameraRotation)
	FRotator ReplicatedRotation;

};