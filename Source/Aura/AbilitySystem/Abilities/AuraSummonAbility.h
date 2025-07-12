#pragma once

#include "CoreMinimal.h"
#include "AuraGameplayAbility.h"
#include "AuraSummonAbility.generated.h"

UCLASS()
class AURA_API UAuraSummonAbility : public UAuraGameplayAbility
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Summoning")
	int32 NumMinions = 5;

	UPROPERTY(EditAnywhere, Category = "Summoning")
	TArray<TSubclassOf<APawn>> MinionClasses;

	UPROPERTY(EditAnywhere, Category = "Summoning")
	float MinSpawnDistance = 150.f;

	UPROPERTY(EditAnywhere, Category = "Summoning")
	float MaxSpawnDistance = 300.f;
	
	UPROPERTY(EditAnywhere, Category = "Summoning")
	float SpawnSpread = 90.f;

public:
	UFUNCTION(BlueprintCallable)
	TArray<FVector> GetSpawnLocations();

	UFUNCTION(BlueprintPure, Category = "Summoning")
	TSubclassOf<APawn> GetRandomMinionClass() const;
};
