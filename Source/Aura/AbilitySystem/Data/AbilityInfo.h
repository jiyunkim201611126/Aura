#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "AbilityInfo.generated.h"

class UGameplayAbility;

/**
 * GameplayAbility의 스킬 아이콘 표시를 위해 Tag와 에셋을 매핑하는 구조체입니다.
 * DataAsset에서 초기화하며, 스킬 아이콘이 필요한 모든 Ability가 여기에 추가됩니다.
 * 양이 많아져 탐색 속도가 걱정된다면 추후 '캐릭터 직업' 클래스마다 따로 TArray를 선언해 개선할 수 있습니다.
 * 예) 캐릭터 직업 Tag 선언 후, FindAbilityInfoForTag에서 매개변수로 함께 받아 탐색
 * 캐릭터 직업을 Tag로 선언한다면, enum과 달리 '다중 직업'이나 '타 직업 스킬 사용 시 패널티' 등의 구현에도 용이합니다.
 * 
 * 단, InputTag의 경우 런타임 중 변화하므로 DataAsset에서 초기화하지 않습니다.
 */

USTRUCT(BlueprintType)
struct FAuraAbilityInfo
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag AbilityTag = FGameplayTag();

	UPROPERTY(BlueprintReadOnly)
	FGameplayTag InputTag = FGameplayTag();

	UPROPERTY(BlueprintReadOnly)
	FGameplayTag StatusTag = FGameplayTag();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag CooldownTag = FGameplayTag();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag AbilityType = FGameplayTag();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UTexture2D> AbilityIcon = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UMaterialInterface> BackgroundMaterial = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 LevelRequirement = 1;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 MaxLevel = 10;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UGameplayAbility> Ability;
};

UCLASS()
class AURA_API UAbilityInfo : public UDataAsset
{
	GENERATED_BODY()

public:
	FAuraAbilityInfo FindAbilityInfoForTag(const FGameplayTag& AbilityTag, bool bLogNotFound = false) const;
	
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AbilityInformation")
	TArray<FAuraAbilityInfo> AbilityInformation;
};
