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
	MaxAIInstanceCount = 10;
	CurrentLivingAICount =0;
}

void AAISpawnVolume::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		SpawnedSoFarCount = 0; // 지금까지 스폰한 수 초기화
		CurrentLivingAICount = 0;

		
		GetWorld()->GetTimerManager().SetTimer(
			SpawnTimerHandle,
			this,
			&AAISpawnVolume::SpawnAI,
			SpawnInterval, 
			true           
		);
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
	
	if (SpawnedSoFarCount >= MaxAIInstanceCount)
	{
		GetWorld()->GetTimerManager().ClearTimer(SpawnTimerHandle);
		return;
	}

	if (ActorToSpawn)
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

			if (SpawnedActor) // 스폰에 성공했을 때만 카운트 증가
			{
				SpawnedSoFarCount++;    // 총 스폰 시도 횟수 증가
				CurrentLivingAICount++; // 현재 살아있는 수 증가

				ABaseAICharacter* AIChar = Cast<ABaseAICharacter>(SpawnedActor);
				if (AIChar)
				{
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
}

// AI가 죽었을 때 호출되는 함수
void AAISpawnVolume::HandleAIDeath(AActor* DeadActor)
{
	if (!HasAuthority()) return;

	CurrentLivingAICount--;
	// 로그 출력
	UE_LOG(LogTemp, Warning, TEXT("[SpawnVolume] AI Died. Remaining: %d, SpawnedSoFar: %d/%d"),
		CurrentLivingAICount, SpawnedSoFarCount, MaxAIInstanceCount);

	// 1. 더 이상 스폰할 예정이 없는지 확인 (타이머가 끝났거나 카운트를 채웠거나)
	bool bNoMoreSpawning = (SpawnedSoFarCount >= MaxAIInstanceCount) || !GetWorld()->GetTimerManager().IsTimerActive(SpawnTimerHandle);

	// 2. 스폰이 끝났고 + 살아있는 놈이 0이라면 클리어!
	if (bNoMoreSpawning && CurrentLivingAICount <= 0)
	{
		ACharacterGameMode* GM = Cast<ACharacterGameMode>(GetWorld()->GetAuthGameMode());
		if (GM)
		{
			GM->ClearGame();
		}
	}
}