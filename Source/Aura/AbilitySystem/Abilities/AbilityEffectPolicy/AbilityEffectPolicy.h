#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectTypes.h"
#include "UObject/Object.h"
#include "AbilityEffectPolicy.generated.h"

class UGameplayEffect;
class UGameplayAbility;

/**
 * Ability의 Effect 부여를 담당하는 클래스입니다.
 * 파생된 자식 클래스는 필요한 GameplayEffect 클래스와 함께 그에 관련된 멤버 변수가 선언 및 할당됩니다.
 */
UCLASS(Blueprintable, EditInlineNew, DefaultToInstanced)
class AURA_API UAbilityEffectPolicy : public UObject
{
	GENERATED_BODY()

public:
	virtual void ApplyEffect(UGameplayAbility* OwningAbility, AActor* TargetActor) PURE_VIRTUAL(...);
	virtual void EndAbility() PURE_VIRTUAL(...);

	FGameplayEffectContextHandle GetEffectContextHandle() const;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UGameplayEffect> EffectClass;

	FGameplayEffectContextHandle EffectContextHandle;
};
