#pragma once

#include "CoreMinimal.h"
#include "AuraCharacterBase.h"
#include "Aura/Interaction/LevelableInterface.h"
#include "Aura/Interaction/SaveGameInterface.h"
#include "AuraCharacter.generated.h"

class UNiagaraComponent;

UCLASS()
class AURA_API AAuraCharacter : public AAuraCharacterBase, public ILevelableInterface, public ISaveGameInterface
{
	GENERATED_BODY()

public:
	AAuraCharacter();
	
	virtual void OnRep_PlayerState() override;

	//~ Begin ICombat Interface
	virtual void RegisterPawn() override;
	virtual void UnregisterPawn() override;
	virtual int32 GetCharacterLevel_Implementation() override;
	//~ End ICombat Interface
	
	//~ Begin ILevelable Interface
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
	//~ End ILevelable Interface

	//~ Begin ISaveGame Interface
	virtual void SaveProgress_Implementation(const FName& CheckpointTag) override;
	//~ End of ISaveGame Interface

protected:
	//~ Begin AActor Interface
	virtual void BeginPlay() override;
	//~ End AActor Interface

	//~ Begin APawn Interface
	virtual void PossessedBy(AController* NewController) override;
	//~ End APawn Interface

	void LoadProgress();
	
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
