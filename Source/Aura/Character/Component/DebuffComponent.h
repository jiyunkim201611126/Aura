#pragma once

#include "CoreMinimal.h"
#include "Components/PawnComponent.h"
#include "DebuffComponent.generated.h"

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
	// ~UActorComponent Interface
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void BeginPlay() override;
	// ~End of UActorComponent Interface

private:
	void HitReactTagChanged(const FGameplayTag CallbackTag, int32 NewCount);
	void BurnTagChanged(const FGameplayTag CallbackTag, int32 NewCount);
	void StunTagChanged(const FGameplayTag CallbackTag, int32 NewCount);
	
	UFUNCTION()
	void OnRep_Stunned();

public:
	// AnimBP에서 참조하는 용도로 사용하는 bool 변수들입니다.
	UPROPERTY(BlueprintReadOnly, Category = "Debuff")
	bool bHitReacting = false;

	UPROPERTY(ReplicatedUsing = OnRep_Stunned, BlueprintReadOnly, Category = "Debuff")
	bool bIsStunned = false;

private:
	UPROPERTY()
	TWeakObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY()
	TWeakObjectPtr<AAuraAIController> AuraAIController;
};
