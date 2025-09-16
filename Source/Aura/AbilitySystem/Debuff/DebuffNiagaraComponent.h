#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "NiagaraComponent.h"
#include "DebuffNiagaraComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), Blueprintable, BlueprintType)
class AURA_API UDebuffNiagaraComponent : public UNiagaraComponent
{
	GENERATED_BODY()

public:
	UDebuffNiagaraComponent();

	//~ Begin Object Interface
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	//~ End Object Interface

protected:
	//~ Begin ActorComponent Interface
	virtual void BeginPlay() override;
	//~ End ActorComponent Interface
	
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
