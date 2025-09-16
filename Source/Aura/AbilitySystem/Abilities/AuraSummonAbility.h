#pragma once

#include "CoreMinimal.h"
#include "AuraGameplayAbility.h"
#include "AuraSummonAbility.generated.h"

UCLASS()
class AURA_API UAuraSummonAbility : public UAuraGameplayAbility
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "Summon")
	TSubclassOf<APawn> GetRandomMinionClass() const;
	
protected:
	UFUNCTION(BlueprintCallable)
	TArray<FVector_NetQuantize> GetSpawnLocations();

	UFUNCTION(BlueprintCallable)
	void SpawnNiagaras(const TArray<FVector_NetQuantize>& SpawnLocations);

	UFUNCTION(BlueprintCallable)
	void SpawnMinion(const FVector_NetQuantize& SpawnLocation);

protected:
	UPROPERTY(EditDefaultsOnly)
	FGameplayTag SpawnNiagaraTag;

private:
	// 한 번에 스폰하는 하수인 수
	UPROPERTY(EditAnywhere, Category = "Summon")
	int32 NumMinions = 1;

	UPROPERTY(EditAnywhere, Category = "Summon")
	TArray<TSubclassOf<APawn>> MinionClasses;

	UPROPERTY(EditAnywhere, Category = "Summon")
	float MinSpawnDistance = 150.f;

	UPROPERTY(EditAnywhere, Category = "Summon")
	float MaxSpawnDistance = 300.f;
	
	UPROPERTY(EditAnywhere, Category = "Summon")
	float SpawnSpread = 90.f;

	
protected:
	//~ Begin GameplayAbility Interface
	virtual void OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
	virtual bool CheckCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	//~ End GameplayAbility Interface
};
