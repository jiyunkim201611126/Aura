#include "ExecCalc_Damage.h"

#include "AbilitySystemComponent.h"
#include "Aura/AbilitySystem/AuraAttributeSet.h"
#include "Aura/Manager/AuraGameplayTags.h"
#include "Aura/AbilitySystem/Data/CharacterClassInfo.h"
#include "Aura/AbilitySystem/AuraAbilitySystemLibrary.h"
#include "Aura/Interaction/CombatInterface.h"

/**
 * 아래 구문은 언리얼에서 제공하는 Capture 선언, 초기화 매크로입니다.
 * 편의성을 위해 제공되었으나, 선언되어있는 Attribute와 이름이 완전히 같아야만 사용 가능하기 때문에
 * Source와 Target의 같은 Attribute를 사용할 수 없다는 단점이 있습니다.
 * 따라서 다양한 상황에선 사용 불가능해 확장성이 저하됩니다.
 * 예) Source의 방어력 비례 데미지가 Target의 방어력에 비례해 줄어드는 계산은 구현 불가
 * 
 * struct AuraDamageStatics
 * {
 *		// FGameplayEffectAttributeCaptureDefinition 선언을 자동화해주는 매크로
 * 		DECLARE_ATTRIBUTE_CAPTUREDEF(Armor);
 * 	
 * 		AuraDamageStatics()
 * 		{
 * 			// FGameplayEffectAttributeCaptureDefinition 초기화를 자동화해주는 매크로
 * 			// Armor를 가져오는 방법을 세팅, 어떤 AttributeSet / 어떤 Attribute / 시전자와 피격자 중 누구 / 지금 즉시 혹은 이후로 계속 반영
 * 			DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, Armor, Target, false);
 * 		}
 * };
 */

/**
 * 데미지 계산 시마다 관련 Attribute를 캡쳐하는 건 비효율적입니다.
 * FGameplayEffectAttributeCaptureDefinition은 '어떤 Attribute를 어떻게 캡쳐하겠다'는 걸 표현하는 메타 데이터입니다.
 * 즉, 한 번 정의하고나면 변하지 않는 값입니다.
 * 또 데미지 계산은 굉장히 자주 발생하기 때문에 한 번 만들어두고 재사용하는 편이 훨씬 효율적입니다.
 * 따라서 Attribute 캡처 정의 구조체를 싱글톤 패턴으로 관리합니다.
 */

// 해당 cpp 파일에서만 사용하는 원시 구조체로, 블루프린트는 물론 다른 클래스에서도 노출되지 않기 때문에 네이밍에 F를 붙이지 않음
struct AuraDamageStatics
{
	FGameplayEffectAttributeCaptureDefinition TargetBlockChance;
	FGameplayEffectAttributeCaptureDefinition TargetArmor;
	FGameplayEffectAttributeCaptureDefinition SourceArmorPenetration;
	FGameplayEffectAttributeCaptureDefinition SourceCriticalHitChance;
	FGameplayEffectAttributeCaptureDefinition SourceCriticalHitDamage;
	FGameplayEffectAttributeCaptureDefinition TargetCriticalHitResistance;
	
	FGameplayEffectAttributeCaptureDefinition TargetFireResistance;
	FGameplayEffectAttributeCaptureDefinition TargetLightningResistance;
	FGameplayEffectAttributeCaptureDefinition TargetArcaneResistance;
	FGameplayEffectAttributeCaptureDefinition TargetPhysicalResistance;
	FGameplayEffectAttributeCaptureDefinition TargetDamageReduction;

	// 데미지 계산 시 편의성을 위해 선언, 각 Resistance의 키와 TargetResistance를 매칭시켜 저장 
	TMap<FGameplayTag, FGameplayEffectAttributeCaptureDefinition> TagsToCaptureResistanceDefs;

	AuraDamageStatics() :
	TargetBlockChance(UAuraAttributeSet::GetBlockChanceAttribute(), EGameplayEffectAttributeCaptureSource::Target, false),
	TargetArmor(UAuraAttributeSet::GetArmorAttribute(), EGameplayEffectAttributeCaptureSource::Target, false),
	SourceArmorPenetration(UAuraAttributeSet::GetArmorPenetrationAttribute(), EGameplayEffectAttributeCaptureSource::Source, false),
	SourceCriticalHitChance(UAuraAttributeSet::GetCriticalHitChanceAttribute(), EGameplayEffectAttributeCaptureSource::Source, false),
	SourceCriticalHitDamage(UAuraAttributeSet::GetCriticalHitDamageAttribute(), EGameplayEffectAttributeCaptureSource::Source, false),
	TargetCriticalHitResistance(UAuraAttributeSet::GetCriticalHitResistanceAttribute(), EGameplayEffectAttributeCaptureSource::Target, false),
	
	TargetFireResistance(UAuraAttributeSet::GetFireResistanceAttribute(), EGameplayEffectAttributeCaptureSource::Target, false),
	TargetLightningResistance(UAuraAttributeSet::GetLightningResistanceAttribute(), EGameplayEffectAttributeCaptureSource::Target, false),
	TargetArcaneResistance(UAuraAttributeSet::GetArcaneResistanceAttribute(), EGameplayEffectAttributeCaptureSource::Target, false),
	TargetPhysicalResistance(UAuraAttributeSet::GetPhysicalResistanceAttribute(), EGameplayEffectAttributeCaptureSource::Target, false),
	TargetDamageReduction(UAuraAttributeSet::GetDamageReductionAttribute(), EGameplayEffectAttributeCaptureSource::Target, false)
	{
		TagsToCaptureResistanceDefs.Add(FAuraGameplayTags::Get().Attributes_Resistance_Fire, TargetFireResistance);
		TagsToCaptureResistanceDefs.Add(FAuraGameplayTags::Get().Attributes_Resistance_Lightning, TargetLightningResistance);
		TagsToCaptureResistanceDefs.Add(FAuraGameplayTags::Get().Attributes_Resistance_Arcane, TargetArcaneResistance);
		TagsToCaptureResistanceDefs.Add(FAuraGameplayTags::Get().Attributes_Resistance_Physical, TargetPhysicalResistance);
	}
};

static const AuraDamageStatics& DamageStatics()
{
	// 함수 내부에 static으로 선언된 변수로, 이 함수에서만 접근 가능한 변수이며 한 번 초기화되면 프로그램 종료 시까지 같은 인스턴스를 반환
	static AuraDamageStatics DStatics;
	return DStatics;
}

UExecCalc_Damage::UExecCalc_Damage()
{
	// 이 클래스의 계산과 관련된 Attribute로 등록
	RelevantAttributesToCapture.Add(DamageStatics().TargetBlockChance);
	RelevantAttributesToCapture.Add(DamageStatics().TargetArmor);
	RelevantAttributesToCapture.Add(DamageStatics().SourceArmorPenetration);
	RelevantAttributesToCapture.Add(DamageStatics().SourceCriticalHitChance);
	RelevantAttributesToCapture.Add(DamageStatics().SourceCriticalHitDamage);
	RelevantAttributesToCapture.Add(DamageStatics().TargetCriticalHitResistance);
	
	RelevantAttributesToCapture.Add(DamageStatics().TargetFireResistance);
	RelevantAttributesToCapture.Add(DamageStatics().TargetLightningResistance);
	RelevantAttributesToCapture.Add(DamageStatics().TargetArcaneResistance);
	RelevantAttributesToCapture.Add(DamageStatics().TargetPhysicalResistance);
	RelevantAttributesToCapture.Add(DamageStatics().TargetDamageReduction);
}

void UExecCalc_Damage::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	// 스킬 시전자와 피격자의 ASC 가져오기
	const UAbilitySystemComponent* SourceASC = ExecutionParams.GetSourceAbilitySystemComponent();
	const UAbilitySystemComponent* TargetASC = ExecutionParams.GetTargetAbilitySystemComponent();

	// 스킬 시전자와 피격자의 AvatarActor 가져오기
	AActor* SourceAvatar = SourceASC ? SourceASC->GetAvatarActor() : nullptr;
	AActor* TargetAvatar = TargetASC ? TargetASC->GetAvatarActor() : nullptr;

	if (!SourceASC || !TargetASC || !SourceAvatar || !TargetAvatar)
	{
		return;
	}

	int32 SourcePlayerLevel = 1;
	if (SourceAvatar->Implements<UCombatInterface>())
	{
		SourcePlayerLevel = ICombatInterface::Execute_GetCharacterLevel(SourceAvatar);
	}
	int32 TargetPlayerLevel = 1;
	if (TargetAvatar->Implements<UCombatInterface>())
	{
		TargetPlayerLevel = ICombatInterface::Execute_GetCharacterLevel(TargetAvatar);
	}	

	// 지금 적용 중인 GE에 대한 정보
	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();

	// 시전자와 피격자의 태그 정보 가져와서 평가 파라미터에 세팅
	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();
	FAggregatorEvaluateParameters EvaluationParameters;
	EvaluationParameters.SourceTags = SourceTags;
	EvaluationParameters.TargetTags = TargetTags;
	
	FGameplayEffectContextHandle EffectContextHandle = Spec.GetContext();

	// 일치하는 데미지 타입 탐색 후 부여된 데미지에 따른 계산 진행
	FGameplayTag DamageType;
	float Damage = 0.f;

	// 현재 데미지 타입과 일치하는 데미지를 탐색합니다.
	for (auto& Pair : FAuraGameplayTags::Get().DamageTypesToResistances)
	{
		const FGameplayTag DamageTypeTag = Pair.Key;
		const FGameplayTag ResistanceTypeTag = Pair.Value;
		checkf(AuraDamageStatics().TagsToCaptureResistanceDefs.Contains(ResistanceTypeTag), TEXT("TagsToCaptureResistanceDefs doesn't contain Tag: [%s] in ExecCalc_Damage."), *ResistanceTypeTag.ToString());

		float DamageTypeValue = Spec.GetSetByCallerMagnitude(DamageTypeTag, false);

		if (DamageTypeValue > 0.f)
		{
			// Resistance Value 가져오옵니다.
			const FGameplayEffectAttributeCaptureDefinition Resistance = AuraDamageStatics().TagsToCaptureResistanceDefs[ResistanceTypeTag];
			float ResistanceTypeValue = 0.f;
			ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(Resistance, EvaluationParameters, ResistanceTypeValue);
			// Resistance는 음수일 가능성도 있으므로 Clamp하지 않습니다.

			DamageType = Pair.Key;

			// 속성 데미지 계산 결과 반영 후 반복문을 빠져나갑니다.
			DamageTypeValue *= ( 100.f - ResistanceTypeValue ) / 100.f;
			Damage = DamageTypeValue;
			break;
		}
	}

	// DamageStatics 구조체에 정의된 FGameplayEffectAttributeCaptureDefinition들을 통해 Source와 Target의 현재 Attribute 값을 캡쳐
	float TargetBlockChance = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().TargetBlockChance, EvaluationParameters, TargetBlockChance);
	TargetBlockChance = FMath::Max<float>(0.0f, TargetBlockChance);
	
	float TargetArmor = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().TargetArmor, EvaluationParameters, TargetArmor);
	TargetArmor = FMath::Max<float>(0.0f, TargetArmor);
	
	float SourceArmorPenetration = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().SourceArmorPenetration, EvaluationParameters, SourceArmorPenetration);
	SourceArmorPenetration = FMath::Max<float>(0.0f, SourceArmorPenetration);
	
	float SourceCriticalHitChance = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().SourceCriticalHitChance, EvaluationParameters, SourceCriticalHitChance);
	SourceCriticalHitChance = FMath::Max<float>(0.0f, SourceCriticalHitChance);
	
	float SourceCriticalHitDamage = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().SourceCriticalHitDamage, EvaluationParameters, SourceCriticalHitDamage);
	SourceCriticalHitDamage = FMath::Max<float>(0.0f, SourceCriticalHitDamage);
	
	float TargetCriticalHitResistance = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().TargetCriticalHitResistance, EvaluationParameters, TargetCriticalHitResistance);

	// Attribute를 그대로 사용하는 게 아닌, 시전자와 대상자의 레벨 차이에 따라 차등 적용
	const int32 LevelGap = FMath::Max<int32>(0, TargetPlayerLevel - SourcePlayerLevel);

	// 어떤 수치로 차등 적용할 것인지 결정하기 위해 Curve Table 및 그 값 가져오기
	UCharacterClassInfo* CharacterClassInfo = UAuraAbilitySystemLibrary::GetCharacterClassInfo(SourceAvatar);
	const FRealCurve* ArmorPenetrationCurve = CharacterClassInfo->DamageCalculationCoefficients->FindCurve(FName("ArmorPenetration"), FString());
	const FRealCurve* EffectiveArmorCurve = CharacterClassInfo->DamageCalculationCoefficients->FindCurve(FName("EffectiveArmor"), FString());
	const FRealCurve* CriticalHitDamage = CharacterClassInfo->DamageCalculationCoefficients->FindCurve(FName("CriticalHitDamage"), FString());
	const float ArmorPenetrationCoefficient = ArmorPenetrationCurve->Eval(LevelGap);
	const float EffectiveArmorCoefficient = EffectiveArmorCurve->Eval(LevelGap);
	const float CriticalHitDamageCoefficient = CriticalHitDamage->Eval(LevelGap);

	// 데미지 계산을 시작합니다.
	bool bBlocked = false;
	bool bCritical = false;
	
	// Block 성공 시 데미지 절반
	if (FMath::RandRange(0.f, 100.f) < TargetBlockChance)
	{
		bBlocked = true;
		Damage = Damage / 2.f;
	}
	else
	{
		// Block된 경우 Critical은 발생하지 않음
		const float CriticalChance = FMath::Max<float>(0.f, SourceCriticalHitChance - TargetCriticalHitResistance);
		if (FMath::RandRange(0.f, 100.f) < CriticalChance)
		{
			bCritical = true;
			Damage = Damage * 2.f + SourceCriticalHitDamage * CriticalHitDamageCoefficient;
		}
	}

	// 시전자의 관통력 %만큼 대상의 방어력 무시
	// TargetArmor: 10, SourceArmorPenetration: 50 / 결과: 5로 적용 (레벨 차이 10 미만인 경우만)
	const float EffectiveArmor = TargetArmor * ((100 - FMath::Clamp(SourceArmorPenetration * ArmorPenetrationCoefficient, 0.f, 100.f)) / 100.f);
	// EffectiveArmor %만큼 데미지 경감
	// EffectiveArmor: 5 / 결과: 데미지의 95%만 적용 (레벨 차이 10 미만인 경우만)
	Damage *= FMath::Clamp(100 - EffectiveArmor * EffectiveArmorCoefficient, 0.f, 100.f) / 100.f;

	// Damage, Block 등 계산 결과를 Context에 기록합니다.
	UAuraAbilitySystemLibrary::SetDamageDataContext(EffectContextHandle, UAuraAbilitySystemLibrary::ReplaceDamageTypeToEnum(DamageType), bBlocked, bCritical);
	
	// 데미지 경감 수치를 적용합니다.
	float TargetDamageReduction = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().TargetDamageReduction, EvaluationParameters, TargetDamageReduction);
	Damage *= 1.f - FMath::Clamp(TargetDamageReduction, 0.f, 1.f);
	
	// IncomingDamage Attribute에 Damage만큼 Additive(더하기) 연산을 적용하라는 Modifier 데이터를 생성합니다.
	const FGameplayModifierEvaluatedData EvaluatedData(UAuraAttributeSet::GetIncomingDamageAttribute(), EGameplayModOp::Additive, Damage);
	// 이번 ExecCalc의 결과로 Modifier를 Output에 추가합니다. (실제 적용은 GAS가 처리하며, 해당 클래스는 계산만 수행)
	OutExecutionOutput.AddOutputModifier(EvaluatedData);
}
