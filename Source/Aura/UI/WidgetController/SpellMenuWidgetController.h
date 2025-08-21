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
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnSpellMenuStatusChangedSignature, bool, bSpendPointsButtonEnabled, bool, bEquipButtonEnabled, FText, DescriptionText, FText, NextLevelDescriptionText);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWaitForEquipSelectionSignature, const FGameplayTag&, AbilityType);

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
	void SpendPointButtonPressed(const FGameplayTag& AbilityTag);

	UFUNCTION(BlueprintCallable)
	void GlobeDeselect();

	UFUNCTION(BlueprintCallable)
	void EquipButtonPressed();

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

private:
	FSelectedAbility SelectedAbility = { FAuraGameplayTags::Get().Abilities_None, FAuraGameplayTags::Get().Abilities_Status_Locked };
	int32 CurrentSpellPoints = 0;
	bool bWaitingForEquipSelection = false;
};
