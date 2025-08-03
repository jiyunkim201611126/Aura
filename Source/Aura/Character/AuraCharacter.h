#pragma once

#include "CoreMinimal.h"
#include "AuraCharacterBase.h"
#include "Aura/Interaction/PlayerInterface.h"
#include "AuraCharacter.generated.h"

UCLASS()
class AURA_API AAuraCharacter : public AAuraCharacterBase, public IPlayerInterface
{
	GENERATED_BODY()

public:
	AAuraCharacter();
	
	virtual void OnRep_PlayerState() override;

	// ~Combat Interface
	virtual void RegisterPawn() override;
	virtual void UnregisterPawn() override;
	virtual int32 GetPlayerLevel_Implementation() override;
	// ~End of Combat Interface
	
	// ~Player Interface
	virtual void AddToXP_Implementation(const int32 InXP) override;
	virtual void LevelUp_Implementation() override;
	// ~End of Player Interface

protected:
	// ~AActor Interface
	virtual void BeginPlay() override;
	// ~End of AActor Interface

	// ~APawn Interface
	virtual void PossessedBy(AController* NewController) override;
	// ~End of APawn Interface
	
	// 게임 시작 시 Attribute를 초기화하는 함수
	void InitializeDefaultAttributes() const;

private:
	virtual void InitAbilityActorInfo() override;

protected:
	// 게임 시작 시 Attribute 초기화를 위해 사용되는 GameplayEffect
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Attributes")
	TSubclassOf<UGameplayEffect> DefaultPrimaryAttributes;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Attributes")
	TSubclassOf<UGameplayEffect> DefaultSecondaryAttributes;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Attributes")
	TSubclassOf<UGameplayEffect> DefaultVitalAttributes;
};
