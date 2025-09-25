#include "AuraGameplayAbility.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilityEffectPolicy/AbilityEffectPolicy_Damage.h"
#include "Aura/AbilitySystem/AuraAttributeSet.h"
#include "Aura/Interaction/CombatInterface.h"
#include "Aura/Interaction/EnemyInterface.h"
#include "Aura/Manager/AuraTextManager.h"
#include "AbilityAdditionalCost/AbilityAdditionalCost.h"

AController* UAuraGameplayAbility::GetController() const
{
	return Cast<APawn>(GetAvatarActorFromActorInfo())->GetController();
}

void UAuraGameplayAbility::UpdateFacingToCombatTarget() const
{
	UObject* SourceActor = GetAvatarActorFromActorInfo();
	const AActor* TargetActor = IEnemyInterface::Execute_GetCombatTarget(SourceActor);
	const FVector TargetLocation = TargetActor->GetActorLocation();
	ICombatInterface::Execute_UpdateFacingTarget(SourceActor, TargetLocation);
}

void UAuraGameplayAbility::ApplyAllEffect(AActor* TargetActor)
{
	for (const auto EffectPolicy : EffectPolicies)
	{
		if (EffectPolicy && TargetActor)
		{
			EffectPolicy->ApplyEffect(this, TargetActor);
		}
	}
}

FGameplayEffectContextHandle UAuraGameplayAbility::GetContextHandle(const TSubclassOf<UAbilityEffectPolicy> PolicyClass) const
{
	for (const auto EffectPolicy : EffectPolicies)
	{
		if (EffectPolicy && EffectPolicy->GetClass() == PolicyClass)
		{
			return EffectPolicy->GetEffectContextHandle();
		}
	}
	return FGameplayEffectContextHandle();
}

FText UAuraGameplayAbility::GetDescription_Implementation(const int32 Level)
{
	return FAuraTextManager::GetText(EStringTableTextType::UI, DescriptionKey);
}

FText UAuraGameplayAbility::GetLockedDescription(const int32 Level)
{
	return FText::Format(FAuraTextManager::GetText(EStringTableTextType::UI, TEXT("Abilities.Description.Locked")), Level);
}

float UAuraGameplayAbility::GetManaCost(const int32 InLevel) const
{
	float ManaCost = 0.f;
	if (const UGameplayEffect* CostEffect = GetCostGameplayEffect())
	{
		for (FGameplayModifierInfo Mod : CostEffect->Modifiers)
		{
			if (Mod.Attribute == UAuraAttributeSet::GetManaAttribute())
			{
				Mod.ModifierMagnitude.GetStaticMagnitudeIfPossible(InLevel, ManaCost);
				break;
			}
		}
	}
	return ManaCost;
}

float UAuraGameplayAbility::GetCooldown(const int32 InLevel) const
{
	float Cooldown = 0.f;
	if (const UGameplayEffect* CooldownEffect = GetCooldownGameplayEffect())
	{
		CooldownEffect->DurationMagnitude.GetStaticMagnitudeIfPossible(InLevel, Cooldown);
	}
	return Cooldown;
}

FText UAuraGameplayAbility::GetDamageTexts(const int32 InLevel) const
{
	if (UAbilityEffectPolicy_Damage* DamagePolicy = GetEffectPoliciesOfClass<UAbilityEffectPolicy_Damage>())
	{
		return DamagePolicy->GetDamageTexts(InLevel);
	}
	return FText();
}

FTaggedMontage UAuraGameplayAbility::GetRandomMontage()
{
	const int RandomIndex = FMath::RandRange(0, TaggedMontages.Num() - 1);
	FTaggedMontage TaggedMontage = TaggedMontages[RandomIndex];

	return TaggedMontage;
}

void UAuraGameplayAbility::RegisterAbilityToAdditionalCostManagers(UAuraAbilitySystemComponent* ASC)
{
	for (const auto AdditionalCost : AdditionalCosts)
	{
		AdditionalCost->OnEquipAbility(this, ASC);
	}
}

void UAuraGameplayAbility::UnregisterAbilityFromAdditionalCostManagers(UAuraAbilitySystemComponent* ASC)
{
	for (const auto AdditionalCost : AdditionalCosts)
	{
		AdditionalCost->OnUnequipAbility(this, ASC);
	}
}

bool UAuraGameplayAbility::CheckCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, FGameplayTagContainer* OptionalRelevantTags) const
{	
	for (const auto AdditionalCost : AdditionalCosts)
	{
		if (!AdditionalCost->CheckCost(this))
		{
			return false;
		}
	}
	
	return Super::CheckCost(Handle, ActorInfo, OptionalRelevantTags);
}

void UAuraGameplayAbility::ApplyCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const
{
	if (!ActorInfo->IsNetAuthority())
	{
		return;
	}
	
	for (const auto AdditionalCost : AdditionalCosts)
	{
		AdditionalCost->ApplyCost(this);
	}
	
	Super::ApplyCost(Handle, ActorInfo, ActivationInfo);
}

void UAuraGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UAuraGameplayAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	for (const auto EffectPolicy : EffectPolicies)
	{
		EffectPolicy->EndAbility();
	}
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}



#if WITH_EDITOR
void UAuraGameplayAbility::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.Property && PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(UAuraGameplayAbility, AbilityTag))
	{
		SyncAbilityTagToAssetTags();
	}
}

void UAuraGameplayAbility::SyncAbilityTagToAssetTags()
{
	// GetAssetTags를 통해 반환받는 변수인 AbilityTags는 GameplayAbility의 멤버 변수입니다.
	// 추후 AssetTags라는 이름으로 변경될 예정이며, 메타데이터 역할을 수행하기 때문에 런타임 중 변경되는 것을 금지하고 있습니다.
	// 따라서 해당 함수는 에디터에서만 호출됩니다.
	if (AbilityTag.IsValid() && !GetAssetTags().HasTag(AbilityTag))
	{
		FGameplayTagContainer NewAbilityTags;
		NewAbilityTags.AddTag(AbilityTag);
		for (auto AssetTag : GetAssetTags())
		{
			NewAbilityTags.AddTag(AssetTag);
		}
PRAGMA_DISABLE_DEPRECATION_WARNINGS
		// AbilityTag에 값을 할당하면 자동으로 AbilityTags(AssetTags)에도 함께 할당해주는 구문입니다.
		// 에디터에서만 호출되기 때문에 안전합니다.
		AbilityTags = NewAbilityTags;
PRAGMA_DISABLE_DEPRECATION_WARNINGS
	}
}
#endif
