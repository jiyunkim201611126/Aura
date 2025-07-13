#include "SummonComponent.h"

void USummonComponent::AddMinion(AActor* InMinion)
{
	if (!CurrentMinion.Contains(InMinion))
	{
		CurrentMinion.Add(InMinion);
		--SpawnableSummonMinionCount;
		InMinion->OnDestroyed.AddDynamic(this, &ThisClass::RemoveMinion);
	}
}

void USummonComponent::RemoveMinion(AActor* DestroyedActor)
{
	if (CurrentMinion.Contains(DestroyedActor))
	{
		CurrentMinion.Remove(DestroyedActor);
	}
}
