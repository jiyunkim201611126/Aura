#pragma once

#include "CoreMinimal.h"
#include "AuraWidgetController.h"
#include "SpellMenuWidgetController.generated.h"

UCLASS(BlueprintType, Blueprintable)
class AURA_API USpellMenuWidgetController : public UAuraWidgetController
{
	GENERATED_BODY()

public:
	virtual void BindCallbacksToDependencies() override;
	virtual void BroadcastInitialValue() override;

public:
	UPROPERTY(BlueprintAssignable, Category = "GAS | SpellMenu")
	FOnPlayerStatChangedSignature OnSpellPointsChanged;
};
