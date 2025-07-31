#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "WaitCooldownChange.generated.h"

struct FActiveGameplayEffectHandle;
struct FGameplayEffectSpec;
class UAbilitySystemComponent;
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FCooldownChangeSignature);

/**
 * ASC에게 Cooldown Tag가 부여 및 제거되는 이벤트를 지켜보는 태스크입니다.
 * 각 SkillGlobe(스킬 아이콘을 표시하는 위젯)들이 하나씩 할당받은 Cooldown Tag를 기반으로 작동합니다.
 *
 * 현재 사용하지 않는 클래스입니다.
 * 태스크를 사용해 이벤트 기반으로 로직을 작성할 경우, UI가 제대로 표시되지 않는 현상이 있습니다.
 * 때문에 UI가 Tick에서 직접 ASC의 Cooldown Tag 상태를 감시하는 로직으로 변경했습니다.
 */

UCLASS(BlueprintType, meta = (ExposedAsyncProxy = "AsyncTask"))
class AURA_API UWaitCooldownChange : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable)
	FCooldownChangeSignature CooldownStart;
	
	UPROPERTY(BlueprintAssignable)
	FCooldownChangeSignature CooldownEnd;

	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"))
	static UWaitCooldownChange* WaitForCooldownChange(UAbilitySystemComponent* InASC, const FGameplayTag& InCooldownTag);

	UFUNCTION(BlueprintCallable)
	void EndTask();

protected:
	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	FGameplayTag CooldownTag;

	void OnActiveEffectAdded(UAbilitySystemComponent* TargetASC, const FGameplayEffectSpec& SpecApplied, FActiveGameplayEffectHandle ActiveGameplayEffect);
	void CooldownTagChanged(const FGameplayTag InCooldownTag, int32 NewCount);
};
