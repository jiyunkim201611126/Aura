#pragma once

#include "CoreMinimal.h"
#include "AuraWidgetController.h"
#include "Aura/Manager/AuraGameplayTags.h"
#include "SpellMenuWidgetController.generated.h"

struct FGameplayTag;

struct FSelectedAbility
{
	FGameplayTag Ability = FGameplayTag();
	FGameplayTag Status = FGameplayTag();
	int32 CurrentLevel = 0;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnSpellMenuStatusChangedSignature, bool, bSpendPointsButtonEnabled, bool, bEquipButtonEnabled, FText, DescriptionText, FText, NextLevelDescriptionText);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWaitForEquipSelectionSignature, const FGameplayTag&, AbilityType);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSuccessRequestEquipSignature);

UCLASS(BlueprintType, Blueprintable)
class AURA_API USpellMenuWidgetController : public UAuraWidgetController
{
	GENERATED_BODY()

public:
	virtual void BindCallbacksToDependencies() override;
	virtual void BroadcastInitialValue() override;

	// 선택된 SpellGlobe를 기반으로 SelectedAbility를 컨트롤하는 함수입니다.
	UFUNCTION(BlueprintCallable)
	void SpellGlobeSelected(const FGameplayTag& AbilityTag);

	UFUNCTION(BlueprintCallable)
	void SpellGlobeDeselected();

	UFUNCTION(BlueprintCallable)
	void SpendPointButtonPressed(const FGameplayTag& AbilityTag);

	UFUNCTION(BlueprintCallable)
	void EquipButtonPressed();

	// Equip 버튼 클릭 후 그 아래 장착 슬롯을 클릭했을 때 호출되는 함수
	UFUNCTION(BlueprintCallable)
	void SpellRowGlobePressed(const FGameplayTag& InputTag, const FGameplayTag& AbilityType);

	void OnAbilityEquipped(const FGameplayTag& AbilityTag, const FGameplayTag& InputTag, const FGameplayTag& StatusTag, const FGameplayTag& PreviousInputTag);

private:
	// SpendPoints 버튼과 Equip 버튼 활성화 여부를 결정 및 델리게이트를 호출하는 함수입니다.
	void ShouldEnableButtons();

public:
	UPROPERTY(BlueprintAssignable, Category = "GAS | SpellMenu")
	FOnPlayerStatChangedSignature OnSpellPointsChanged;

	UPROPERTY(BlueprintAssignable, Category = "GAS | SpellMenu")
	FOnSpellMenuStatusChangedSignature OnSpellMenuStatusChangedDelegate;

	UPROPERTY(BlueprintAssignable, Category = "GAS | SpellMenu")
	FWaitForEquipSelectionSignature WaitForEquipSelectionDelegate;

	UPROPERTY(BlueprintAssignable, Category = "GAS | SpellMenu")
	FWaitForEquipSelectionSignature StopWaitingForEquipSelectionDelegate;

	UPROPERTY(BlueprintAssignable, Category = "GAS | SpellMenu")
	FOnSuccessRequestEquipSignature OnSuccessRequestEquipDelegate;

private:
	FSelectedAbility SelectedAbility = { FAuraGameplayTags::Get().Abilities_None, FAuraGameplayTags::Get().Abilities_Status_Locked };
	int32 CurrentSpellPoints = 0;
	bool bWaitingForEquipSelection = false;
	FGameplayTag SelectedInputTag;
};
