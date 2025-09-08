#include "AuraGameplayAbility.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilityEffectPolicy/AbilityEffectPolicy_Damage.h"
#include "AbilityEffectPolicy/AbilityEffectPolicy_Debuff.h"
#include "Aura/AbilitySystem/AuraAttributeSet.h"
#include "Aura/Interaction/CombatInterface.h"
#include "Aura/Interaction/EnemyInterface.h"
#include "Aura/Manager/AuraTextManager.h"
#include "AbilityAdditionalCost/AbilityAdditionalCost.h"

void UAuraGameplayAbility::UpdateFacingToCombatTarget() const
{
	UObject* SourceActor = GetAvatarActorFromActorInfo();
	const AActor* TargetActor = IEnemyInterface::Execute_GetCombatTarget(SourceActor);
	const FVector TargetLocation = TargetActor->GetActorLocation();
	ICombatInterface::Execute_UpdateFacingTarget(SourceActor, TargetLocation);
}

FText UAuraGameplayAbility::GetDescription_Implementation(int32 Level)
{
	return FAuraTextManager::GetText(EStringTableTextType::UI, DescriptionKey);
}

FText UAuraGameplayAbility::GetLockedDescription(int32 Level)
{
	return FText::Format(FAuraTextManager::GetText(EStringTableTextType::UI, TEXT("Abilities_Description_Locked")), Level);
}

void UAuraGameplayAbility::ApplyAllEffect(AActor* TargetActor)
{
	for (const auto EffectPolicy : EffectPolicies)
	{
		EffectPolicy->ApplyAllEffect(this, TargetActor);
	}
}

FGameplayEffectContextHandle UAuraGameplayAbility::GetDamageContextHandle() const
{
	UAbilityEffectPolicy_Damage* DamageEffectPolicy = GetEffectPoliciesOfClass<UAbilityEffectPolicy_Damage>(EffectPolicies);
	return DamageEffectPolicy->DamageEffectContextHandle;
}

FGameplayEffectContextHandle UAuraGameplayAbility::GetDebuffContextHandle() const
{
	UAbilityEffectPolicy_Debuff* DebuffEffectPolicy = GetEffectPoliciesOfClass<UAbilityEffectPolicy_Debuff>(EffectPolicies);
	return DebuffEffectPolicy->DebuffEffectContextHandle;
}

float UAuraGameplayAbility::GetManaCost(int32 InLevel) const
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

float UAuraGameplayAbility::GetCooldown(int32 InLevel) const
{
	float Cooldown = 0.f;
	if (const UGameplayEffect* CooldownEffect = GetCooldownGameplayEffect())
	{
		CooldownEffect->DurationMagnitude.GetStaticMagnitudeIfPossible(InLevel, Cooldown);
	}
	return Cooldown;
}

FText UAuraGameplayAbility::GetDamageTexts(int32 InLevel)
{
	for (auto EffectPolicy : EffectPolicies)
	{
		if (UAbilityEffectPolicy_Damage* DamageEffectPolicy = Cast<UAbilityEffectPolicy_Damage>(EffectPolicy))
		{
			return DamageEffectPolicy->GetDamageTexts(InLevel);
		}
	}
	return FText();
}

FTaggedMontage UAuraGameplayAbility::GetRandomMontage()
{
	const int RandomIndex = FMath::RandRange(0, TaggedMontages.Num() - 1);
	FTaggedMontage TaggedMontage = TaggedMontages[RandomIndex];

	return TaggedMontage;
}

void UAuraGameplayAbility::RegisterAbilityToUsableTypeManagers(UAuraAbilitySystemComponent* ASC)
{
	for (const auto AdditionalCost : AdditionalCosts)
	{
		AdditionalCost->OnEquipAbility(this, ASC);
	}
}

void UAuraGameplayAbility::UnregisterAbilityFromUsableTypeManagers(UAuraAbilitySystemComponent* ASC)
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
	for (const auto AdditionalCost : AdditionalCosts)
	{
		AdditionalCost->ActivateAbility(this);
	}
	
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UAuraGameplayAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	for (const auto AdditionalCost : AdditionalCosts)
	{
		AdditionalCost->EndAbility(this);
	}

	for (const auto EffectPolicy : EffectPolicies)
	{
		EffectPolicy->EndAbility();
	}
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

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
		
		SetAssetTags(NewAbilityTags);
	}
}
