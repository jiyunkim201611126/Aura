#pragma once

#include "CoreMinimal.h"
#include "AuraGameplayAbility.h"
#include "AuraPassiveAbility.generated.h"

class UAuraNiagaraComponent;

UCLASS()
class AURA_API UAuraPassiveAbility : public UAuraGameplayAbility
{
	GENERATED_BODY()

public:
	//~ Begin UGameplayAbility Interface
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	//~ End UGameplayAbility Interface
	
	void ReceiveDeactivate(const FGameplayTag& InAbilityTag);

protected:
	UFUNCTION(BlueprintImplementableEvent)
	void SetNiagaraComponentTransform();

protected:
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UAuraNiagaraComponent> NiagaraComponent;
};
