#include "AuraDamageGameplayAbility.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Aura/Interaction/CombatInterface.h"
#include "Aura/Manager/AuraTextManager.h"

void UAuraDamageGameplayAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	DamageEffectContextHandle.Clear();
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

TArray<FGameplayEffectSpecHandle> UAuraDamageGameplayAbility::MakeDamageSpecHandle()
{
	if (!DamageEffectContextHandle.Get())
	{
		DamageEffectContextHandle = GetAbilitySystemComponentFromActorInfo()->MakeEffectContext();
	}
	
	TArray<FGameplayEffectSpecHandle> DamageSpecs;
	for (TPair<FGameplayTag, FScalableFloat>& Pair : DamageTypes)
	{
		const float AbilityLevel = GetAbilityLevel();
		const float ScaledDamage = Pair.Value.GetValueAtLevel(AbilityLevel);
		
		// 할당받은 DamageEffectClass를 기반으로 Projectile이 가질 GameplayEffectSpecHandle을 생성합니다.
		FGameplayEffectSpecHandle DamageSpecHandle = GetAbilitySystemComponentFromActorInfo()->MakeOutgoingSpec(DamageEffectClass, 1.f, DamageEffectContextHandle);
		
		// Spec 안에 SetByCallerMagnitudes라는 이름의 TMap이 있으며, 거기에 Tag를 키, Damage를 밸류로 값을 추가하는 함수입니다.
		// 이 값은 GetSetByCallerMagnitude로 꺼내올 수 있습니다.
		UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(DamageSpecHandle, Pair.Key, ScaledDamage);

		DamageSpecs.Add(DamageSpecHandle);
	}
	
	return DamageSpecs;
}

void UAuraDamageGameplayAbility::CauseDamage(AActor* TargetActor, TArray<FGameplayEffectSpecHandle> DamageSpecs)
{
	// 관련 Actor에 추가
	if (DamageEffectContextHandle.IsValid())
	{
		TArray<TWeakObjectPtr<AActor>> TargetActors;
		TargetActors.Add(TargetActor);
		DamageEffectContextHandle.AddActors(TargetActors);
	}

	for (auto& Spec : DamageSpecs)
	{
		GetAbilitySystemComponentFromActorInfo()->ApplyGameplayEffectSpecToTarget(*Spec.Data.Get(), UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor));
	}
	
	CauseDebuff(TargetActor, MakeDebuffSpecContexts());
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
	if (DamageEffectContextHandle.IsValid())
	{
		return DamageEffectContextHandle;
	}

	return FGameplayEffectContextHandle();
}
