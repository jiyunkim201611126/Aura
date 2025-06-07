#include "ExecCalc_Damage.h"

#include "AbilitySystemComponent.h"
#include "Aura/AbilitySystem/AuraAttributeSet.h"
#include "Aura/Manager/AuraGameplayTags.h"

/**
 * 데미지 계산 시마다 관련 Attribute를 캡쳐하는 건 비효율적입니다.
 * FGameplayEffectAttributeCaptureDefinition은 '어떤 Attribute를 어떻게 캡쳐하겠다'는 걸 표현하는 메타 데이터입니다.
 * 즉, 한 번 정의하고나면 변하지 않는 값입니다.
 * 또 데미지 계산은 굉장히 자주 발생하기 때문에 한 번 만들어두고 재사용하는 편이 훨씬 효율적입니다. 
 * 따라서 Attribute 캡처 정의 구조체를 싱글톤 패턴으로 관리합니다.
 */

/**
 * 아래 구문은 언리얼에서 제공하는 Capture 선언, 초기화 매크로입니다.
 * 편의성을 위해 제공되었으나, 선언되어있는 Attribute와 이름이 완전히 같아야만 사용 가능하기 때문에
 * Source와 Target의 같은 Attribute를 사용할 수 없다는 단점이 있습니다.
 * 따라서 다양한 상황에선 사용 불가능해 확장성이 저하됩니다.
 * 예) Source의 방어력 비례 데미지가 Target의 방어력에 비례해 줄어듦
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

// 해당 cpp 파일에서만 사용하는 원시 구조체로, 블루프린트는 물론 다른 클래스에서도 노출되지 않기 때문에 네이밍에 F를 붙이지 않음
struct AuraDamageStatics
{
	FGameplayEffectAttributeCaptureDefinition SourceArmorPenetration;
	FGameplayEffectAttributeCaptureDefinition TargetArmorDef;
	FGameplayEffectAttributeCaptureDefinition TargetBlockChanceDef;

	AuraDamageStatics() :
	SourceArmorPenetration(UAuraAttributeSet::GetArmorPenetrationAttribute(), EGameplayEffectAttributeCaptureSource::Source, false),
	TargetArmorDef(UAuraAttributeSet::GetArmorAttribute(), EGameplayEffectAttributeCaptureSource::Target, false),
	TargetBlockChanceDef(UAuraAttributeSet::GetBlockChanceAttribute(), EGameplayEffectAttributeCaptureSource::Target, false)
	{}
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
	RelevantAttributesToCapture.Add(DamageStatics().TargetArmorDef);
	RelevantAttributesToCapture.Add(DamageStatics().TargetBlockChanceDef);
}

void UExecCalc_Damage::Execute_Implementation(
	const FGameplayEffectCustomExecutionParameters& ExecutionParams,
	FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	// 스킬 시전자와 피격자의 ASC 가져오기
	const UAbilitySystemComponent* SourceASC = ExecutionParams.GetSourceAbilitySystemComponent();
	const UAbilitySystemComponent* TargetASC = ExecutionParams.GetTargetAbilitySystemComponent();

	// 스킬 시전자와 피격자의 AvatarActor 가져오기
	const AActor* SourceAvatar = SourceASC ? SourceASC->GetAvatarActor() : nullptr;
	const AActor* TargetAvatar = TargetASC ? TargetASC->GetAvatarActor() : nullptr;

	// 지금 적용 중인 GE에 대한 정보
	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();

	// 시전자와 피격자의 태그 정보 가져와서 평가 파라미터에 세팅
	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();
	FAggregatorEvaluateParameters EvaluationParameters;
	EvaluationParameters.SourceTags = SourceTags;
	EvaluationParameters.TargetTags = TargetTags;

	// Damage Set by Caller Magnitude 가져오기
	float Damage = Spec.GetSetByCallerMagnitude(FAuraGameplayTags::Get().Damage);

	// DamageStatics 구조체에 정의된 FGameplayEffectAttributeCaptureDefinition들을 통해 Source와 Target의 현재 Attribute 값을 캡쳐
	float SourceArmorPenetration = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().SourceArmorPenetration, EvaluationParameters, SourceArmorPenetration);
	SourceArmorPenetration = FMath::Max<float>(0.0f, SourceArmorPenetration);

	float TargetBlockChance = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().TargetBlockChanceDef, EvaluationParameters, TargetBlockChance);
	TargetBlockChance = FMath::Max<float>(0.0f, TargetBlockChance);
	
	float TargetArmor = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().TargetArmorDef, EvaluationParameters, TargetArmor);
	TargetArmor = FMath::Max<float>(0.0f, TargetArmor);

	// 시전자의 관통력 %만큼 방어력 무시
	const float EffectiveArmor = TargetArmor * ((100 - SourceArmorPenetration) / 100.f);
	// EffectiveArmor %만큼 데미지 경감
	Damage *= (100 - EffectiveArmor) / 100.f;
	
	// Block 성공 시 데미지 절반
	const bool bBlocked = FMath::RandRange(1, 100) < TargetBlockChance;
	Damage = bBlocked ? Damage / 2.f : Damage;
	
	// IncomingDamage Attribute에 Damage만큼 Additive(더하기) 연산을 적용하라는 Modifier 데이터를 생성
	const FGameplayModifierEvaluatedData EvaluatedData(UAuraAttributeSet::GetIncomingDamageAttribute(), EGameplayModOp::Additive, Damage);
	// 이번 ExecCalc의 결과로 Modifier를 Output에 추가 (실제 적용은 GAS가 처리하며, 해당 클래스는 계산만 수행)
	OutExecutionOutput.AddOutputModifier(EvaluatedData);
}
