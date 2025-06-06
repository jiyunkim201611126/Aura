#include "ExecCalc_Damage.h"

#include "AbilitySystemComponent.h"
#include "Aura/AbilitySystem/AuraAttributeSet.h"

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
	// FGameplayEffectAttributeCaptureDefinition 선언을 자동화해주는 매크로
	DECLARE_ATTRIBUTE_CAPTUREDEF(Armor);
	
	AuraDamageStatics()
	{
		// FGameplayEffectAttributeCaptureDefinition 초기화를 자동화해주는 매크로
		// Armor를 가져오는 방법을 세팅, 어떤 AttributeSet / 어떤 Attribute / 시전자와 피격자 중 누구 / 지금 즉시 혹은 이후로 계속 반영
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, Armor, Target, false);
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
	RelevantAttributesToCapture.Add(DamageStatics().ArmorDef);
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

	// 지역변수 Armor 선언
	float Armor = 0.f;
	// DamageStatics 구조체에 정의된 ArmorDef를 통해 피격자의 Armor Attribute 값을 캡처해서 Armor 변수에 저장
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().ArmorDef, EvaluationParameters, Armor);
	// 그 값을 지역변수 Armor에 초기화
	Armor = FMath::Max<float>(0.f, Armor);
	++Armor;

	// Armor Attribute에 Armor만큼 Additive(더하기) 연산을 적용하라는 Modifier 데이터를 생성
	const FGameplayModifierEvaluatedData EvaluatedData(DamageStatics().ArmorProperty, EGameplayModOp::Additive, Armor);
	// 이번 ExecCalc의 결과로 Modifier를 Output에 추가 (실제 적용은 GAS가 처리하며, 해당 클래스는 계산만 수행)
	OutExecutionOutput.AddOutputModifier(EvaluatedData);
}
