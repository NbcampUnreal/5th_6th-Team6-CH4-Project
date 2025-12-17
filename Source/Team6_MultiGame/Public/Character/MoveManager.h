// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MoveManager.generated.h"

UCLASS()
class TEAM6_MULTIGAME_API AMoveManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMoveManager();

	UFUNCTION()
	void OnClientWInput(APlayerController* PC, const FVector2D& MoveInput);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_DebugWInput(const FString& Msg);

	UPROPERTY()
	USceneComponent* Root;

};
