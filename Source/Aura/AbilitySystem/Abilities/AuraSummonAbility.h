#pragma once

#include "CoreMinimal.h"
#include "AuraGameplayAbility.h"
#include "AuraSummonAbility.generated.h"

UCLASS()
class AURA_API UAuraSummonAbility : public UAuraGameplayAbility
{
	GENERATED_BODY()

private:
	// 한 번에 스폰하는 하수인 수
	UPROPERTY(EditAnywhere, Category = "Summoning")
	int32 NumMinions = 1;

	UPROPERTY(EditAnywhere, Category = "Summoning")
	TArray<TSubclassOf<APawn>> MinionClasses;

	UPROPERTY(EditAnywhere, Category = "Summoning")
	float MinSpawnDistance = 150.f;

	UPROPERTY(EditAnywhere, Category = "Summoning")
	float MaxSpawnDistance = 300.f;
	
	UPROPERTY(EditAnywhere, Category = "Summoning")
	float SpawnSpread = 90.f;

protected:
	virtual void OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;

	virtual bool CheckCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

public:
	UFUNCTION(BlueprintCallable)
	TArray<FVector> GetSpawnLocations();

	UFUNCTION(BlueprintPure, Category = "Summoning")
	TSubclassOf<APawn> GetRandomMinionClass() const;
};
