// ALCProjectileBase.cpp

#include "KYG/ALCProjectileBase.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "KYG/LCDamageable.h"

AALCProjectileBase::AALCProjectileBase()
{
	//발사체 모든 클라이언트에 복제
	bReplicates = true;

	//충돌 컴포넌트
	CollisionComp = CreateDefaultSubobject<USphereComponent>("Collision");
	RootComponent = CollisionComp;

	CollisionComp->InitSphereRadius(8.f);

	//총알이 물리 충돌 없이 오버렙만 체크
	CollisionComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionComp->SetCollisionResponseToAllChannels(ECR_Overlap);

	//오버렙 발생 시 OnHit호출
	CollisionComp->OnComponentBeginOverlap.AddDynamic(this, &AALCProjectileBase::OnHit);

	//총알 움직임 생성
	MovementComp = CreateDefaultSubobject<UProjectileMovementComponent>("MoveComp");
	MovementComp->InitialSpeed = 9000.f;
	MovementComp->MaxSpeed = 9000.f;
	MovementComp->ProjectileGravityScale = 0.f;	//중력 영향 x
}

//발사 초기 세팅 데미지 크기, 방향, 속도
void AALCProjectileBase::Init(float InDamage, FVector Direction, float Speed)
{
	Damage = InDamage;

	MovementComp->Velocity = Direction.GetSafeNormal() * Speed;
}

//오버렙 이벤트 동작 
void AALCProjectileBase::OnHit(
	UPrimitiveComponent* Overlapped,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& Hit)
{	//서버에서만 대미지 처리
	if (!HasAuthority()) 
	{ return; }
	
	//실제 대미지를 받을 Actor
	AActor* DamageTarget = OtherActor;

	if (OtherComp)
	{
		DamageTarget = OtherComp->GetOwner();
	}

	//OtherActor가 Damageable 인터페이스를 구현했는지 확인
	//if (OtherActor && OtherActor->GetClass()->ImplementsInterface(ULCDamageable::StaticClass()))
	if (DamageTarget &&
		DamageTarget->GetClass()->ImplementsInterface(ULCDamageable::StaticClass()))
	{
		//인터페이스 함수 호출
		ILCDamageable::Execute_ReceiveDamage(OtherActor, Damage, GetInstigator());
	}
	//처리 후 곧바로 발사체 제거
	Destroy();
}
