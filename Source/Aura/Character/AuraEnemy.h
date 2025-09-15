#pragma once

#include "CoreMinimal.h"
#include "AuraCharacterBase.h"
#include "Aura/Interaction/EnemyInterface.h"
#include "Aura/UI/WidgetController/OverlayWidgetController.h"
#include "AuraEnemy.generated.h"

class UWidgetComponent;
class UBehaviorTree;
class AAuraAIController;

UCLASS()
class AURA_API AAuraEnemy : public AAuraCharacterBase, public IEnemyInterface
{
	GENERATED_BODY()

public:
	AAuraEnemy();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// ~Enemy Interface
	virtual void HighlightActor() override;
	virtual void UnHighlightActor() override;
	virtual void SetCombatTarget_Implementation(AActor* InCombatTarget) override;
	virtual AActor* GetCombatTarget_Implementation() const override;
	virtual void ShouldPlaySpawnAnimation() override;
	// ~End of Enemy Interface

	// ~Combat Interface
	virtual void RegisterPawn() override;
	virtual void UnregisterPawn() override;
	virtual int32 GetCharacterLevel_Implementation() override;
	virtual void Die(const FVector& Impulse) override;
	// ~End of Combat Interface

	virtual void MulticastDeath_Implementation(const FVector& Impulse) override;
	
protected:
	// ~AActor Interface
	virtual void BeginPlay() override;
	// ~End of AActor Interface
	
	// ~APawn Interface
	virtual void PossessedBy(AController* NewController) override;
	// ~End of APawn Interface

	// ~AuraCharacterBase Interface
	virtual void InitAbilityActorInfo() override;
	virtual void AddCharacterStartupAbilities() const override;
	// ~End of AuraCharacterBase Interface

private:
	UFUNCTION()
	void OnRep_PlaySpawnAnimation();

public:
	// 아래 2개의 델리게이트 선언으로 인해 이 클래스가 OverlayWidgetController를 참조하게 되었으나,
	// 이 클래스에 이름이 다른 같은 역할의 델리게이트를 선언하는 게 불필요하다 생각되어 이렇게 선언합니다.
	UPROPERTY(BlueprintAssignable)
	FOnAttributeChangedSignature OnHealthChanged;
	
	UPROPERTY(BlueprintAssignable)
	FOnAttributeChangedSignature OnMaxHealthChanged;

	// Die 함수 호출 시 해당 시간 이후 객체가 파괴됩니다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	float LifeSpan = 5.f;

	UPROPERTY(BlueprintReadWrite, Category = "Combat")
	TObjectPtr<AActor> CombatTarget;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Class Defaults")
	int32 Level = 1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UWidgetComponent> HealthBar;

	/**
	 * AI
	 */
	
	UPROPERTY()
	TObjectPtr<AAuraAIController> AuraAIController;

	UPROPERTY(EditAnywhere, Category = "AI")
	TObjectPtr<UBehaviorTree> BehaviorTree;

	// 해당 거리 안으로 캐릭터가 접근하면 AgroBehaviorTree가 실행됩니다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
	float AgroRange = 1000.f;

	// 해당 거리 안으로 캐릭터가 접근하면 CombatBehaviorTree가 실행됩니다.
	// 일반적으로 근접 공격 캐릭터는 150 이상, 원거리 공격 캐릭터는 600을 사용합니다.
	// 너무 낮게 설정하는 경우, 최대한 가까워져도 사정거리보다 멀어 공격하지 못 하는 현상이 있습니다.
	// 따라서 Melee Attack이라면 캐릭터의 Capsule Collision의 크기와 Melee Attack Radius를 생각해 설정해주고,
	// Range Attack이라면 Projectile의 LifeSpan을 생각해 설정해줍니다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
	float CombatRange = 150.f;
	
	// 메인 BT 안에 서브로 들어가는 Tree들입니다.
	// 기본값은 메인 BT에서 직접 할당하기 때문에, 범용 SubTree를 사용하는 경우 값을 할당하지 않습니다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
	TObjectPtr<UBehaviorTree> AgroBehaviorTree;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
	TObjectPtr<UBehaviorTree> CombatBehaviorTree;

private:
	UPROPERTY(ReplicatedUsing = OnRep_PlaySpawnAnimation)
	bool bPlaySpawnAnimation = false;
};
