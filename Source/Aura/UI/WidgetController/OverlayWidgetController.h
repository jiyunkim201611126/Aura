#pragma once

#include "CoreMinimal.h"
#include "AuraWidgetController.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "OverlayWidgetController.generated.h"

class UAuraAbilitySystemComponent;
struct FAuraAbilityInfo;
class UAbilityInfo;
class UAuraUserWidget;
struct FGameplayAbilitySpec;
struct FOnAttributeChangeData;

USTRUCT(BlueprintType)
struct FUIWidgetRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag AssetTag = FGameplayTag();

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<UAuraUserWidget> MessageWidget;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText Message = FText();

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UTexture2D* Image = nullptr;
};

USTRUCT(BlueprintType)
struct FAbilityUsableTypeInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	bool bIsStackable = false;

	FORCEINLINE bool HasAnyTrue() const
	{
		return bIsStackable;
	}

	// 추후 Ability 사용 타입이 추가됐을 때, UI에도 반영해야 하는 경우 이곳에 변수를 추가해 HUD에 알려줍니다.
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAttributeChangedSignature, float, NewValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMessageWidgetRowSignature, FUIWidgetRow, Row);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAbilityUsableTypeSignature, FGameplayTag, InAbilityTag, const FAbilityUsableTypeInfo&, Info);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStackCountChangedSignature, FGameplayTag, InAbilityTag, int32, StackCount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnStackTimerStartedSignature, FGameplayTag, InAbilityTag, float, RemainingTime, float, RechargeTime);

UCLASS(BlueprintType, Blueprintable)
class AURA_API UOverlayWidgetController : public UAuraWidgetController
{
	GENERATED_BODY()

public:
	//~ Begin AuraWidgetController Interface
	virtual void BindCallbacksToDependencies() override;
	virtual void BroadcastInitialValue() override;
	//~ End AuraWidgetController Interface

protected:
	template<typename T>
	T* GetDataTableRowByTag(UDataTable* DataTable, const FGameplayTag& Tag);

	void BindForUsableTypes(UAuraAbilitySystemComponent* AuraASC, FGameplayTag AbilityTag, bool bShouldRequestStackTime) const;
	
	void OnXPChanged(int32 InXP);
	
	virtual void OnAbilityEquipped(const FGameplayTag& AbilityTag, const FGameplayTag& InputTag, const FGameplayTag& StatusTag, const FGameplayTag& PreviousInputTag) override;

public:
	// 아래 델리게이트들에 위젯 블루프린트들이 각자 필요한 함수를 바인드
	UPROPERTY(BlueprintAssignable, Category = "GAS | Attributes")
	FOnAttributeChangedSignature OnHealthChanged;
	
	UPROPERTY(BlueprintAssignable, Category = "GAS | Attributes")
	FOnAttributeChangedSignature OnMaxHealthChanged;

	UPROPERTY(BlueprintAssignable, Category = "GAS | Attributes")
	FOnAttributeChangedSignature OnManaChanged;

	UPROPERTY(BlueprintAssignable, Category = "GAS | Attributes")
	FOnAttributeChangedSignature OnMaxManaChanged;

	UPROPERTY(BlueprintAssignable, Category = "GAS | Messages")
	FMessageWidgetRowSignature MessageWidgetRowDelegate;
	
	UPROPERTY(BlueprintAssignable, Category = "GAS | XP")
	FOnAttributeChangedSignature OnXPBarPercentChangedDelegate;

	UPROPERTY(BlueprintAssignable, Category = "GAS | Level")
	FOnPlayerStatChangedSignature OnPlayerLevelChangedDelegate;

	UPROPERTY(BlueprintAssignable, Category = "GAS | AbilityIcon")
	FOnAbilityUsableTypeSignature OnAbilityUsableTypeDelegate;

	UPROPERTY(BlueprintAssignable, Category = "GAS | AbilityIcon")
	FOnStackCountChangedSignature OnStackCountChangedDelegate;
	
	UPROPERTY(BlueprintAssignable, Category = "GAS | AbilityIcon")
	FOnStackTimerStartedSignature OnStackTimerStartedDelegate;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Widget Data")
	TObjectPtr<UDataTable> MessageWidgetDataTable;
};

template <typename T>
T* UOverlayWidgetController::GetDataTableRowByTag(UDataTable* DataTable, const FGameplayTag& Tag)
{
	return DataTable->FindRow<T>(Tag.GetTagName(), TEXT(""));
}
