#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectTypes.h"
#include "Components/ActorComponent.h"
#include "SummonComponent.generated.h"

/**
 * '하수인 소환 스킬'의 경우 모든 하수인 통합 최대 소환 가능 수가 있어야 한다고 생각해 따로 Component를 선언해 관리합니다.
 */

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class AURA_API USummonComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFUNCTION()
	void OnOwnerDied(AActor* DeathActor);
	
	bool CanSummon() const;

	UFUNCTION(BlueprintCallable, Category = "Summoning")
	void AddMinion(AActor* InMinion);

	UFUNCTION()
	void RemoveMinion(AActor* DestroyedActor);

protected:
	// ~UActorComponent Interface
	virtual void BeginPlay() override;
	// ~End of UActorComponent Interface
	
public:
	UPROPERTY()
	TArray<TWeakObjectPtr<AActor>> CurrentMinions;

	// 최대 소환 가능한 하수인 수입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Summoning")
	int32 MaxSummonMinionCount = 3;
};
