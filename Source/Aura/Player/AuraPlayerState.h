#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "Aura/AbilitySystem/Data/LevelUpInfo.h"
#include "GameFramework/PlayerState.h"
#include "AuraPlayerState.generated.h"

class UAbilitySystemComponent;
class UAttributeSet;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnPlayerStatChanged, int32 /* StatValue */);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnLevelChanged, int32 /* NewLevel */, bool /* bLevelUp */);

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
	FORCEINLINE int32 GetAttributePoints() const { return AttributePoints; }
	FORCEINLINE int32 GetSpellPoints() const { return SpellPoints; }
	
	void SetPlayerLevel(int32 InLevel);
	void SetXP(const int32 InXP);
	void SetAttributePoints(const int32 InAttributePoints);
	void SetSpellPoints(const int32 InSpellPoints);

	void AddToLevel(int32 InLevel);
	void AddToXP(const int32 InXP);
	void AddToAttributePoints(const int32 InAttributePoints);
	void AddToSpellPoints(const int32 InSpellPoints);

public:
	/**
	 * WidgetController 클래스가 콜백을 걸어두는 델리게이트입니다.
	 * Set 함수 혹은 Add 함수를 통해 값이 변경되면 리슨 서버와 클라이언트 모두 해당 델리게이트로 콜백을 호출합니다.
	 */
	FOnPlayerStatChanged OnXPChangedDelegate;
	FOnLevelChanged OnLevelChangedDelegate;
	FOnPlayerStatChanged OnAttributePointsChangedDelegate;
	FOnPlayerStatChanged OnSpellPointsChangedDelegate;

public:
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<ULevelUpInfo> LevelUpInfo;

protected:
	// Simulated Proxy와 다르게 Autonomous Proxy의 경우, 리스폰 시 유지되어야 하는 정보가 존재할 수 있습니다.
	// 따라서 아래 객체들을 캐릭터에 붙이지 않고 PlayerState에 붙입니다.
	// 즉, Autonomous Proxy는 Owner Actor는 PlayerState, Avatar Actor가 캐릭터입니다.
	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY()
	TObjectPtr<UAttributeSet> AttributeSet;

private:
	// Level의 초기값을 0으로 설정해, Level의 값 변동 시 게임 시작 시점인지 레벨업 시점인지 클라이언트도 파악할 수 있도록 합니다.
	// 예시) [0 -> 4]: Level 4 플레이어 접속 / [4 -> 5]: Level 4 플레이어 레벨업
	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_Level)
	int32 Level = 0;

	UFUNCTION()
	void OnRep_Level(int32 OldLevel);

	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_XP)
	int32 XP = 0;

	UFUNCTION()
	void OnRep_XP(int32 OldXP);

	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_AttributePoints)
	int32 AttributePoints = 0;

	UFUNCTION()
	void OnRep_AttributePoints(int32 OldAttributePoints);

	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_SpellPoints)
	int32 SpellPoints = 0;

	UFUNCTION()
	void OnRep_SpellPoints(int32 OldSpellPoints);
};
