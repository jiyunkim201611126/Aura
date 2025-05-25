#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "AttributeInfo.generated.h"

/**
 * Gameplay Attribute에 대한 정보를 캡슐화한 구조체입니다.
 * 태그, 이름, 설명, 현재 값이 변수로 선언되어있으며, DataAsset에서 초기화합니다.
 * 단, Value의 경우 런타임 중 계속해서 변화하므로 DataAsset에서 초기화하지 않습니다.
 * Tag를 통해 Attribute의 값을 찾을 때 이 인스턴스에게서 Name, Description을 가져오게 됩니다.
 */

USTRUCT(BlueprintType)
struct FAuraAttributeInfo
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag AttributeTag = FGameplayTag();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FText AttributeName = FText();
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FText AttributeDescription = FText();

	UPROPERTY(BlueprintReadOnly)
	float AttributeValue = 0.f;
};

UCLASS()
class AURA_API UAttributeInfo : public UDataAsset
{
	GENERATED_BODY()

public:
	// Tag를 매개변수로 받고, 아래 배열에서 해당되는 AttributeInfo를 찾아 반환
	FAuraAttributeInfo FindAttributeInfoForTag(const FGameplayTag& AttributeTag, bool bLogNotFound = false) const;

	// 해당 클래스로부터 파생된 DataAsset에 위에서 선언된 구조체를 배열로 추가하여 초기화할 수 있음
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<FAuraAttributeInfo> AttributeInformation;
};
