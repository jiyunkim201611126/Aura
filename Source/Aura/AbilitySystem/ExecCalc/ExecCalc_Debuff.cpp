#include "ExecCalc_Debuff.h"

#include "Aura/AbilitySystem/AuraAbilitySystemLibrary.h"
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

UExecCalc_Debuff::UExecCalc_Debuff()
{
	RelevantAttributesToCapture.Add(DebuffStatics().TargetFireResistance);
	RelevantAttributesToCapture.Add(DebuffStatics().TargetLightningResistance);
	RelevantAttributesToCapture.Add(DebuffStatics().TargetArcaneResistance);
}

void UExecCalc_Debuff::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	const FAuraGameplayTags& GameplayTags = FAuraGameplayTags::Get();
	
	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();
	
	// 부여 성공 시 디버프 관련 계산을 시작합니다.
	FGameplayTagContainer GrantedTags;
	Spec.GetAllGrantedTags(GrantedTags);
	const float Damage = Spec.GetSetByCallerMagnitude(GameplayTags.Debuff_Damage, false);
	
	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();
	FAggregatorEvaluateParameters EvaluationParameters;
	EvaluationParameters.SourceTags = SourceTags;
	EvaluationParameters.TargetTags = TargetTags;

	const bool bIsBurn = GrantedTags.HasTagExact(GameplayTags.Debuff_Type_Burn);
	const bool bIsStun = GrantedTags.HasTagExact(GameplayTags.Debuff_Type_Stun);
	const bool bIsConfuse = GrantedTags.HasTagExact(GameplayTags.Debuff_Type_Confuse);

	// 타입에 해당하는 관련 저항력 Attribute를 가져옵니다.
	float ResistanceTypeValue = 0.f;
	if (bIsBurn)
	{
		const FGameplayEffectAttributeCaptureDefinition Resistance = AuraDebuffStatics().TagsToCaptureResistanceDefs[GameplayTags.Attributes_Resistance_Fire];
		ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(Resistance, EvaluationParameters, ResistanceTypeValue);
		
		FGameplayEffectContextHandle EffectContextHandle = Spec.GetContext();
		UAuraAbilitySystemLibrary::SetDamageDataContext(EffectContextHandle, EDamageTypeContext::Fire, false, false);
		const FGameplayModifierEvaluatedData EvaluatedData(UAuraAttributeSet::GetIncomingDamageAttribute(), EGameplayModOp::Additive, Damage);
		OutExecutionOutput.AddOutputModifier(EvaluatedData);
	}
	else if (bIsStun)
	{
		const FGameplayEffectAttributeCaptureDefinition Resistance = AuraDebuffStatics().TagsToCaptureResistanceDefs[GameplayTags.Attributes_Resistance_Lightning];
		ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(Resistance, EvaluationParameters, ResistanceTypeValue);
	}
	else if (bIsConfuse)
	{
		// 미구현 디버프입니다.
		const FGameplayEffectAttributeCaptureDefinition Resistance = AuraDebuffStatics().TagsToCaptureResistanceDefs[GameplayTags.Attributes_Resistance_Arcane];
		ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(Resistance, EvaluationParameters, ResistanceTypeValue);
	}
}
