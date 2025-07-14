#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SummonComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class AURA_API USummonComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Summoning")
	TArray<AActor*> CurrentMinion;

	// 현재 소환 가능한 하수인 수입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Summoning")
	int32 SpawnableSummonMinionCount = 3;

	// 최대 소환 가능한 하수인 수입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Summoning")
	int32 MaxSummonMinionCount = 3;

	// 하수인 사망 시점에 남은 하수인의 수가 해당 수치 이하인 경우, 소환 가능한 하수인 수를 초기화합니다.
	// 음수로 할당하면 초기화하지 않습니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Summoning")
	int32 ResetCountThreshold = 0;

	UFUNCTION(BlueprintCallable, Category = "Summoning")
	bool CanSummon() const { return SpawnableSummonMinionCount > 0 && MaxSummonMinionCount > CurrentMinion.Num(); }

	UFUNCTION(BlueprintCallable, Category = "Summoning")
	void AddMinion(AActor* InMinion);

	UFUNCTION()
	void RemoveMinion(AActor* DestroyedActor);
	
	UFUNCTION(BlueprintCallable, Category = "Summoning")
	void ResetSpawnableCount();
};
