// ALCBaseItem.cpp

#include "KYG/ALCBaseItem.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"

AALCBaseItem::AALCBaseItem()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComp"));
	RootComponent = CollisionComp;
	CollisionComp->SetSphereRadius(60.f);
	CollisionComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionComp->SetCollisionResponseToAllChannels(ECR_Overlap);

	CollisionComp->OnComponentBeginOverlap.AddDynamic(this, &AALCBaseItem::OnItemOverlap);
}

void AALCBaseItem::BeginPlay()
{
	Super::BeginPlay();
}

void AALCBaseItem::OnItemOverlap(
	UPrimitiveComponent* OverlappedComponent, 
	AActor* OtherActor, UPrimitiveComponent* 
	OtherComp, int32 OtherBodyIndex, 
	bool bFromSweep, 
	const FHitResult& SweepResult)
{
	//서버에서 처리
	if(HasAuthority() == false)
	{return;}

	if(!IsValid(OtherActor))
	{return;}

	ACharacter* Character = Cast<ACharacter>(OtherActor);
	if(!Character)
	{return;}

	UE_LOG(LogTemp, Warning, TEXT("[SERVER] %s Get Item!"), *GetName());

	Destroy();
}


