#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectTypes.h"
#include "Components/ActorComponent.h"
#include "SummonComponent.generated.h"

/**
 * 단순히 '충전식 스킬'을 구현하려면 굳이 Component까진 필요없으나,
 * '하수인 소환 스킬'의 경우 모든 하수인 통합 최대 소환 가능 수가 있어야 한다고 생각해 따로 Component를 선언해 관리합니다.
 */

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class AURA_API USummonComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	void CheckOwnerDie(const FOnAttributeChangeData& OnAttributeChangeData);
	
	UFUNCTION(BlueprintCallable, Category = "Summoning")
	bool CanSummon() const { return SpawnableSummonMinionCount > 0 && MaxSummonMinionCount > CurrentMinions.Num(); }

	UFUNCTION(BlueprintCallable, Category = "Summoning")
	void AddMinion(AActor* InMinion);

	UFUNCTION()
	void RemoveMinion(AActor* DestroyedActor);
	
	void ResetSpawnableCount();

protected:
	// ~UActorComponent Interface
	virtual void BeginPlay() override;
	// ~End of UActorComponent Interface
	
public:
	UPROPERTY()
	TArray<TWeakObjectPtr<AActor>> CurrentMinions;

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
};
