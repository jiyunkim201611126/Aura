#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "AuraWidgetController.generated.h"

struct FGameplayAbilitySpec;
class UAbilityInfo;
class UAbilitySystemComponent;
class UAttributeSet;
class AAuraPlayerController;
class AAuraPlayerState;
class UAuraAbilitySystemComponent;
class UAuraAttributeSet;

// 위젯 컨트롤러의 멤버 변수 초기화를 간편화하는 구조체
USTRUCT(BlueprintType)
struct FWidgetControllerParams
{
	GENERATED_BODY()

	FWidgetControllerParams() {}
	FWidgetControllerParams(APlayerController* PC, APlayerState* PS, UAbilitySystemComponent* ASC, UAttributeSet* AS)
	: PlayerController(PC), PlayerState(PS), AbilitySystemComponent(ASC), AttributeSet(AS) {}

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<APlayerController> PlayerController = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<APlayerState> PlayerState = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UAttributeSet> AttributeSet = nullptr;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerStatChangedSignature, int32, NewValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAbilityInfoSignature, const FAuraAbilityInfo&, Info);

/**
 * MVVM 패턴의 VM을 담당하는 Widget Controller입니다.
 * MVVM 패턴은 UI를 관리하는 패턴 중 GAS 기반의 프로젝트와 가장 잘 어울리는 패턴이라는 게 정설입니다.
 * HUD를 포함해 UI와 관련된 모든 객체(ASC, AttributeSet, PlayerController 등)이 모두 생성되었다고 판단되는 순간, HUD에 의해 생성되는 객체입니다.
 * 생성 이후 즉시 관련 객체들을 할당받으며, 그 객체들에게 콜백 함수를 바인드합니다.
 * 바인드를 마치면 모든 관련 Widget들에게 뿌려지게 되며, Widget들은 이를 변수로 할당하고 마찬가지로 콜백 함수를 바인드합니다.
 *
 * 완료 후 로직 흐름은 이렇습니다.
 * UI에 반영되어야 하는 변경 사항 발생(Ability 사용, Attribute 변경 등) -> Widget Controller의 콜백 함수 호출 -> Widget의 콜백 함수 호출 -> 반영 완료
 */

UCLASS()
class AURA_API UAuraWidgetController : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void SetWidgetControllerParams(const FWidgetControllerParams& WidgetControllerParams);
	UFUNCTION(BlueprintCallable)
	virtual void BroadcastInitialValue();

	virtual void OnAbilitiesGiven(const FGameplayAbilitySpec& AbilitySpec);

	// AttributeSet에 있는 변수의 값이 변할 때마다 호출되는 델리게이트에 자신의 함수를 바인드하는 함수
	virtual void BindCallbacksToDependencies();

	AAuraPlayerController* GetAuraPC();
	AAuraPlayerState* GetAuraPS();
	UAuraAbilitySystemComponent* GetAuraASC();
	UAuraAttributeSet* GetAuraAS();

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Widget Data")
	TObjectPtr<UAbilityInfo> AbilityInfo;
	
	UPROPERTY(BlueprintAssignable, Category = "GAS | AbilityInfo")
	FAbilityInfoSignature AbilityInfoDelegate;

protected:
	UPROPERTY(BlueprintReadOnly, Category = "WidgetController")
	TObjectPtr<APlayerController> PlayerController;

	UPROPERTY(BlueprintReadOnly, Category = "WidgetController")
	TObjectPtr<APlayerState> PlayerState;

	UPROPERTY(BlueprintReadOnly, Category = "WidgetController")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(BlueprintReadOnly, Category = "WidgetController")
	TObjectPtr<UAttributeSet> AttributeSet;

	UPROPERTY(BlueprintReadOnly, Category = "WidgetController")
	TObjectPtr<AAuraPlayerController> AuraPlayerController;

	UPROPERTY(BlueprintReadOnly, Category = "WidgetController")
	TObjectPtr<AAuraPlayerState> AuraPlayerState;

	UPROPERTY(BlueprintReadOnly, Category = "WidgetController")
	TObjectPtr<UAuraAbilitySystemComponent> AuraAbilitySystemComponent;

	UPROPERTY(BlueprintReadOnly, Category = "WidgetController")
	TObjectPtr<UAuraAttributeSet> AuraAttributeSet;
};
