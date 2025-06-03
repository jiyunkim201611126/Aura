#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "CharacterClassInfo.generated.h"

class UGameplayEffect;
class UGameplayAbility;

UENUM(BlueprintType)
enum class ECharacterClass
{
	None,
	Elementalist,
	Warrior,
	Ranger,

	EndCharacterClass
};

USTRUCT(BlueprintType)
struct FCharacterClassDefaultInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Class Defaults")
	TSubclassOf<UGameplayEffect> PrimaryAttribute;
};

UCLASS()
class AURA_API UCharacterClassInfo : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, Category = "Character Class Defaults")
	TMap<ECharacterClass, FCharacterClassDefaultInfo> CharacterClassInformation;
	
	UPROPERTY(EditAnywhere, Category = "Common Class Defaults")
	TSubclassOf<UGameplayEffect> SecondaryAttribute;
	
	UPROPERTY(EditAnywhere, Category = "Common Class Defaults")
	TSubclassOf<UGameplayEffect> VitalAttribute;

	UPROPERTY(EditAnywhere, Category = "Common Class Defaults")
	TArray<TSubclassOf<UGameplayAbility>> CommonAbilities;

	FCharacterClassDefaultInfo GetClassDefaultInfo(ECharacterClass CharacterClass);
};
