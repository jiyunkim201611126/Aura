#pragma once

#include "CoreMinimal.h"
#include "AuraCharacterBase.h"
#include "Aura/Interaction/EnemyInterface.h"
#include "Aura/UI/WidgetController/OverlayWidgetController.h"
#include "Aura/AbilitySystem/Data/CharacterClassInfo.h"
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
	virtual void PossessedBy(AController* NewController) override;

	/** Enemy Interface */
	virtual void HighlightActor() override;
	virtual void UnHighlightActor() override;
	virtual void SetCombatTarget_Implementation(AActor* InCombatTarget) override;
	virtual AActor* GetCombatTarget_Implementation() const override;
	/** end Enemy Interface */

	/** Combat Interface */
	virtual void RegisterPawn() override;
	virtual void UnregisterPawn() override;
	virtual int32 GetPlayerLevel() override;
	virtual void Die(bool bShouldAddImpulse, const FVector& Impulse) override;
	/** end Combat Interface */

	virtual void MulticastHandleDeath_Implementation(bool bShouldAddImpulse, const FVector& Impulse) override;

	// 아래 2개의 델리게이트 선언으로 인해 이 클래스가 OverlayWidgetController를 참조하게 되었으나,
	// 여기서 같은 델리게이트를 선언하거나 WidgetComponent를 위한 또 다른 WidgetController 클래스를 만드는 것보다 이게 더 단순함
	// 물론 이 델리게이트의 선언부만 따로 클래스로 뽑아낼 수도 있겠지만, 당장은 이렇게 구현
	UPROPERTY(BlueprintAssignable)
	FOnAttributeChangedSignature OnHealthChanged;
	
	UPROPERTY(BlueprintAssignable)
	FOnAttributeChangedSignature OnMaxHealthChanged;
	
	void HitReactTagChanged(const FGameplayTag CallbackTag, int32 NewCount);

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	bool bHitReacting = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	float BaseWalkSpeed = 250.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	float LifeSpan = 5.f;

	UPROPERTY(BlueprintReadWrite, Category = "Combat")
	TObjectPtr<AActor> CombatTarget;

protected:
	virtual void BeginPlay() override;
	virtual void InitAbilityActorInfo() override;
	virtual void AddCharacterStartupAbilities() const override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Class Defaults")
	int32 Level = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Class Defaults")
	ECharacterClass CharacterClass = ECharacterClass::Elementalist;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UWidgetComponent> HealthBar;

	UPROPERTY()
	TObjectPtr<AAuraAIController> AuraAIController;

	UPROPERTY(EditAnywhere, Category = "AI")
	TObjectPtr<UBehaviorTree> BehaviorTree;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
	float AgroRange = 1000.f;

	// 일반적으로 근접 공격 캐릭터는 150 이상, 원거리 공격 캐릭터는 600을 사용합니다.
	// 너무 낮게 설정하는 경우, 최대한 가까워져도 사정거리보다 멀어 공격하지 못 하는 현상이 있습니다.
	// 따라서 캐릭터의 Capsule Collision의 크기를 생각해 설정해줍니다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
	float CombatRange = 150.f;
	
	// 메인 BT 안에 서브로 들어가는 Tree들입니다.
	// 기본값은 메인 BT에서 직접 할당하기 때문에, 범용 SubTree를 사용하는 경우 값을 할당하지 않습니다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
	TObjectPtr<UBehaviorTree> AgroBehaviorTree;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
	TObjectPtr<UBehaviorTree> CombatBehaviorTree;
};
