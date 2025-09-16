#pragma once

#include "CoreMinimal.h"
#include "AuraCharacterBase.h"
#include "Aura/Interaction/LevelableInterface.h"
#include "AuraCharacter.generated.h"

class UNiagaraComponent;

UCLASS()
class AURA_API AAuraCharacter : public AAuraCharacterBase, public ILevelableInterface
{
	GENERATED_BODY()

public:
	AAuraCharacter();
	
	virtual void OnRep_PlayerState() override;

	//~ Begin Combat Interface
	virtual void RegisterPawn() override;
	virtual void UnregisterPawn() override;
	virtual int32 GetCharacterLevel_Implementation() override;
	//~ End Combat Interface
	
	//~ Begin Levelable Interface
	virtual int32 FindLevelForXP_Implementation(int32 InXP) const override;
	virtual int32 GetXP_Implementation() const override;
	virtual int32 GetAttributePointsReward_Implementation(int32 Level) const override;
	virtual int32 GetSpellPointsReward_Implementation(int32 Level) const override;
	virtual void AddToXP_Implementation(const int32 InXP) override;
	virtual void AddToLevel_Implementation(const int32 InPlayerLevel) override;
	virtual void AddToAttributePoints_Implementation(const int32 InAttributePoints) override;
	virtual int32 GetAttributePoints_Implementation() const override;
	virtual void AddToSpellPoints_Implementation(const int32 InSpellPoints) override;
	virtual int32 GetSpellPoints_Implementation() const override;
	virtual void LevelUp_Implementation() override;
	//~ End Levelable Interface

protected:
	//~ Begin Actor Interface
	virtual void BeginPlay() override;
	//~ End Actor Interface

	//~ Begin Pawn Interface
	virtual void PossessedBy(AController* NewController) override;
	//~ End Pawn Interface
	
	// 게임 시작 시 Attribute를 초기화하는 함수
	void InitializeDefaultAttributes() const;

private:
	virtual void InitAbilityActorInfo() override;

	UFUNCTION(NetMulticast, Reliable)
	void MulticastLevelUpParticles() const;

protected:
	UPROPERTY(EditDefaultsOnly)
	FGameplayTag LevelUpNiagaraTag;
	
	// 게임 시작 시 Attribute 초기화를 위해 사용되는 GameplayEffect
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Attributes")
	TSubclassOf<UGameplayEffect> DefaultPrimaryAttributes;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Attributes")
	TSubclassOf<UGameplayEffect> DefaultSecondaryAttributes;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Attributes")
	TSubclassOf<UGameplayEffect> DefaultVitalAttributes;
};
