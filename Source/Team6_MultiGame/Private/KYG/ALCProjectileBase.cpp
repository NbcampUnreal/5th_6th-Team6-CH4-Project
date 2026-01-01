// ALCProjectileBase.cpp

#include "KYG/ALCProjectileBase.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"   
//#include "KYG/LCDamageable.h"
#include "Kismet/GameplayStatics.h"

AALCProjectileBase::AALCProjectileBase()
{
	PrimaryActorTick.bCanEverTick = false;
	//발사체 모든 클라이언트에 복제
	bReplicates = true;
	SetReplicateMovement(true);

	//충돌 컴포넌트
	CollisionComp = CreateDefaultSubobject<USphereComponent>("Collision");
	RootComponent = CollisionComp;

	CollisionComp->InitSphereRadius(8.f);

	//총알이 물리 충돌 없이 오버렙만 체크
	CollisionComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionComp->SetCollisionObjectType(ECC_WorldDynamic);
	CollisionComp->SetCollisionResponseToAllChannels(ECR_Overlap);
	CollisionComp->SetGenerateOverlapEvents(true);

	//오버렙 발생 시 OnHit호출
	CollisionComp->OnComponentBeginOverlap.AddDynamic(this, &AALCProjectileBase::OnHit);

	//시각용
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetupAttachment(CollisionComp);
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	//총알 움직임 생성
	MovementComp = CreateDefaultSubobject<UProjectileMovementComponent>("MoveComp");
	MovementComp->InitialSpeed = 3000.f;
	MovementComp->MaxSpeed = 3000.f;
	MovementComp->ProjectileGravityScale = 0.f;	//중력 영향 x

	// 자동 제거 
	SetLifeSpan(1.0f);
}

void AALCProjectileBase::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogTemp, Warning,
		TEXT("[Projectile] BeginPlay Name=%s HasAuth=%d NetMode=%d Role=%d"),
		*GetName(),
		HasAuthority() ? 1 : 0,
		(int32)GetWorld()->GetNetMode(),
		(int32)GetLocalRole());

	SpawnLocation = GetActorLocation();

	// 클라이언트(또는 리슨의 로컬)에서만 트레일 붙이기
	if (LaserVFX && RootComponent)
	{
		UNiagaraFunctionLibrary::SpawnSystemAttached(
			LaserVFX,
			RootComponent,                 // CollisionComp(=Root)에 붙음
			NAME_None,
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			EAttachLocation::KeepRelativeOffset,
			true,                          // bAutoDestroy
			true,                          // bAutoActivate
			ENCPoolMethod::None,
			true
		);
	}

}

//발사 초기 세팅 데미지 크기, 방향, 속도
void AALCProjectileBase::Init(float InDamage, FVector Direction, float Speed)
{
	Damage = InDamage;

	if (MovementComp)
	{
		const FVector Dir = Direction.GetSafeNormal();
		MovementComp->Velocity = Dir * Speed;
	}

	// 발사자 무시
	if (AActor* OwnerActor = GetOwner())
	{
		CollisionComp->IgnoreActorWhenMoving(OwnerActor, true);
	}
	if (APawn* Inst = GetInstigator())
	{
		CollisionComp->IgnoreActorWhenMoving(Inst, true);
	}
}

//오버렙 이벤트 동작 
void AALCProjectileBase::OnHit(
	UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{	//서버에서만 대미지 처리
	if (!HasAuthority()) 
	{ return; }
	
	//실제 대미지를 받을 Actor
	AActor* Target = OtherActor;
	if (OtherComp && OtherComp->GetOwner())
	{
		Target = OtherComp->GetOwner();
	}

	if (Target && Target != this )
		//Target->GetClass()->ImplementsInterface(ULCDamageable::StaticClass()))
	{
		UGameplayStatics::ApplyDamage(
			Target,
			Damage,
			GetInstigator() ? GetInstigator()->GetController() : nullptr, // 공격자 컨트롤러
			this,                                                          // DamageCauser = 이 발사체
			UDamageType::StaticClass()
		);
	}

	Destroy();
}
