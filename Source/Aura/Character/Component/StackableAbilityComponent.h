#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "StackableAbilityComponent.generated.h"

USTRUCT(BlueprintType)
struct FAbilityStackData
{
	GENERATED_BODY()
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stackable")
	int32 CurrentStack;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stackable")
	int32 MaxStack;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stackable")
	float RechargeTime = 0.f;
	
	FTimerHandle RechargeTimerHandle;

	/**
	 * 원한다면 RechargeStackAtOnce, UseStackAtOnce 등을 선언해 한 번에 충전되는 횟수나 사용되는 횟수를 정해줄 수도 있습니다.
	 * 위 변수의 값들은 Ability에서 결정해 넘겨줍니다.
	 */
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class AURA_API UStackableAbilityComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	void RegisterAbility(FGameplayTag AbilityTag, int32 MaxStack, float RechargeTime);
	void UnregisterAbility(FGameplayTag AbilityTag);

	bool CheckCost(FGameplayTag AbilityTag) const;
	void ApplyCost(FGameplayTag AbilityTag);

	int32 GetCurrentStack(FGameplayTag AbilityTag) const;

private:
	void StartRecharge(FGameplayTag AbilityTag);
	void StopRecharge(FGameplayTag AbilityTag);
	void Recharge(FGameplayTag AbilityTag);

private:
	TMap<FGameplayTag, FAbilityStackData> AbilityStacks;
};
