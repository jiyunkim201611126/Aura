#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "AuraInputConfig.generated.h"

class UInputAction;

/**
 * InputAction을 태그와 묶어놓은 구조체입니다.
 * 함수를 통해 매개변수로 Tag를 받아 InputAction을 반환하도록 구성되어있습니다.
 */

USTRUCT(BlueprintType)
struct FAuraInputAction
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly)
	const UInputAction* InputAction = nullptr;

	UPROPERTY(EditDefaultsOnly)
	FGameplayTag InputTag = FGameplayTag();
};

UCLASS()
class AURA_API UAuraInputConfig : public UDataAsset
{
	GENERATED_BODY()

public:
	// Tag를 매개변수로 받아 해당하는 InputAction을 아래 구조체 배열에서 탐색, 반환합니다.
	const UInputAction* FindAbilityInputActionForTag(const FGameplayTag& InputTag, bool bLogNotFount);

	// 블루프린트로 확장한 DataAsset에서 이 배열의 값을 초기화합니다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<FAuraInputAction> AbilityInputActions;
};
