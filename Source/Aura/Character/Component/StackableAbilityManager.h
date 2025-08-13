#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "StackableAbilityManager.generated.h"

USTRUCT(BlueprintType)
struct FAbilityStackItem : public FFastArraySerializerItem
{
	GENERATED_BODY()

	UPROPERTY()
	FGameplayTag AbilityTag;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stackable")
	int32 CurrentStack = 0;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stackable")
	int32 MaxStack;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stackable")
	float RechargeTime = 0.f;
	
	// 충전 타이머가 시작돼야 함을 클라이언트에게 명시적으로 알려주기 위한 변수입니다.
	// 이는 이벤트 중복을 방지하기 위해 작성되었습니다.
	UPROPERTY()
	bool bShouldTimerStart = true;
	
	FTimerHandle RechargeTimerHandle;

	// MarkArrayDirty, MarkItemDirty에 의해 호출되는 함수들입니다.
	// 네이밍 기반으로 호출되기 때문에 더이상의 처리가 필요하지 않습니다.
	void PostReplicatedAdd(const struct FAbilityStackArray& InArraySerializer);
	void PostReplicatedChange(const FAbilityStackArray& InArraySerializer);
	void PostReplicatedRemove(const FAbilityStackArray& InArraySerializer);

	/**
	 * 원한다면 RechargeStackAtOnce, UseStackAtOnce 등을 선언해 한 번에 충전되는 횟수나 사용되는 횟수를 정해줄 수도 있습니다.
	 * 위 변수의 값들은 Ability에서 결정해 넘겨줍니다.
	 */
};

USTRUCT()
struct FAbilityStackArray : public FFastArraySerializer
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FAbilityStackItem> Items;

	UPROPERTY(NotReplicated)
	class AStackableAbilityManager* OwnerActor = nullptr;

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParams)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FAbilityStackItem, FAbilityStackArray>(Items, DeltaParams, *this);
	}
};
template<>
struct TStructOpsTypeTraits<FAbilityStackArray> : public TStructOpsTypeTraitsBase2<FAbilityStackArray>
{
	enum { WithNetDeltaSerializer = true };
};

DECLARE_DELEGATE_TwoParams(FOnStackCountChanged, FGameplayTag /*InAbilityTag*/, int32 /*StackCount*/);
DECLARE_DELEGATE_TwoParams(FOnStackTimerStarted, FGameplayTag /*InAbilityTag*/, float /*RechargeTime*/);

UCLASS()
class AURA_API AStackableAbilityManager : public AActor
{
	GENERATED_BODY()

public:
	AStackableAbilityManager();

	// ~ActorComponent Interface
	virtual void Destroyed() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	// ~End of ActorComponent Interface
	
	void RegisterAbility(FGameplayTag AbilityTag, int32 CurrentStack, int32 MaxStack, float RechargeTime);
	void UnregisterAbility(FGameplayTag AbilityTag);

	bool CheckCost(FGameplayTag AbilityTag) const;
	void ApplyCost(FGameplayTag AbilityTag);

	int32 GetCurrentStack(FGameplayTag AbilityTag) const;
	bool CheckHasAbility(FGameplayTag AbilityTag) const;

	const FAbilityStackItem* FindItem(const FGameplayTag AbilityTag) const;

private:
	void StartRecharge(FGameplayTag AbilityTag);
	void Recharge(FGameplayTag AbilityTag);
	void StopRecharge(FGameplayTag AbilityTag);

	int32 FindIndexByTag(const FGameplayTag AbilityTag) const;
	FAbilityStackItem* FindItemMutable(const FGameplayTag AbilityTag);

public:
	// WidgetController가 바인드할 델리게이트
	FOnStackCountChanged OnStackCountChanged;
	FOnStackTimerStarted OnStackTimerStarted;

private:
	UPROPERTY(Replicated)
	FAbilityStackArray AbilityStacks;

	// 복제되지 않는 데이터입니다. 해당하는 Ability Tag가 몇 번째 인덱스에 존재하는지 로컬에서 캐싱하는 용도입니다.
	UPROPERTY(Transient)
	TMap<FGameplayTag, int32> TagToIndex;

	friend struct FAbilityStackItem;
	friend struct FAbilityStackArray;
};
