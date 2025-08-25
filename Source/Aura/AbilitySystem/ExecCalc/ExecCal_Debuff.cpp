#include "ExecCal_Debuff.h"

#include "Aura/AbilitySystem/AuraAttributeSet.h"
#include "Aura/Manager/AuraGameplayTags.h"

struct AuraDebuffStatics
{
	FGameplayEffectAttributeCaptureDefinition TargetFireResistance;
	FGameplayEffectAttributeCaptureDefinition TargetLightningResistance;
	FGameplayEffectAttributeCaptureDefinition TargetArcaneResistance;
	
	TMap<FGameplayTag, FGameplayEffectAttributeCaptureDefinition> TagsToCaptureResistanceDefs;

	AuraDebuffStatics() :
	TargetFireResistance(UAuraAttributeSet::GetFireResistanceAttribute(), EGameplayEffectAttributeCaptureSource::Target, false),
	TargetLightningResistance(UAuraAttributeSet::GetLightningResistanceAttribute(), EGameplayEffectAttributeCaptureSource::Target, false),
	TargetArcaneResistance(UAuraAttributeSet::GetArcaneResistanceAttribute(), EGameplayEffectAttributeCaptureSource::Target, false)
	{
		TagsToCaptureResistanceDefs.Add(FAuraGameplayTags::Get().Attributes_Resistance_Fire, TargetFireResistance);
		TagsToCaptureResistanceDefs.Add(FAuraGameplayTags::Get().Attributes_Resistance_Lightning, TargetLightningResistance);
		TagsToCaptureResistanceDefs.Add(FAuraGameplayTags::Get().Attributes_Resistance_Arcane, TargetArcaneResistance);
	}
};

static const AuraDebuffStatics& DebuffStatics()
{
	// 함수 내부에 static으로 선언된 변수로, 이 함수에서만 접근 가능한 변수이며 한 번 초기화되면 프로그램 종료 시까지 같은 인스턴스를 반환
	static AuraDebuffStatics DStatics;
	return DStatics;
}

UExecCal_Debuff::UExecCal_Debuff()
{
	RelevantAttributesToCapture.Add(DebuffStatics().TargetFireResistance);
	RelevantAttributesToCapture.Add(DebuffStatics().TargetLightningResistance);
	RelevantAttributesToCapture.Add(DebuffStatics().TargetArcaneResistance);
}

void UExecCal_Debuff::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	const FAuraGameplayTags& GameplayTags = FAuraGameplayTags::Get();
	
	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();

	const float Chance = Spec.GetSetByCallerMagnitude(GameplayTags.Debuff_Chance, false);
	const float Damage = Spec.GetSetByCallerMagnitude(GameplayTags.Debuff_Damage, false);
	const float Duration = Spec.GetSetByCallerMagnitude(GameplayTags.Debuff_Duration, false);
	const float Frequency = Spec.GetSetByCallerMagnitude(GameplayTags.Debuff_Frequency, false);

	FGameplayTagContainer GrantedTags;
	Spec.GetAllGrantedTags(GrantedTags);
	
	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();
	FAggregatorEvaluateParameters EvaluationParameters;
	EvaluationParameters.SourceTags = SourceTags;
	EvaluationParameters.TargetTags = TargetTags;

	const bool bIsBurn = GrantedTags.HasTagExact(GameplayTags.Debuff_Burn);
	const bool bIsStun = GrantedTags.HasTagExact(GameplayTags.Debuff_Stun);
	const bool bIsConfuse = GrantedTags.HasTagExact(GameplayTags.Debuff_Confuse);

	float ResistanceTypeValue = 0.f;
	if (bIsBurn)
	{
		const FGameplayEffectAttributeCaptureDefinition Resistance = AuraDebuffStatics().TagsToCaptureResistanceDefs[GameplayTags.Attributes_Resistance_Fire];
		ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(Resistance, EvaluationParameters, ResistanceTypeValue);
	}
	else if (bIsStun)
	{
		const FGameplayEffectAttributeCaptureDefinition Resistance = AuraDebuffStatics().TagsToCaptureResistanceDefs[GameplayTags.Attributes_Resistance_Lightning];
		ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(Resistance, EvaluationParameters, ResistanceTypeValue);
	}
	else if (bIsConfuse)
	{
		const FGameplayEffectAttributeCaptureDefinition Resistance = AuraDebuffStatics().TagsToCaptureResistanceDefs[GameplayTags.Attributes_Resistance_Arcane];
		ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(Resistance, EvaluationParameters, ResistanceTypeValue);
	}
}
