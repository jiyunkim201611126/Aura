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
	FGameplayTag DebuffType;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float DebuffChance = 0.f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float DebuffDamage = 0.f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float DebuffDuration = 0.f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float DebuffFrequency = 0.f;
};

UCLASS()
class AURA_API UAbilityEffectPolicy_Debuff : public UAbilityEffectPolicy
{
	GENERATED_BODY()

public:
	virtual void EndAbility() override;
	
	virtual void ApplyAllEffect(UGameplayAbility* OwningAbility, AActor* TargetActor) override;
	
	TArray<FGameplayEffectSpecHandle> MakeDebuffSpecHandle(const UGameplayAbility* OwningAbility);
	void CauseDebuff(const UGameplayAbility* OwningAbility, AActor* TargetActor, const TArray<FGameplayEffectSpecHandle>& DebuffSpecs);

	FGameplayEffectContextHandle DebuffEffectContextHandle;
	
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Debuff")
	TSubclassOf<UGameplayEffect> DebuffEffectClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Debuff")
	TArray<FDebuffData> DebuffData;
};
