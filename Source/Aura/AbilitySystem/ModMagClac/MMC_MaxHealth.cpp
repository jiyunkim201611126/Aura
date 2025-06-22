#include "MMC_MaxHealth.h"

#include "Aura/AbilitySystem/AuraAttributeSet.h"
#include "Aura/Interaction/CombatInterface.h"

UMMC_MaxHealth::UMMC_MaxHealth()
{
	// MaxHealth는 Vigor Attribute에 의해 결정되는 스탯
	VigorDef.AttributeToCapture = UAuraAttributeSet::GetVigorAttribute();

	// 캐릭터 자신의 스탯을 결정하는 중이므로 Source든 Target이든 관계 없음
	VigorDef.AttributeSource = EGameplayEffectAttributeCaptureSource::Target;

	// true면 적용 즉시 계산 후 이후로는 반영 안 함, false면 관련 Attribute 갱신 시마다 함께 갱신
	VigorDef.bSnapshot = false;

	// 이 클래스의 계산과 관련된 Attribute로 등록
	RelevantAttributesToCapture.Add(VigorDef);
}

float UMMC_MaxHealth::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	// Effect를 발생시킨 액터와 적용될 액터의 Tags를 가져오기
	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	FAggregatorEvaluateParameters EvaluationParameters;
	EvaluationParameters.SourceTags = SourceTags;
	EvaluationParameters.TargetTags = TargetTags;

	float Vigor = 0.f;
	GetCapturedAttributeMagnitude(VigorDef, Spec, EvaluationParameters, Vigor);
	Vigor = FMath::Max<float>(Vigor, 0.f);

	ICombatInterface* CombatInterface = Cast<ICombatInterface>(Spec.GetContext().GetSourceObject());
	const int32 PlayerLevel = CombatInterface->GetPlayerLevel();

	return 1.5f * Vigor + 10.f * PlayerLevel;
}
