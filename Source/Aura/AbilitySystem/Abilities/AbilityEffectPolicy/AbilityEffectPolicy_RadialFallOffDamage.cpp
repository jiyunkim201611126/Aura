#include "AbilityEffectPolicy_RadialFallOffDamage.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Abilities/GameplayAbility.h"
#include "Aura/Manager/AuraTextManager.h"

void UAbilityEffectPolicy_RadialFallOffDamage::ApplyEffect(UGameplayAbility* OwningAbility, AActor* TargetActor, const FEffectPolicyContext& EffectPolicyContext)
{
	RadialDamageOrigin = EffectPolicyContext.OriginVector;
	CauseDamage(OwningAbility, TargetActor, MakeDamageSpecHandleWithRadial(OwningAbility, TargetActor));
}

TArray<FGameplayEffectSpecHandle> UAbilityEffectPolicy_RadialFallOffDamage::MakeDamageSpecHandleWithRadial(const UGameplayAbility* OwningAbility, const AActor* TargetActor)
{
	const UAbilitySystemComponent* ASC = OwningAbility->GetAbilitySystemComponentFromActorInfo();
	if (!ASC)
	{
		return TArray<FGameplayEffectSpecHandle>();
	}

	MakeEffectContextHandle(OwningAbility);

	// 데미지 원점으로부터 타겟까지 거리를 계산합니다.
	const float TargetDist = FVector::Dist(RadialDamageOrigin, TargetActor->GetActorLocation());

	// 데미지 감쇠율을 계산합니다.
	const float DamageRatio = CalculateFalloffRatio(TargetDist);
	
	TArray<FGameplayEffectSpecHandle> DamageSpecs;
	for (TPair<FGameplayTag, FScalableFloat>& Pair : DamageTypes)
	{
		const float ScaledDamage = Pair.Value.GetValueAtLevel(OwningAbility->GetAbilityLevel());
		
		FGameplayEffectSpecHandle DamageSpecHandle = ASC->MakeOutgoingSpec(EffectClass, 1.f, EffectContextHandle);
		
		// 감쇠율이 적용된 데미지를 주입합니다.
		UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(DamageSpecHandle, Pair.Key, ScaledDamage * DamageRatio);

		DamageSpecs.Add(DamageSpecHandle);
	}
	
	return DamageSpecs;
}

FText UAbilityEffectPolicy_RadialFallOffDamage::GetDamageTexts(int32 InLevel)
{
	TArray<FText> FormattedTexts;

	for (const auto& Damage : DamageTypes)
	{
		const FGameplayTag& DamageTag = Damage.Key;
		const float DamageValue = Damage.Value.GetValueAtLevel(InLevel);

		// 태그 네임을 String으로 바꿔 그대로 String Table의 Key로 사용합니다.
		// ToString으로 변환될 때 언더바(_)가 아닌 마침표(.)으로 변환되므로, String Table에서도 마침표로 Key를 작성합니다. (예시: Damage.Fire)
		FString TextKey = DamageTag.GetTagName().ToString();
		// 최대 소수점 1자리까지 표기합니다.
		FNumberFormattingOptions FormattingOptions;
		FormattingOptions.MinimumFractionalDigits = 0;
		FormattingOptions.MaximumFractionalDigits = 1;
		FText DamageTypeText = FAuraTextManager::GetText(EStringTableTextType::UI, TextKey, FText::AsNumber(DamageValue, &FormattingOptions));
		
		FormattedTexts.Add(DamageTypeText);
	}

	// FallOff에 관련된 정보를 추가합니다.
	const FText InnerRadiusText = FText::FromString(FString::Printf(TEXT("<Yellow>Inner Radius: %.0f</>"), RadialDamageInnerRadius));
	const FText OuterRadiusText = FText::FromString(FString::Printf(TEXT("<Yellow>Outer Radius: %.0f</>"), RadialDamageOuterRadius));
	const FText MinDamageRatioText = FText::FromString(FString::Printf(TEXT("<Yellow>Min Damage Ratio: %.0f%%</>"), MinDamageRatio * 100.f));

	FormattedTexts.Add(InnerRadiusText);
	FormattedTexts.Add(OuterRadiusText);
	FormattedTexts.Add(MinDamageRatioText);

	// 각 원소 사이마다 \n을 삽입해 반환합니다.
	return FText::Join(FText::FromString(TEXT("\n")), FormattedTexts);
}

float UAbilityEffectPolicy_RadialFallOffDamage::CalculateFalloffRatio(const float TargetDist) const
{
	if (TargetDist < RadialDamageInnerRadius)
	{
		return 1.f;
	}
	
	const float FalloffRatio = 1.f - (TargetDist - RadialDamageInnerRadius) / (RadialDamageOuterRadius - RadialDamageInnerRadius);

	return FMath::Clamp(FalloffRatio, MinDamageRatio, 1.f);
}
