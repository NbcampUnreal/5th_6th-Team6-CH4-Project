#include "AI/AISpawnVolume.h"
#include "Components/BoxComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"


AAISpawnVolume::AAISpawnVolume()
{
 
	// 박스 컴포넌트 생성 및 루트로 설정
	SpawningBox = CreateDefaultSubobject<UBoxComponent>(TEXT("SpawningBox"));
	RootComponent = SpawningBox;
}


void AAISpawnVolume::BeginPlay()
{
    Super::BeginPlay();

    // 서버(권한자)인 경우에만 AI를 생성
    if (HasAuthority())
    {
        for (int32 i = 0; i < MaxAIInstanceCount; i++)
        {
            SpawnAI();
        }
    }
}

FVector AAISpawnVolume::GetRandomPointInBox()
{
	FVector Center = SpawningBox->GetComponentLocation();
	FVector Extents = SpawningBox->GetScaledBoxExtent();

	// UKismetMathLibrary를 사용하면 박스 범위 내 랜덤 위치를 쉽게 구할 수 있다.
	return UKismetMathLibrary::RandomPointInBoundingBox(Center, Extents);
}

void AAISpawnVolume::SpawnAI()
{
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
            // 스폰 시 충돌 처리 설정
            SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

            AActor* SpawnedActor = World->SpawnActor<AActor>(ActorToSpawn, SpawnLocation, SpawnRotation, SpawnParams);

            
            APawn* SpawnedPawn = Cast<APawn>(SpawnedActor);
            if (SpawnedPawn)
            {
                SpawnedPawn->SpawnDefaultController();
            }
        }
    }
}

