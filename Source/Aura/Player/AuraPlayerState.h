#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/PlayerState.h"
#include "AuraPlayerState.generated.h"

class UAbilitySystemComponent;
class UAttributeSet;

UCLASS()
class AURA_API AAuraPlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AAuraPlayerState();
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	UAttributeSet* GetAttributeSet() const { return AttributeSet; }
	
	// Simulated Proxy와 다르게 Autonomous Proxy의 경우, 리스폰 시 유지되어야 하는 정보가 존재할 수 있다.
	// 따라서 아래 객체들을 캐릭터에 붙이지 않고 PlayerState에 붙인다.
	// Autonomous Proxy는 Owner Actor는 PlayerState, Avatar Actor가 캐릭터다.
	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY()
	TObjectPtr<UAttributeSet> AttributeSet;
};
