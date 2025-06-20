#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "Aura/Interaction/CombatInterface.h"
#include "GameFramework/Character.h"
#include "AuraCharacterBase.generated.h"

class UAbilitySystemComponent;
class UAttributeSet;
class UGameplayEffect;
class UGameplayAbility;
class UAnimMontage;

UCLASS()
class AURA_API AAuraCharacterBase : public ACharacter, public IAbilitySystemInterface, public ICombatInterface
{
	GENERATED_BODY()

public:
	AAuraCharacterBase();
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	UAttributeSet* GetAttributeSet() const { return AttributeSet; }
	
	/** Combat Interface */
	virtual FVector GetCombatSocketLocation_Implementation() override;
	virtual UAnimMontage* GetHitReactMontage_Implementation() override;
	virtual void Die(bool bShouldAddImpulse, const FVector& Impulse) override;
	virtual bool IsDead_Implementation() override;
	virtual AActor* GetAvatar_Implementation() override;
	virtual TArray<FTaggedMontage> GetAttackMontages_Implementation() override;
	/** end Combat Interface*/

	UFUNCTION(NetMulticast, Reliable)
	virtual void MulticastHandleDeath(bool bShouldAddImpulse, const FVector& Impulse);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastSpawnDamageText(float Damage, bool bBlockedHit, bool bCriticalHit);

	UPROPERTY(EditAnywhere, Category = "Combat")
	TArray<FTaggedMontage> AttackMontages;

protected:
	UPROPERTY(EditAnywhere, Category = "Combat")
	TObjectPtr<USkeletalMeshComponent> Weapon;

	UPROPERTY(EditAnywhere, Category = "Combat")
	FName WeaponTipSocketName;

	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY()
	TObjectPtr<UAttributeSet> AttributeSet;

	virtual void InitAbilityActorInfo();

	// GameplayEffect를 본인에게 적용하는 함수
	void ApplyEffectToSelf(TSubclassOf<UGameplayEffect> GameplayEffectClass, float Level) const;
	// 게임 시작 시 캐릭터가 Ability를 장착하는 함수
	void AddCharacterAbilities() const;
	
	void Dissolve();

	UFUNCTION(BlueprintImplementableEvent)
	void StartDissolveTimeline(UMaterialInstanceDynamic* DynamicMaterialInstance, UMaterialInstanceDynamic* WeaponDynamicMaterialInstance);
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UMaterialInstance> DissolveMaterialInstance;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UMaterialInstance> WeaponDissolveMaterialInstance;

private:
	// 게임 시작 시 장착하고 있는 Ability
	UPROPERTY(EditAnywhere, Category = "Abilities")
	TArray<TSubclassOf<UGameplayAbility>> StartupAbilities;

	UPROPERTY(EditAnywhere, Category = "Combat")
	TObjectPtr<UAnimMontage> HitReactMontage;

	bool bDead = false;
};
