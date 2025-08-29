#include "SummonComponent.h"

#include "AbilitySystemComponent.h"
#include "Aura/AbilitySystem/AuraAttributeSet.h"
#include "Aura/Character/AuraCharacterBase.h"

void USummonComponent::BeginPlay()
{
	Super::BeginPlay();

	if (ICombatInterface* Owner = Cast<ICombatInterface>(GetOwner()))
	{
		Owner->GetOnDeathDelegate().AddDynamic(this, &ThisClass::OnOwnerDied);
	}
}

void USummonComponent::OnOwnerDied(AActor* DeathActor)
{
	for (auto Minion : CurrentMinions)
	{
		if (ICombatInterface* CombatInterface = Cast<ICombatInterface>(Minion.Get()))
		{
			CombatInterface->Die(FVector::ZeroVector);
		}
		
		Minion->OnDestroyed.RemoveDynamic(this, &ThisClass::RemoveMinion);
		if (CurrentMinions.Contains(Minion))
		{
			CurrentMinions.Remove(Minion);
		}
	}
}

bool USummonComponent::CanSummon() const
{
	return MaxSummonMinionCount > CurrentMinions.Num();
}

void USummonComponent::AddMinion(AActor* InMinion)
{
	if (!InMinion)
	{
		return;
	}
	
	if (!CurrentMinions.Contains(InMinion))
	{
		CurrentMinions.Add(InMinion);
		InMinion->OnDestroyed.AddDynamic(this, &ThisClass::RemoveMinion);
	}
}

void USummonComponent::RemoveMinion(AActor* DestroyedActor)
{
	if (!DestroyedActor)
	{
		return;
	}
	
	if (CurrentMinions.Contains(DestroyedActor))
	{
		CurrentMinions.Remove(DestroyedActor);
	}
}
