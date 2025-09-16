#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayEffectTypes.h"
#include "AuraEffectActor.generated.h"

class UGameplayEffect;
class UAbilitySystemComponent;

// 트리거 작동 시 Effect 적용을 어떻게 처리할 것인지에 대한 enum 
UENUM(BlueprintType)
enum class EEffectApplicationPolicy : uint8
{
	ApplyOnOverlap,
	ApplyOnEndOverlap,
	DoNotApply
};

// Infinite 적용 상태인 경우 언제 Effect를 제거할 것인지에 대한 enum
UENUM(BlueprintType)
enum class EEffectRemovalPolicy : uint8
{
	RemoveOnEndOverlap,
	DoNotRemove
};

USTRUCT(BlueprintType)
struct FGameplayEffectInfo
{
	GENERATED_BODY()

	FGameplayEffectInfo(){}

	// 적용할 Effect
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<UGameplayEffect> GameplayEffectClass;

	// 적용 시점
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EEffectApplicationPolicy ApplicationPolicy = EEffectApplicationPolicy::DoNotApply;

	// 제거 시점
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EEffectRemovalPolicy RemovalPolicy = EEffectRemovalPolicy::RemoveOnEndOverlap;

	// 제거가 필요한 경우 추적을 위해 사용
	UPROPERTY()
	TMap<UAbilitySystemComponent*, FActiveGameplayEffectHandle> ActiveGameplayEffectHandle;
};

/**
 * GameplayEffect만 처리하는 액터 클래스
 * StaticMesh나 SphereComponent 등은 블루프린트에서 추가하는 편이 더 확장성 좋음
 * 따라서 이벤트 트리거 등도 블루프린트의 이벤트 그래프에서 처리
 */

UCLASS()
class AURA_API AAuraEffectActor : public AActor
{
	GENERATED_BODY()

public:
	AAuraEffectActor();
	
protected:
	//~ Begin Actor Interface
	virtual void BeginPlay() override;
	//~ End Actor Interface

	UFUNCTION(BlueprintCallable)
	void OnOverlap(AActor* TargetActor);
	
	UFUNCTION(BlueprintCallable)
	void OnEndOverlap(AActor* TargetActor);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Applied Effects")
	bool bDestroyOnEffectApplication = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Applied Effects")
	bool bApplyEffectsToEnemies = false;

	// 해당 클래스를 상속받는 자손 블루프린트 액터의 디테일 패널에서 Element를 추가
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Applied Effects")
	TArray<FGameplayEffectInfo> GameplayEffects;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Applied Effects")
	float ActorLevel = 1.f;
};
