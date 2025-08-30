#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "NiagaraComponent.h"
#include "DebuffNiagaraComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class AURA_API UDebuffNiagaraComponent : public UNiagaraComponent
{
	GENERATED_BODY()

public:
	UDebuffNiagaraComponent();

protected:
	// ~ActorComponent Interface
	virtual void BeginPlay() override;
	// ~End of ActorComponent Interface

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void DebuffTagChanged(const FGameplayTag CallbackTag, int32 NewCount);

	UFUNCTION()
	void OnOwnerDeath(AActor* DeadActor);

	UFUNCTION()
	void OnRep_DebuffTag();

	UFUNCTION(Client, Reliable)
	void ClientDeactivateNiagara();
	
public:
	UPROPERTY(ReplicatedUsing = OnRep_DebuffTag)
	FGameplayTag DebuffTag;
};
