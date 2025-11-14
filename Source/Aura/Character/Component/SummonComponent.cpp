#include "SummonComponent.h"
#include "Aura/Character/AuraCharacterBase.h"

void USummonComponent::BeginPlay()
{
	Super::BeginPlay();
	
	if (!GetOwner()->HasAuthority())
	{
		return;
	}

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
	}
}

bool USummonComponent::CanSummon() const
{
	return MaxSummonMinionCount > CurrentMinions.Num();
}

void USummonComponent::AddMinion(AActor* InMinion)
{
	if (!InMinion || !GetOwner()->HasAuthority())
	{
		return;
	}
	
	if (!CurrentMinions.Contains(InMinion))
	{
		CurrentMinions.Add(InMinion);
		InMinion->OnEndPlay.AddDynamic(this, &ThisClass::RemoveMinion);
	}
}

void USummonComponent::RemoveMinion(AActor* InMinion, EEndPlayReason::Type EndPlayReason)
{
	if (!InMinion || !GetOwner()->HasAuthority())
	{
		return;
	}
	
	if (CurrentMinions.Contains(InMinion))
	{
		CurrentMinions.Remove(InMinion);
	}
}
