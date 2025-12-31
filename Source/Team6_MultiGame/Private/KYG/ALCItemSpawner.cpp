// ALCItemSpawner.cpp

#include "KYG/ALCItemSpawner.h"
#include "Components/BoxComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"  

AALCItemSpawner::AALCItemSpawner()
{
    PrimaryActorTick.bCanEverTick = false;

    // 루트
    SpawnArea = CreateDefaultSubobject<UBoxComponent>(TEXT("SpawnArea"));
    RootComponent = SpawnArea;

    // 그냥 쿼리만, 충돌은 필요 없음
    SpawnArea->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    IntervalA = 10.f;
    IntervalB = 20.f;
    MaxSpawnedItems = 5;
}

void AALCItemSpawner::BeginPlay()
{
    //서버에서만 스폰 동작
    Super::BeginPlay();

    if (HasAuthority())
    {
        ScheduleNextSpawn();
    }
}

void AALCItemSpawner::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // 맵 이동 / 액터 파괴될 때 타이머 정리 -> 크래시 방지
    if (HasAuthority())
    {
        GetWorldTimerManager().ClearTimer(SpawnTimerHandle);
    }

    Super::EndPlay(EndPlayReason);
}

void AALCItemSpawner::ScheduleNextSpawn()
{
    if (!HasAuthority())
        return;

    // 10초 / 20초 중 하나 랜덤 선택
    const float Delay = FMath::RandBool() ? IntervalA : IntervalB;

    GetWorldTimerManager().SetTimer(
        SpawnTimerHandle,
        this,
        &AALCItemSpawner::SpawnRandomItem,
        Delay,
        false
    );
}

FVector AALCItemSpawner::GetRandomPointInBox() const
{
    if (!SpawnArea)
        return GetActorLocation();

    const FVector Extent = SpawnArea->GetScaledBoxExtent();
    const FVector Origin = SpawnArea->GetComponentLocation();

    const FVector RandomOffset(
        FMath::FRandRange(-Extent.X, Extent.X),
        FMath::FRandRange(-Extent.Y, Extent.Y),
        FMath::FRandRange(-Extent.Z, Extent.Z)
    );

    return Origin + RandomOffset;
}

void AALCItemSpawner::SpawnRandomItem()
{
    if (!HasAuthority())
    {return;}

    UWorld* World = GetWorld();
    if (!World)
    {return;}

    //현재 아이템 개수가 Max 이상이면 스폰 스킵
    const int32 ExistingCount = CountExistingItems();
    if (ExistingCount >= MaxSpawnedItems)
    {
        UE_LOG(LogTemp, Log,
            TEXT("[ItemSpawner] Skip spawn. Existing=%d / Max=%d"),
            ExistingCount, MaxSpawnedItems);

        // 그래도 다음 스폰은 예약 (다음에 줄어들 수 있으니까)
        ScheduleNextSpawn();
        return;
    }

    if (ItemClasses.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("[ItemSpawner] No ItemClasses set"));
        ScheduleNextSpawn();
        return;
    }

    // 어떤 아이템을 뿌릴지 랜덤 선택
    const int32 Index = FMath::RandRange(0, ItemClasses.Num() - 1);
    TSubclassOf<AActor> ItemClass = ItemClasses[Index];

    if (!ItemClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("[ItemSpawner] ItemClasses[%d] is null"), Index);
        ScheduleNextSpawn();
        return;
    }

    const FVector SpawnLoc = GetRandomPointInBox();
    const FRotator SpawnRot = FRotator::ZeroRotator;

    FActorSpawnParameters Params;
    Params.SpawnCollisionHandlingOverride =
        ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    AActor* NewItem = World->SpawnActor<AActor>(ItemClass, SpawnLoc, SpawnRot, Params);

    if (NewItem)
    {
        UE_LOG(LogTemp, Log,
            TEXT("[ItemSpawner] Spawned %s at %s (Existing now ~ %d)"),
            *GetNameSafe(NewItem),
            *SpawnLoc.ToString(),
            ExistingCount + 1);
    }
    else
    {
        UE_LOG(LogTemp, Warning,
            TEXT("[ItemSpawner] Failed to spawn item (Class=%s)"),
            *GetNameSafe(ItemClass));
    }

    // 다음 스폰 예약
    ScheduleNextSpawn();
}

// 월드에 현재 살아 있는 "아이템 클래스들" 개수 세기
int32 AALCItemSpawner::CountExistingItems() const
{
    UWorld* World = GetWorld();
    if (!World || ItemClasses.Num() == 0)
        return 0;

    int32 Count = 0;

    for (TActorIterator<AActor> It(World); It; ++It)
    {
        AActor* Actor = *It;
        if (!IsValid(Actor))
            continue;

        // 등록해둔 ItemClasses 중 하나인지 검사
        for (const TSubclassOf<AActor>& ItemClass : ItemClasses)
        {
            if (!ItemClass)
                continue;

            if (Actor->IsA(ItemClass))
            {
                Count++;
                break;
            }
        }
    }

    return Count;
}