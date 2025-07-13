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
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Summoning")
	int32 SpawnableSummonMinionCount = 3;

	UFUNCTION(BlueprintCallable, Category = "Summoning")
	bool CanSummon() const { return SpawnableSummonMinionCount > 0; }

	UFUNCTION(BlueprintCallable, Category = "Summoning")
	void AddMinion(AActor* InMinion);

	UFUNCTION()
	void RemoveMinion(AActor* DestroyedActor);
};
