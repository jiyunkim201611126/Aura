#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Data/CharacterClassInfo.h"
#include "AuraAbilitySystemLibrary.generated.h"

class UOverlayWidgetController;
class UAttributeMenuWidgetController;
class UAbilitySystemComponent;
struct FGameplayEffectContextHandle;
struct FGameplayTag;

UCLASS()
class AURA_API UAuraAbilitySystemLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "AuraAbilitySystemLibrary | WidgetController", meta = (WorldContext = "WorldContextObject"))
	static UOverlayWidgetController* GetOverlayWidgetController(const UObject* WorldContextObject);
	
	UFUNCTION(BlueprintPure, Category = "AuraAbilitySystemLibrary | WidgetController", meta = (WorldContext = "WorldContextObject"))
	static UAttributeMenuWidgetController* GetAttributeMenuWidgetController(const UObject* WorldContextObject);

	// 적 캐릭터 직업군별로 Attribute를 초기화하는 함수
	UFUNCTION(BlueprintCallable, Category = "AuraAbilitySystemLibrary | CahracterClassDefaults", meta = (WorldContext = "WorldContextObject"))
	static void InitializeDefaultAttributes(const UObject* WorldContextObject, ECharacterClass CharacterClass, float Level, UAbilitySystemComponent* ASC);

	// 게임 시작 시 모든 캐릭터가 기본으로 사용하는 Ability(HitReact 등)를 장착시키는 함수
	UFUNCTION(BlueprintCallable, Category = "AuraAbilitySystemLibrary | CahracterClassDefaults", meta = (WorldContext = "WorldContextObject"))
	static void AddCommonAbilities(const UObject* WorldContextObject, UAbilitySystemComponent* ASC, ECharacterClass CharacterClass);
	
	UFUNCTION(BlueprintCallable, Category = "AuraAbilitySystemLibrary | CahracterClassDefaults", meta = (WorldContext = "WorldContextObject"))
	static UCharacterClassInfo* GetCharacterClassInfo(const UObject* WorldContextObject);

	// 데미지 계산 중 Block 되었는지 Context를 통해 확인하는 함수
	UFUNCTION(BlueprintPure, Category = "AuraAbilitySystemLibrary | GameplayEffects")
	static bool IsBlockedHit(const FGameplayEffectContextHandle& EffectContextHandle);
	
	// 데미지 계산 중 Critical 되었는지 Context를 통해 확인하는 함수
	UFUNCTION(BlueprintPure, Category = "AuraAbilitySystemLibrary | GameplayEffects")
	static bool IsCriticalHit(const FGameplayEffectContextHandle& EffectContextHandle);

	// 데미지 계산 중 Block 되었는지 Context에 기록하는 함수
	// UPARAM(ref)는 &가 붙은 매개변수가 output이 아니라는 걸 알려주는 매크로
	UFUNCTION(BlueprintCallable, Category = "AuraAbilitySystemLibrary | GameplayEffects")
	static void SetIsBlockedHit(UPARAM(ref) FGameplayEffectContextHandle& EffectContextHandle, bool bInIsBlockedHit);
	
	// 데미지 계산 중 Critical 되었는지 Context에 기록하는 함수
	UFUNCTION(BlueprintCallable, Category = "AuraAbilitySystemLibrary | GameplayEffects")
	static void SetIsCriticalHit(UPARAM(ref) FGameplayEffectContextHandle& EffectContextHandle, bool bInIsCriticalHit);

	// 구체 Collision과 겹쳐있는 모든 UCombatInterface 객체를 TArray<AActor*>로 뱉어주는 함수
	UFUNCTION(BlueprintCallable, Category = "AuraAbilitySystemLibrary | GameplayMechanics", meta = (WorldContext = "WorldContextObject"))
	static void GetOverlappedLivePawns(const UObject* WorldContextObject, TArray<AActor*>& OutOverlappingActors, const TArray<AActor*>& ActorsToIgnore, float Radius, const FVector& SphereOrigin);

	UFUNCTION(BlueprintPure, Category = "AuraAbilitySystemLibrary | GameplayMechanics")
	static bool IsFriend(const AActor* FirstActor, const AActor* SecondActor);

	UFUNCTION(BlueprintPure, Category = "AuraAbilitySystemLibrary | GameplayEffects")
	static TArray<AActor*> GetActorsFromContext(UPARAM(ref) FGameplayEffectContextHandle& EffectContextHandle);

	// 적용된 Duration 기반 GameplayEffect를 Tag로 추적, 남은 시간을 반환하는 함수입니다.
	UFUNCTION(BlueprintCallable, Category = "AuraAbilitySystemLibrary | GameplayEffects")
	static float GetRemainingTimeByTag(UAbilitySystemComponent* ASC, FGameplayTag Tag);
};
