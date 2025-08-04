#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "Aura/AbilitySystem/Data/LevelUpInfo.h"
#include "GameFramework/PlayerState.h"
#include "AuraPlayerState.generated.h"

class UAbilitySystemComponent;
class UAttributeSet;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnPlayerStatChanged, int32 /* Stat Value */);

UCLASS()
class AURA_API AAuraPlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AAuraPlayerState();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	UAttributeSet* GetAttributeSet() const { return AttributeSet; }

	FORCEINLINE int32 GetPlayerLevel() const { return Level; }
	FORCEINLINE int32 GetXP() const { return XP; }
	
	void SetPlayerLevel(int32 InLevel);
	void SetXP(const int32 InXP);

	void AddToLevel(int32 InLevel);
	void AddToXP(const int32 InXP);

public:
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<ULevelUpInfo> LevelUpInfo;

	FOnPlayerStatChanged OnXPChangedDelegate;
	FOnPlayerStatChanged OnLevelChangedDelegate;

protected:
	// Simulated Proxy와 다르게 Autonomous Proxy의 경우, 리스폰 시 유지되어야 하는 정보가 존재할 수 있습니다.
	// 따라서 아래 객체들을 캐릭터에 붙이지 않고 PlayerState에 붙입니다.
	// 즉, Autonomous Proxy는 Owner Actor는 PlayerState, Avatar Actor가 캐릭터입니다.
	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY()
	TObjectPtr<UAttributeSet> AttributeSet;

private:
	// PlayerState -> WidgetController -> Widget 로 로직 흐름을 구성했습니다.
	// 위 흐름은 델리게이트를 통해 제어됩니다.
	// 1. WidgetController가 PlayerState의 델리게이트에 콜백 바인드
	// 2. Widget이 WidgetController의 델리게이트에 콜백 바인드
	// 3. PlayerState에서 XP 변경 시 이 순서를 통해 전파됩니다.
	
	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_Level)
	int32 Level = 1;

	UFUNCTION()
	void OnRep_Level(int32 OldLevel);

	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_XP)
	int32 XP = 0;

	UFUNCTION()
	void OnRep_XP(int32 OldXP);
};
