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

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSpellGlobeSelectedSignature, bool, bSpendPointsButtonEnabled, bool, bEquipButtonEnabled);

UCLASS(BlueprintType, Blueprintable)
class AURA_API USpellMenuWidgetController : public UAuraWidgetController
{
	GENERATED_BODY()

public:
	virtual void BindCallbacksToDependencies() override;
	virtual void BroadcastInitialValue() override;

	// SpellGlobe가 선택되었을 때, SpendPoints 버튼과 Equip 버튼 활성화 여부를 결정 및 델리게이트를 호출하는 함수입니다.
	UFUNCTION(BlueprintCallable)
	void SpellGlobeSelected(const FGameplayTag& AbilityTag);

	UFUNCTION(BlueprintCallable)
	void SpendPointButtonPressed(const FGameplayTag& AbilityTag);

private:
	// SpendPoints 버튼과 Equip 버튼 활성화 여부를 결정하는 함수입니다.
	void ShouldEnableButtons(const FGameplayTag& AbilityStatus, int32 SpellPoints, bool& bShouldEnableSpellPointsButton, bool& bShouldEnableEquipButton);

public:
	UPROPERTY(BlueprintAssignable, Category = "GAS | SpellMenu")
	FOnPlayerStatChangedSignature OnSpellPointsChanged;

	UPROPERTY(BlueprintAssignable, Category = "GAS | SpellMenu")
	FSpellGlobeSelectedSignature OnSpellGlobeSelectedDelegate;

private:
	FSelectedAbility SelectedAbility = { FAuraGameplayTags::Get().Abilities_None, FAuraGameplayTags::Get().Abilities_Status_Locked };
	int32 CurrentSpellPoints = 0;
};
