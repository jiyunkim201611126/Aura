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
class UNiagaraSystem;

UCLASS()
class AURA_API AAuraCharacterBase : public ACharacter, public IAbilitySystemInterface, public ICombatInterface
{
	GENERATED_BODY()

public:
	AAuraCharacterBase();

	virtual void BeginPlay() override;

	virtual void PossessedBy(AController* NewController) override;
	
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	UAttributeSet* GetAttributeSet() const { return AttributeSet; }
	
	/** Combat Interface */
	virtual FVector GetCombatSocketLocation_Implementation(const FGameplayTag& SocketTag) override;
	virtual void Die(bool bShouldAddImpulse, const FVector& Impulse) override;
	virtual bool IsDead_Implementation() override;
	virtual AActor* GetAvatar_Implementation() override;
	/** end Combat Interface*/

	UFUNCTION(NetMulticast, Reliable)
	virtual void MulticastHandleDeath(bool bShouldAddImpulse, const FVector& Impulse);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastSpawnDamageText(float Damage, bool bBlockedHit, bool bCriticalHit);

protected:
	// 모든 캐릭터가 무기를 사용하는 건 아니지만, 확장성과 유연성을 위해 선언해놓습니다.
	// 무기가 없는 캐릭터라도, 캐릭터에게 '무기처럼 보이는 이펙트'나 '임시 무기'를 쥐여줘야 하는 때에 무리 없이 구현할 수 있습니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<USkeletalMeshComponent> Weapon;

	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY()
	TObjectPtr<UAttributeSet> AttributeSet;

	virtual void InitAbilityActorInfo();

	// GameplayEffect를 본인에게 적용하는 함수
	void ApplyEffectToSelf(TSubclassOf<UGameplayEffect> GameplayEffectClass, float Level) const;
	// 적(BeginPlay)과 플레이어 캐릭터(Possess)가 Ability를 장착하는 함수
	virtual void AddCharacterStartupAbilities() const;
	
	void Dissolve();

	UFUNCTION(BlueprintImplementableEvent)
	void StartDissolveTimeline(UMaterialInstanceDynamic* DynamicMaterialInstance, UMaterialInstanceDynamic* WeaponDynamicMaterialInstance);
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UMaterialInstance> DissolveMaterialInstance;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UMaterialInstance> WeaponDissolveMaterialInstance;
	
	// 게임 시작 시 장착하고 있는 Ability
	UPROPERTY(EditAnywhere, Category = "Abilities")
	TArray<TSubclassOf<UGameplayAbility>> StartupAbilities;

	UPROPERTY(EditAnywhere, Category = "Combat")
	FGameplayTag DeathSoundTag;

private:
	bool bDead = false;
};
