#pragma once

#include "CoreMinimal.h"
#include "AuraWidgetController.h"
#include "AttributeMenuWidgetController.generated.h"

class UAttributeInfo;
struct FAuraAttributeInfo;
struct FGameplayTag;
struct FGameplayAttribute;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAttributeInfoSignature, const FAuraAttributeInfo&, Info);

UCLASS(BlueprintType, Blueprintable)
class AURA_API UAttributeMenuWidgetController : public UAuraWidgetController
{
	GENERATED_BODY()

public:
	virtual void BindCallbacksToDependencies() override;
	virtual void BroadcastInitialValue() override;

	UFUNCTION(BlueprintCallable)
	void UpgradeAttribute(const FGameplayTag& AttributeTag);

private:
	void BroadcastAttributeInfo(const FGameplayTag& AttributeTag, const FGameplayAttribute& Attribute) const;

public:
	UPROPERTY(BlueprintAssignable, Category = "GAS | Attributes")
	FAttributeInfoSignature AttributeInfoDelegate;
	
	UPROPERTY(BlueprintAssignable, Category = "GAS | Attributes")
	FOnPlayerStatChangedSignature OnAttributePointsChangedDelegate;

protected:
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UAttributeInfo> AttributeInfo;
};
