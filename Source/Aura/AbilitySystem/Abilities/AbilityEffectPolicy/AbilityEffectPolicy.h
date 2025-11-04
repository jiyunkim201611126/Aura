#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectTypes.h"
#include "UObject/Object.h"
#include "AbilityEffectPolicy.generated.h"

class UGameplayEffect;
class UGameplayAbility;

/**
 * ApplyEffect가 호출되기 전, 추가적인 매개변수가 필요한 경우 사용하는 Context입니다.
 * Effect를 갖고 있는 대상(Ability, Projectile 등)이 하위 클래스에 대한 정보나 분기 처리 필요 없이 Effect 부여가 가능하도록 하는 구조체입니다.
 */
USTRUCT(BlueprintType, Blueprintable)
struct FEffectPolicyContext
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	FVector OriginVector = FVector::ZeroVector;
};

/**
 * Ability의 Effect 부여를 담당하는 클래스입니다.
 * 파생된 자식 클래스는 필요한 GameplayEffect 클래스와 함께 그에 관련된 멤버 변수가 선언 및 할당됩니다.
 */
UCLASS(NotBlueprintable, EditInlineNew, DefaultToInstanced)
class AURA_API UAbilityEffectPolicy : public UObject
{
	GENERATED_BODY()

public:
	virtual void ApplyEffect(UGameplayAbility* OwningAbility, AActor* TargetActor, const FEffectPolicyContext& EffectPolicyContext) PURE_VIRTUAL(UAbilityEffectPolicy::ApplyEffect, );
	virtual void EndAbility() PURE_VIRTUAL(UAbilityEffectPolicy::EndAbility, );
	
	void MakeEffectContextHandle(const UGameplayAbility* OwningAbility);

	FGameplayEffectContextHandle GetEffectContextHandle() const;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UGameplayEffect> EffectClass;

	FGameplayEffectContextHandle EffectContextHandle;
};
