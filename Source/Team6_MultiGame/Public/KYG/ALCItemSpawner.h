// ALCItemSpawner.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ALCItemSpawner.generated.h"

class UBoxComponent;

UCLASS()
class TEAM6_MULTIGAME_API AALCItemSpawner : public AActor
{
	GENERATED_BODY()
	
public:    
    AALCItemSpawner();

protected:
    virtual void BeginPlay() override;

    //맵 종료시 타이머 종료
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    // ===== 스폰 영역 =====
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spawn")
    UBoxComponent* SpawnArea;

    // ===== 어떤 아이템들을 랜덤 스폰할지 ===== 
    // 여기다가 BP_ALCGunItem, BP_MP_pistolItem, BP_ALCHealingActor 넣어줄 거임
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn")
    TArray<TSubclassOf<AActor>> ItemClasses;

    //===== 스폰 간격 ===== 
    // 10초 / 20초 중에서 랜덤 선택
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn")
    float IntervalA = 10.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn")
    float IntervalB = 20.f;

    // ===== 최대 아이템 개수 제한 ===== 
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn|Limit")
    int32 MaxSpawnedItems = 5;  //5개로 제한

    FTimerHandle SpawnTimerHandle;

    UFUNCTION()
    void SpawnRandomItem();

    void ScheduleNextSpawn();

    // 박스 안 랜덤 위치 뽑는 헬퍼
    FVector GetRandomPointInBox() const;

    // 월드에 현재 살아 있는 아이템 개수 세기
    int32 CountExistingItems() const;
};