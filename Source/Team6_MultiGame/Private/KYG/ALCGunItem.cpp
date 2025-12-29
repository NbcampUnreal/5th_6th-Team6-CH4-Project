// ALCGunItem.cpp

#include "KYG/ALCGunItem.h"
#include "KYG/ALCGunBase.h"
#include "Character/Squirrel.h"  
#include "GameFramework/Character.h"
#include "KYG/KYGTestCharacter.h"	//나중에 지워도됨
#include "Character/Squirrel.h"

//아이템을 주웠을 때 호출되는 함수
void AALCGunItem::OnPickedUp(ACharacter* Character)
{
	//서버에서만 아이템 획득/장착 처리
	if (!HasAuthority() || !GunClass || !Character)
		return;

	UWorld* World = GetWorld();
	if (!World) return;

	// Squirrel로 캐스팅
	ASquirrel* Squirrel = Cast<ASquirrel>(Character);
	if (!Squirrel)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GunItem] Character %s is not ASquirrel"), *Character->GetName());
		return;
	}

	FActorSpawnParameters Params;
	Params.Owner = Squirrel;
	Params.Instigator = Squirrel;

	AALCGunBase* NewGun = World->SpawnActor<AALCGunBase>(GunClass, Params);
	if (!NewGun)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GunItem] Failed to spawn gun"));
		return;
	}

	// Squirrel에게 장착 요청 (서버 함수)
	Squirrel->ServerEquipGun(NewGun);

	UE_LOG(LogTemp, Warning, TEXT("[GunItem] %s picked up gun %s"),
		*Squirrel->GetName(), *NewGun->GetName());



	////현재 월드 가져오기
	//UWorld* World = GetWorld();
	//if (!World) 
	//{ return; }

	////총기 스폰 피라미터
	//FActorSpawnParameters Params;
	//Params.Owner = Character;		//총의 소유자
	//Params.Instigator = Cast<APawn>(Character);//총의 발사 시 피해 원인 추적

	////새 총 스폰
	//AALCGunBase* NewGun = World->SpawnActor<AALCGunBase>(GunClass, Params);
	//if (!NewGun) { return; }

	//총 스폰 성공 시 캐릭터 손 소켓에 부착
	//if (Gun)
	//{
	//	Gun->AttachToComponent(
	//		Character->GetMesh(),		//캐릭터의 스켈레탈 메시
	//		FAttachmentTransformRules::SnapToTargetNotIncludingScale,
	//		TEXT("Hand_R_Socket")	//캐릭터의 오른손 소켓 이름
	//	);

	//}
	
		//캐릭터가 "현재 들고 있는 총"을 알도록 전달
		/*if (AKYGTestCharacter* TestChar = Cast<AKYGTestCharacter>(Character))
		{
			TestChar->ServerEquipGun(NewGun);
		}*/
	//Destroy();
}
