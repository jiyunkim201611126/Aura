#pragma once

#include "CoreMinimal.h"
#include "AuraAttributeSet.h"
#include "Aura/AuraAbilityTypes.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Data/CharacterClassInfo.h"
#include "AuraAbilitySystemLibrary.generated.h"

class AAuraHUD;
class UOverlayWidgetController;
class UAttributeMenuWidgetController;
class USpellMenuWidgetController;
class UAbilitySystemComponent;
class UAbilityInfo;
struct FWidgetControllerParams;
struct FGameplayEffectContextHandle;
struct FGameplayTag;

UCLASS()
class AURA_API UAuraAbilitySystemLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "AuraAbilitySystemLibrary | WidgetController", meta = (WorldContext = "WorldContextObject"))
	static bool MakeWidgetControllerParams(const UObject* WorldContextObject, FWidgetControllerParams& OutWidgetControllerParams, AAuraHUD*& OutAuraHUD);
	
	UFUNCTION(BlueprintPure, Category = "AuraAbilitySystemLibrary | WidgetController", meta = (WorldContext = "WorldContextObject"))
	static UOverlayWidgetController* GetOverlayWidgetController(const UObject* WorldContextObject);
	
	UFUNCTION(BlueprintPure, Category = "AuraAbilitySystemLibrary | WidgetController", meta = (WorldContext = "WorldContextObject"))
	static UAttributeMenuWidgetController* GetAttributeMenuWidgetController(const UObject* WorldContextObject);
	
	UFUNCTION(BlueprintPure, Category = "AuraAbilitySystemLibrary | WidgetController", meta = (WorldContext = "WorldContextObject"))
	static USpellMenuWidgetController* GetSpellMenuWidgetController(const UObject* WorldContextObject);

	// 적 캐릭터 직업군별로 Attribute를 초기화하는 함수
	UFUNCTION(BlueprintCallable, Category = "AuraAbilitySystemLibrary | CahracterClassDefaults", meta = (WorldContext = "WorldContextObject"))
	static void InitializeDefaultAttributes(const UObject* WorldContextObject, ECharacterClass CharacterClass, float Level, UAbilitySystemComponent* ASC);

	// 게임 시작 시 모든 캐릭터가 기본으로 사용하는 Ability(HitReact 등)를 장착시키는 함수
	UFUNCTION(BlueprintCallable, Category = "AuraAbilitySystemLibrary | CahracterClassDefaults", meta = (WorldContext = "WorldContextObject"))
	static void AddCommonAbilities(const UObject* WorldContextObject, UAbilitySystemComponent* ASC, ECharacterClass CharacterClass);
	
	UFUNCTION(BlueprintCallable, Category = "AuraAbilitySystemLibrary | CahracterClassDefaults", meta = (WorldContext = "WorldContextObject"))
	static UCharacterClassInfo* GetCharacterClassInfo(const UObject* WorldContextObject);
	
	UFUNCTION(BlueprintCallable, Category = "AuraAbilitySystemLibrary | CahracterClassDefaults", meta = (WorldContext = "WorldContextObject"))
	static UAbilityInfo* GetAbilityInfo(const UObject* WorldContextObject);

	UFUNCTION(BlueprintPure, Category = "AuraAbilitySystemLibrary | GameplayEffects")
	static FDamageDataContext GetDamageData(const FGameplayEffectContextHandle& EffectContextHandle);

	UFUNCTION(BlueprintPure, Category = "AuraAbilitySystemLibrary | GameplayEffects")
	static TArray<FVector_NetQuantize> GetLocationsFromContext(const FGameplayEffectContextHandle& EffectContextHandle);
	
	static void SetDamageDataContext(FGameplayEffectContextHandle& EffectContextHandle, const EDamageTypeContext DamageType, bool bIsBlocked, bool bIsCritical);
	static void SetDeathImpulse(FGameplayEffectContextHandle& EffectContextHandle, const FVector& Impulse);
	static void SetKnockbackForce(FGameplayEffectContextHandle& EffectContextHandle, const FVector& Force);
	static void SetLocationsToContext(FGameplayEffectContextHandle& EffectContextHandle, const TArray<FVector_NetQuantize>& InLocations);

	static EDamageTypeContext ReplaceDamageTypeToEnum(const FGameplayTag& DamageTypeTag);
	static FGameplayTag ReplaceDamageTypeToTag(const EDamageTypeContext DamageTypeEnum);
	static EDebuffTypeContext ReplaceDebuffTypeToEnum(const FGameplayTag& DebuffTypeTag);
	static FGameplayTag ReplaceDebuffTypeToTag(const EDebuffTypeContext DebuffTypeEnum);

	static TArray<FVector> EvenlyRotatedVectors(const FVector& Forward, const FVector& Axis, const float Spread, const int32 NumOfVectors);
	static TArray<FRotator> EvenlySpacedRotators(const FVector& Forward, const FVector& Axis, const float Spread, const int32 NumOfRotators);

	// 구체 Collision과 겹쳐있는 모든 UCombatInterface 객체를 TArray<AActor*>로 반환해주는 함수
	UFUNCTION(BlueprintCallable, Category = "AuraAbilitySystemLibrary | GameplayMechanics", meta = (WorldContext = "WorldContextObject"))
	static void GetOverlappedLivePawnsWithinRadius(const UObject* WorldContextObject, TArray<AActor*>& OutOverlappingActors, const TArray<AActor*>& ActorsToIgnore, float Radius, const FVector& SphereOrigin);

	UFUNCTION(BlueprintCallable, Category = "AuraAbilitySystemLibrary | GameplayMechanics")
	static void GetClosestTargets(int32 MaxTargets, const TArray<AActor*>& Actors, TArray<AActor*>& OutClosestTargets, const FVector& Origin);

	UFUNCTION(BlueprintPure, Category = "AuraAbilitySystemLibrary | GameplayMechanics")
	static bool IsFriend(const AActor* FirstActor, const AActor* SecondActor);

	UFUNCTION(BlueprintPure, Category = "AuraAbilitySystemLibrary | GameplayEffects")
	static TArray<AActor*> GetActorsFromContext(UPARAM(ref) FGameplayEffectContextHandle& EffectContextHandle);

	// 적용된 Duration 기반 GameplayEffect를 Tag로 추적, 남은 시간을 반환하는 함수입니다.
	UFUNCTION(BlueprintCallable, Category = "AuraAbilitySystemLibrary | GameplayEffects")
	static float GetRemainingTimeByTag(UAbilitySystemComponent* ASC, FGameplayTag Tag);

	UFUNCTION(BlueprintCallable, Category = "AuraAbilitySystemLibrary | GameplayEffects", meta = (WorldContext = "WorldContextObject"))
	static int32 GetXPRewardForRankAndLevel(const UObject* WorldContextObject, ECharacterRank CharacterRank, int32 CharacterLevel);
};
