#include "AuraDamageGameplayAbility.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Aura/Interaction/CombatInterface.h"
#include "Aura/Manager/AuraTextManager.h"

void UAuraDamageGameplayAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	EffectContextHandle.Clear();
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UAuraDamageGameplayAbility::CauseDamage(AActor* TargetActor)
{
	// Context 생성 및 관련 Actor에 추가
	if (!EffectContextHandle.Get())
	{
		EffectContextHandle = GetAbilitySystemComponentFromActorInfo()->MakeEffectContext();
	}

	TargetActors.Add(TargetActor);
	
	FGameplayEffectSpecHandle DamageSpecHandle = MakeOutgoingGameplayEffectSpec(DamageEffectClass, 1.f);
	for (TTuple<FGameplayTag, FScalableFloat> Pair : DamageTypes)
	{
		const float ScaledDamage = Pair.Value.GetValueAtLevel(GetAbilityLevel());
		UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(DamageSpecHandle, Pair.Key, ScaledDamage);
	}
	GetAbilitySystemComponentFromActorInfo()->ApplyGameplayEffectSpecToTarget(*DamageSpecHandle.Data.Get(), UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor));
}

void UAuraDamageGameplayAbility::SetTargetActorsToContext()
{
	if (EffectContextHandle.IsValid())
	{
		EffectContextHandle.AddActors(TargetActors);
	}

	TargetActors.Empty();
}

FText UAuraDamageGameplayAbility::GetDamageTexts(int32 InLevel)
{
	TArray<FText> FormattedTexts;

	for (const auto& Pair : DamageTypes)
	{
		const FGameplayTag& DamageTag = Pair.Key;
		const float DamageValue = Pair.Value.GetValueAtLevel(InLevel);

		// 태그 네임을 String으로 바꿔 그대로 String Table의 Key로 사용합니다.
		// ToString으로 변환될 때 언더바(_)가 아닌 마침표(.)으로 변환되므로, String Table에서도 마침표로 Key를 작성합니다.. (예시: Damage.Fire)
		FString TextKey = DamageTag.GetTagName().ToString();
		// 최대 소수점 1자리까지 표기합니다.
		FNumberFormattingOptions FormattingOptions;
		FormattingOptions.MinimumFractionalDigits = 0;
		FormattingOptions.MaximumFractionalDigits = 1;
		FText DamageTypeText = FAuraTextManager::GetText(EStringTableTextType::UI, TextKey, FText::AsNumber(DamageValue, &FormattingOptions));
		
		FormattedTexts.Add(DamageTypeText);
	}
	
	return FText::Join(FText::FromString(TEXT("\n")), FormattedTexts);
}

FGameplayEffectContextHandle UAuraDamageGameplayAbility::GetContext()
{
	if (EffectContextHandle.IsValid())
	{
		return EffectContextHandle;
	}

	return FGameplayEffectContextHandle();
}
