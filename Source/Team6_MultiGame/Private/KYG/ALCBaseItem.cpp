// ALCBaseItem.cpp

#include "KYG/ALCBaseItem.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"

AALCBaseItem::AALCBaseItem()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;	//클라이언트로 복제
	
	//충돌용 Sphere 컴포넌트
	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComp"));
	RootComponent = CollisionComp;

	CollisionComp->SetSphereRadius(60.f);

	//아이템은 물리 충돌 필요 없음
	CollisionComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionComp->SetCollisionResponseToAllChannels(ECR_Overlap);

	//플레이어가 근처 들어오면 OnItemOverlap 호출
	CollisionComp->OnComponentBeginOverlap.AddDynamic(this, &AALCBaseItem::OnItemOverlap);
}

void AALCBaseItem::BeginPlay()
{
	Super::BeginPlay();
}

//아이템 오버렙 이벤트
void AALCBaseItem::OnItemOverlap(
	UPrimitiveComponent* OverlappedComponent, 
	AActor* OtherActor, UPrimitiveComponent* 
	OtherComp, int32 OtherBodyIndex, 
	bool bFromSweep, 
	const FHitResult& SweepResult)
{
	//서버에서 처리(서버에서만 아이템 획득 처리 권한 가짐)
	if(HasAuthority() == false)
	{return;}

	if(!IsValid(OtherActor))
	{return;}

	//ACharacter만 아이템을 습득 가능
	ACharacter* Character = Cast<ACharacter>(OtherActor);
	if(!Character)
	{return;}

	UE_LOG(LogTemp, Warning, TEXT("[SERVER] %s Get Item!"), *GetName());

	OnPickedUp(Character);

	//아이템을 월드에서 제거 클라에서도 같이 제거됨
	Destroy();
}

void AALCBaseItem::OnPickedUp(ACharacter* Character)
{
	// 기본 아이템은 아무 효과 없음
	UE_LOG(LogTemp, Warning, TEXT("BaseItem picked up"));
}
