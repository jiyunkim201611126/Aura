#include "SummonComponent.h"

void USummonComponent::AddMinion(AActor* InMinion)
{
	if (!InMinion)
	{
		return;
	}
	
	if (!CurrentMinion.Contains(InMinion))
	{
		CurrentMinion.Add(InMinion);
		--SpawnableSummonMinionCount;
		InMinion->OnDestroyed.AddDynamic(this, &ThisClass::RemoveMinion);
	}
}

void USummonComponent::RemoveMinion(AActor* DestroyedActor)
{
	if (!DestroyedActor)
	{
		return;
	}
	
	if (CurrentMinion.Contains(DestroyedActor))
	{
		CurrentMinion.Remove(DestroyedActor);
	}

	if (ResetCountThreshold >= CurrentMinion.Num())
	{
		ResetSpawnableCount();
	}
}

void USummonComponent::ResetSpawnableCount()
{
	SpawnableSummonMinionCount = MaxSummonMinionCount;
}
