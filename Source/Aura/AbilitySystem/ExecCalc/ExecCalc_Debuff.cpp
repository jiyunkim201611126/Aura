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

	// 디버프 부여 확률을 계산합니다.
	const float Chance = Spec.GetSetByCallerMagnitude(GameplayTags.Debuff_Chance, false);
	const bool bDebuff = FMath::FRandRange(0.f, 100.f) < Chance;
	if (!bDebuff)
	{
		return;
	}

	// 부여 성공 시 디버프 관련 계산을 시작합니다.
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

	const bool bIsBurn = GrantedTags.HasTagExact(GameplayTags.Debuff_Type_Burn);
	const bool bIsStun = GrantedTags.HasTagExact(GameplayTags.Debuff_Type_Stun);
	const bool bIsConfuse = GrantedTags.HasTagExact(GameplayTags.Debuff_Type_Confuse);

	// 타입에 해당하는 태그(Context용)와 함께 관련 저항력 Attribute를 가져옵니다.
	FGameplayTag TypeTagForContext;
	float ResistanceTypeValue = 0.f;
	if (bIsBurn)
	{
		TypeTagForContext = GameplayTags.Debuff_Type_Burn;
		const FGameplayEffectAttributeCaptureDefinition Resistance = AuraDebuffStatics().TagsToCaptureResistanceDefs[GameplayTags.Attributes_Resistance_Fire];
		ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(Resistance, EvaluationParameters, ResistanceTypeValue);
	}
	else if (bIsStun)
	{
		TypeTagForContext = GameplayTags.Debuff_Type_Stun;
		const FGameplayEffectAttributeCaptureDefinition Resistance = AuraDebuffStatics().TagsToCaptureResistanceDefs[GameplayTags.Attributes_Resistance_Lightning];
		ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(Resistance, EvaluationParameters, ResistanceTypeValue);
	}
	else if (bIsConfuse)
	{
		TypeTagForContext = GameplayTags.Debuff_Type_Confuse;
		const FGameplayEffectAttributeCaptureDefinition Resistance = AuraDebuffStatics().TagsToCaptureResistanceDefs[GameplayTags.Attributes_Resistance_Arcane];
		ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(Resistance, EvaluationParameters, ResistanceTypeValue);
	}
	
	FGameplayEffectContextHandle EffectContextHandle = Spec.GetContext();
	FDebuffDataContext DebuffData = UAuraAbilitySystemLibrary::GetDebuffData(EffectContextHandle);
	DebuffData.DebuffType = UAuraAbilitySystemLibrary::ReplaceDebuffTypeToEnum(TypeTagForContext);
	DebuffData.DebuffDamage = Damage;
	DebuffData.DebuffDuration = Duration;
	DebuffData.DebuffFrequency = Frequency;
	UAuraAbilitySystemLibrary::SetDebuffDataContext(EffectContextHandle, DebuffData);

	// 이벤트 전달용 Attribute Modifier를 생성 및 할당합니다.
	// Magnitude가 0이기 때문에 Attribute에 값 변화가 일어나지 않고 일어난다 해도 게임 플레이에 영향을 주지 않습니다.
	// 하지만 AttributeSet에게 '이 Attribute에 값 변화가 발생했다.'고 이벤트가 전달됩니다.
	// 사실상 GAS의 시스템을 이용한 트릭성 로직입니다.
	const FGameplayModifierEvaluatedData EvaluatedData(UAuraAttributeSet::GetIncomingDebuffAttribute(), EGameplayModOp::Additive, 0.f);
	OutExecutionOutput.AddOutputModifier(EvaluatedData);
}
