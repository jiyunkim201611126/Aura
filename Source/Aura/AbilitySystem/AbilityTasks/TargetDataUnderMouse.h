#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "TargetDataUnderMouse.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMouseTargetDataSignature, const FGameplayAbilityTargetDataHandle&, DataHandle);

/**
 * AbilityTask는 GameplayAbility 안에서 '어떤 이벤트를 기다렸다가 처리하는' 비동기 작업을 담당하는 클래스
 * 단, Activate에서 델리게이트를 즉시 Broadcast하면 비동기가 아닌 방식으로도 사용 가능
 */

UCLASS()
class AURA_API UTargetDataUnderMouse : public UAbilityTask
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintAssignable)
	FMouseTargetDataSignature ValidData;

	// TargetDataUnderMouse 객체를 생성해 반환하는 함수
	// OwningAbility 핀은 호출한 Ability가 매개변수로 들어가고, 핀으로 노출되지 않음
	UFUNCTION(BlueprintCallable, Category = "Ability|Tasks", meta = (DisplayName = "TargetDataUnderMouse", HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "true"))
	static UTargetDataUnderMouse* CreateTargetDataUnderMouse(UGameplayAbility* OwningAbility);

protected:
	virtual void Activate() override;

private:
	void SendMouseCursorData();

	void OnTargetDataReplicatedCallback(const FGameplayAbilityTargetDataHandle& DataHandle, FGameplayTag ActivationTag);
};
