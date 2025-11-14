#pragma once

#include "CoreMinimal.h"
#include "Engine/TargetPoint.h"
#include "AuraEnemySpawnPoint.generated.h"

class AAuraEnemy;

UCLASS()
class AURA_API AAuraEnemySpawnPoint : public ATargetPoint
{
	GENERATED_BODY()

public:
	AAuraEnemySpawnPoint();

	UFUNCTION(BlueprintCallable)
	void SpawnEnemy();

protected:
	//~ Begin AActor Interface
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//~ End of AActor Interface

	UFUNCTION()
	void StartSpawnTimer(AActor* DeadActor);
	void EndSpawnTimer();

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "EnemySpawnPoint")
	TSubclassOf<AAuraEnemy> EnemyClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "EnemySpawnPoint")
	int32 EnemyLevel = 1;

	UPROPERTY(EditAnywhere)
	float SpawnPeriod = 10.f;

private:
	UPROPERTY()
	FTimerHandle SpawnTimerHandle;
};
