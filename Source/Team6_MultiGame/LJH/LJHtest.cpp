// Fill out your copyright notice in the Description page of Project Settings.


#include "LJH/LJHtest.h"

// Sets default values
ALJHtest::ALJHtest()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ALJHtest::BeginPlay()
{
	Super::BeginPlay();
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("TEST LJH"));
	
}

// Called every frame
void ALJHtest::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

