#pragma once

#include "CoreMinimal.h"
#include "AbilityEffectPolicy.h"
#include "GameplayEffectTypes.h"
#include "AbilityEffectPolicy_Debuff.generated.h"

class UGameplayEffect;

USTRUCT(BlueprintType)
struct FDebuffData
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float DebuffChance = 0.f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float DebuffDuration = 0.f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float DebuffDamage = 0.f;
};

/**
 * Buff 관련 EffectPolicy의 경우 Buff 하나하나에 대응하는 클래스가 있으나, 이 클래스는 Debuff 범용 클래스입니다.
 * 추후 Buff처럼 일대일 대응으로 리팩토링할 예정입니다.
 */

UCLASS(Blueprintable)
class AURA_API UAbilityEffectPolicy_Debuff : public UAbilityEffectPolicy
{
	GENERATED_BODY()

public:
	virtual void ApplyEffect(UGameplayAbility* OwningAbility, AActor* TargetActor, const FEffectPolicyContext& EffectPolicyContext) override;
	virtual void EndAbility() override;
	
	TArray<FGameplayEffectSpecHandle> MakeDebuffSpecHandle(const UGameplayAbility* OwningAbility);
	void CauseDebuff(const UGameplayAbility* OwningAbility, AActor* TargetActor, const TArray<FGameplayEffectSpecHandle>& DebuffSpecs);
	
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Debuff")
	TArray<FDebuffData> DebuffData;
};
