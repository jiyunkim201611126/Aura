#include "AuraGameplayAbility.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Aura/AbilitySystem/AuraAttributeSet.h"
#include "Aura/Interaction/CombatInterface.h"
#include "Aura/Interaction/EnemyInterface.h"
#include "Aura/Manager/AuraGameplayTags.h"
#include "Aura/Manager/AuraTextManager.h"
#include "UsableTypes/AbilityUsableType.h"

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

TArray<FGameplayEffectSpecHandle> UAuraGameplayAbility::MakeDebuffSpecHandle()
{
	if (!DebuffEffectContextHandle.Get())
	{
		DebuffEffectContextHandle = GetAbilitySystemComponentFromActorInfo()->MakeEffectContext();
	}
	
	const FAuraGameplayTags& GameplayTags = FAuraGameplayTags::Get();
	TArray<FGameplayEffectSpecHandle> DebuffSpecs;
	for (auto& Data : DebuffData)
	{
		FGameplayEffectSpecHandle DebuffSpecHandle = GetAbilitySystemComponentFromActorInfo()->MakeOutgoingSpec(DebuffEffectClass, 1.f, DebuffEffectContextHandle);
		UAbilitySystemBlueprintLibrary::AddGrantedTag(DebuffSpecHandle, Data.DebuffType);
		UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(DebuffSpecHandle, GameplayTags.Debuff_Chance, Data.DebuffChance);
		UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(DebuffSpecHandle, GameplayTags.Debuff_Damage, Data.DebuffDamage);
		UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(DebuffSpecHandle, GameplayTags.Debuff_Duration, Data.DebuffDuration);
		UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(DebuffSpecHandle, GameplayTags.Debuff_Frequency, Data.DebuffFrequency);
		DebuffSpecs.Add(DebuffSpecHandle);
	}

	return DebuffSpecs;
}

void UAuraGameplayAbility::CauseDebuff(AActor* TargetActor, const TArray<FGameplayEffectSpecHandle>& DebuffSpecs) const
{
	for (auto& DebuffSpecHandle : DebuffSpecs)
	{
		GetAbilitySystemComponentFromActorInfo()->ApplyGameplayEffectSpecToTarget(*DebuffSpecHandle.Data.Get(), UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor));
	}
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

FTaggedMontage UAuraGameplayAbility::GetRandomMontage()
{
	const int RandomIndex = FMath::RandRange(0, TaggedMontages.Num() - 1);
	FTaggedMontage TaggedMontage = TaggedMontages[RandomIndex];

	return TaggedMontage;
}

void UAuraGameplayAbility::RegisterAbilityToUsableTypeManagers(UAuraAbilitySystemComponent* ASC)
{
	for (const auto AbilityUsableType : UsableTypes)
	{
		AbilityUsableType->OnEquipAbility(this, ASC);
	}
}

void UAuraGameplayAbility::UnregisterAbilityFromUsableTypeManagers(UAuraAbilitySystemComponent* ASC)
{
	for (const auto AbilityUsableType : UsableTypes)
	{
		AbilityUsableType->OnUnequipAbility(this, ASC);
	}
}

bool UAuraGameplayAbility::CheckCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, FGameplayTagContainer* OptionalRelevantTags) const
{	
	for (const auto AbilityUsableType : UsableTypes)
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
	for (const auto AbilityUsableType : UsableTypes)
	{
		AbilityUsableType->ApplyCost(this);
	}
	
	Super::ApplyCost(Handle, ActorInfo, ActivationInfo);
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
