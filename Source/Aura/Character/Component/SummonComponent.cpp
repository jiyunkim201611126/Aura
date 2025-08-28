#include "SummonComponent.h"

#include "AbilitySystemComponent.h"
#include "Aura/AbilitySystem/AuraAttributeSet.h"
#include "Aura/Character/AuraCharacterBase.h"

void USummonComponent::BeginPlay()
{
	Super::BeginPlay();

	UAbilitySystemComponent* OwnerASC = GetOwner()->FindComponentByClass<UAbilitySystemComponent>();
	if (OwnerASC)
	{
		OwnerASC->GetGameplayAttributeValueChangeDelegate(UAuraAttributeSet::GetHealthAttribute()).AddUObject(this, &ThisClass::CheckOwnerDie);
	}
}

void USummonComponent::CheckOwnerDie(const FOnAttributeChangeData& OnAttributeChangeData)
{
	float NewHealth = OnAttributeChangeData.NewValue;
	float OldHealth = OnAttributeChangeData.OldValue;

	if (NewHealth <= 0.f && OldHealth > 0.f)
	{
		for (auto Minion : CurrentMinions)
		{
			if (ICombatInterface* CombatInterface = Cast<ICombatInterface>(Minion.Get()))
			{
				CombatInterface->Die(FVector::ZeroVector);
			}
		}
	}
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
	
	if (CurrentMinions.Contains(DestroyedActor))
	{
		CurrentMinions.Remove(DestroyedActor);
	}

	if (ResetCountThreshold >= CurrentMinions.Num())
	{
		ResetSpawnableCount();
	}
}

void USummonComponent::ResetSpawnableCount()
{
	SpawnableSummonMinionCount = MaxSummonMinionCount;
}
