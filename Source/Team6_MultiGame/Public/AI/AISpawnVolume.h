#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AISpawnVolume.generated.h"

UCLASS()
class TEAM6_MULTIGAME_API AAISpawnVolume : public AActor
{
	GENERATED_BODY()
	
public:	
	
	AAISpawnVolume();

protected:
	
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spawning")
	class UBoxComponent* SpawningBox;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawning")
	TSubclassOf<AActor> ActorToSpawn;

	// 한 번에 스폰할 AI의 총 마리 수 (에디터에서 수정 가능)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawning")
	int32 MaxAIInstanceCount = 10;

	// 스폰 간격 조절
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawning")
	float SpawnInterval = 4.0f;

	// 지금까지 스폰을 시도한 총 횟수
	int32 SpawnedSoFarCount;

	//  순차 스폰 타이머를 관리할 핸들
	FTimerHandle SpawnTimerHandle;

	// 현재 살아있는 AI 숫자를 저장할 변수
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawning")
	int32 CurrentLivingAICount;

	// AI가 죽었을 때 호출되는 함수
	UFUNCTION()
	void HandleAIDeath(AActor* DeadActor);


public:	
	// 박스 안의 랜덤 위치를 계산하는 함수
	UFUNCTION(BlueprintCallable, Category = "Spawning")
	FVector GetRandomPointInBox();

	// 실제 스폰을 수행하는 함수
	void SpawnAI();

};
