#pragma once

#include "CoreMinimal.h"
#include "Components/PawnComponent.h"
#include "DebuffComponent.generated.h"

class UAuraNiagaraComponent;
class AAuraAIController;
class AAuraCharacterBase;
class UAbilitySystemComponent;
struct FGameplayTag;

UCLASS(BlueprintType)
class AURA_API UDebuffComponent : public UPawnComponent
{
	GENERATED_BODY()

public:
	UDebuffComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	void InitAbilityActorInfo(UAbilitySystemComponent* InAbilitySystemComponent);

protected:
	//~ Begin ActorComponent Interface
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void BeginPlay() override;
	//~ End ActorComponent Interface

private:
	UAuraNiagaraComponent* CreateNiagaraComponent(const FGameplayTag& DebuffTypeTag);
	
	void HitReactTagChanged(const FGameplayTag CallbackTag, int32 NewCount);
	void BurnTagChanged(const FGameplayTag CallbackTag, int32 NewCount);
	void StunTagChanged(const FGameplayTag CallbackTag, int32 NewCount);

	void SetBlockInputState() const;

public:
	// AnimBP에서 참조하는 용도로 사용하는 bool 변수들입니다.
	UPROPERTY(BlueprintReadOnly, Category = "Debuff")
	bool bHitReacting = false;

	UPROPERTY(BlueprintReadOnly, Category = "Debuff")
	bool bIsStunned = false;

private:
	UPROPERTY()
	TWeakObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY()
	TWeakObjectPtr<AAuraAIController> AuraAIController;
};
