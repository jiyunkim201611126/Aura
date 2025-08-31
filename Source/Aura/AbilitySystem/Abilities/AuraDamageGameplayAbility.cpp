#include "AuraDamageGameplayAbility.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Aura/AbilitySystem/AuraAbilitySystemLibrary.h"
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
		// EffectContext를 생성 및 할당합니다.
		// MakeEffectContext 함수는 자동으로 OwnerActor를 Instigator로, AvatarActor를 EffectCauser로 할당합니다.
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
	if (DamageEffectContextHandle.IsValid())
	{
		// 대상을 관련 액터에 추가합니다.
		TArray<TWeakObjectPtr<AActor>> TargetActors;
		TargetActors.Add(TargetActor);
		DamageEffectContextHandle.AddActors(TargetActors);

		// 여기선 사망 여부를 알 수 없으므로, DeathImpulse를 일단 세팅합니다.
		UAuraAbilitySystemLibrary::SetDeathImpulse(DamageEffectContextHandle, GetAvatarActorFromActorInfo()->GetActorForwardVector() * DeathImpulseMagnitude);

		// 넉백은 확률 계산 후 성공 시 세팅합니다.
		const bool bKnockback = FMath::FRandRange(0.f, 100.f) < KnockbackChance;
		if (bKnockback)
		{
			UAuraAbilitySystemLibrary::SetKnockbackForce(DamageEffectContextHandle, GetAvatarActorFromActorInfo()->GetActorForwardVector() * KnockbackForceMagnitude);
		}
	}

	for (auto& Spec : DamageSpecs)
	{
		if (TargetActor->Implements<UCombatInterface>() && ICombatInterface::Execute_IsDead(TargetActor))
		{
			return;
		}
		
		GetAbilitySystemComponentFromActorInfo()->ApplyGameplayEffectSpecToTarget(*Spec.Data.Get(), UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor));
	}
	
	CauseDebuff(TargetActor, MakeDebuffSpecHandle());
}

FText UAuraDamageGameplayAbility::GetDamageTexts(int32 InLevel)
{
	TArray<FText> FormattedTexts;

	for (const auto& Damage : DamageTypes)
	{
		const FGameplayTag& DamageTag = Damage.Key;
		const float DamageValue = Damage.Value.GetValueAtLevel(InLevel);

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
