// Fill out your copyright notice in the Description page of Project Settings.


#include "PGH/PGHActor.h"

// Sets default values
APGHActor::APGHActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void APGHActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void APGHActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

