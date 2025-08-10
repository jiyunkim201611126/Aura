#include "AuraGameplayAbility.h"

#include "Aura/Interaction/CombatInterface.h"
#include "Aura/Interaction/EnemyInterface.h"
#include "UsableTypes/AbilityUsableType.h"

void UAuraGameplayAbility::UpdateFacingToCombatTarget() const
{
	UObject* SourceActor = GetAvatarActorFromActorInfo();
	const AActor* TargetActor = IEnemyInterface::Execute_GetCombatTarget(SourceActor);
	const FVector TargetLocation = TargetActor->GetActorLocation();
	ICombatInterface::Execute_UpdateFacingTarget(SourceActor, TargetLocation);
}

FTaggedMontage UAuraGameplayAbility::GetRandomMontage()
{
	const int RandomIndex = FMath::RandRange(0, TaggedMontages.Num() - 1);
	FTaggedMontage TaggedMontage = TaggedMontages[RandomIndex];

	return TaggedMontage;
}

void UAuraGameplayAbility::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnGiveAbility(ActorInfo, Spec);

	for (auto AbilityUsableType : UsableTypes)
	{
		AbilityUsableType->OnGivenAbility(ActorInfo, Spec);
	}
}

bool UAuraGameplayAbility::CheckCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, FGameplayTagContainer* OptionalRelevantTags) const
{	
	for (auto AbilityUsableType : UsableTypes)
	{
		if (!AbilityUsableType->CheckCost(this))
		{
			return false;
		}
	}
	
	return Super::CheckCost(Handle, ActorInfo, OptionalRelevantTags);
}

void UAuraGameplayAbility::ApplyCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const
{
	for (auto AbilityUsableType : UsableTypes)
	{
		AbilityUsableType->ApplyCost(this);
	}
	
	Super::ApplyCost(Handle, ActorInfo, ActivationInfo);
}

void UAuraGameplayAbility::OnRemoveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	for (auto AbilityUsableType : UsableTypes)
	{
		AbilityUsableType->OnRemoveAbility(this);
	}
	
	Super::OnRemoveAbility(ActorInfo, Spec);
}
