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
	int32 MaxAIInstanceCount = 5;

	// (선택 사항) 스폰 간격을 조절하고 싶다면 추가
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawning")
	float SpawnInterval = 2.0f;


public:	
	// 박스 안의 랜덤 위치를 계산하는 함수
	UFUNCTION(BlueprintCallable, Category = "Spawning")
	FVector GetRandomPointInBox();

	// 실제 스폰을 수행하는 함수
	void SpawnAI();

};
