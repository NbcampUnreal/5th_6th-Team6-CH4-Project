#include "AI/AISpawnVolume.h"
#include "Components/BoxComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"
#include "AI/BaseAICharacter.h" 
#include "CharacterGameMode/CharacterGameMode.h"        


AAISpawnVolume::AAISpawnVolume()
{
	SpawningBox = CreateDefaultSubobject<UBoxComponent>(TEXT("SpawningBox"));
	RootComponent = SpawningBox;

	// 초기값 설정
	MaxAIInstanceCount = 5;
	CurrentLivingAICount = 0;
}

void AAISpawnVolume::BeginPlay()
{
	Super::BeginPlay();
	// 서버에서만 AI 스폰 로직 실행
	if (HasAuthority())
	{
		// 초기 살아있는 AI 수 설정
		CurrentLivingAICount = MaxAIInstanceCount;
		// 지정된 수만큼 AI 스폰
		for (int32 i = 0; i < MaxAIInstanceCount; i++)
		{
			SpawnAI();
		}
	}
}
// 박스 컴포넌트 내의 랜덤 위치 반환
FVector AAISpawnVolume::GetRandomPointInBox()
{
	FVector Center = SpawningBox->GetComponentLocation();// 박스의 중심 위치
	FVector Extents = SpawningBox->GetScaledBoxExtent(); // 박스의 반지름 크기
	return UKismetMathLibrary::RandomPointInBoundingBox(Center, Extents); // 랜덤 위치 계산
}

// 실제 AI 스폰 함수
void AAISpawnVolume::SpawnAI()
{
	if (ActorToSpawn) // 스폰할 액터가 지정되어 있는지 확인
	{
		UWorld* World = GetWorld();
		if (World)
		{
			FVector SpawnLocation = GetRandomPointInBox(); 
			FRotator SpawnRotation = FRotator::ZeroRotator; 

			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = this;
			SpawnParams.Instigator = GetInstigator();
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

			AActor* SpawnedActor = World->SpawnActor<AActor>(ActorToSpawn, SpawnLocation, SpawnRotation, SpawnParams);

			// 1. AI 캐릭터의 사망 델리게이트에 바인딩
			ABaseAICharacter* AIChar = Cast<ABaseAICharacter>(SpawnedActor);
			if (AIChar)
			{
				// 이 볼륨의 HandleAIDeath 함수를 등록함
				AIChar->OnAICharacterDeadDelegate.AddDynamic(this, &AAISpawnVolume::HandleAIDeath);
			}

			APawn* SpawnedPawn = Cast<APawn>(SpawnedActor);
			if (SpawnedPawn)
			{
				SpawnedPawn->SpawnDefaultController();
			}
		}
	}
}

// AI 사망 시 실행될 로직 구현
void AAISpawnVolume::HandleAIDeath(AActor* DeadActor)
{
	// 서버에서만 로직을 처리하도록 보장
	if (!HasAuthority()) return;

	CurrentLivingAICount--;

	if (CurrentLivingAICount <= 0)
	{
		if (UWorld* World = GetWorld())
		{
			
			ACharacterGameMode* GM = Cast<ACharacterGameMode>(World->GetAuthGameMode());
			if (GM)
			{
				GM->EndGame();
			
			}
		}
	}
}