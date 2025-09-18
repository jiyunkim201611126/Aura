#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "NiagaraComponent.h"
#include "AuraNiagaraComponent.generated.h"

UCLASS(BlueprintType)
class AURA_API UAuraNiagaraComponent : public UNiagaraComponent
{
	GENERATED_BODY()

public:
	UAuraNiagaraComponent();

	//~ Begin UObject Interface
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	//~ End UObject Interface

protected:
	//~ Begin UActorComponent Interface
	virtual void BeginPlay() override;
	//~ End UActorComponent Interface

	UFUNCTION()
	void OnRep_NiagaraTag();
	
	void NiagaraTagChanged(const FGameplayTag CallbackTag, int32 NewCount);

	UFUNCTION()
	void OnOwnerDeath(AActor* DeadActor);

	UFUNCTION(Client, Reliable)
	void ClientDeactivateNiagara();
	
public:
	UPROPERTY(ReplicatedUsing = OnRep_NiagaraTag, BlueprintReadWrite)
	FGameplayTag NiagaraTag;
};
