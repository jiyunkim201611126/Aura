#include "AuraEffectActor.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"

AAuraEffectActor::AAuraEffectActor()
{
	PrimaryActorTick.bCanEverTick = false;

	SetRootComponent(CreateDefaultSubobject<USceneComponent>("SceneRoot"));
}

void AAuraEffectActor::BeginPlay()
{
	Super::BeginPlay();
}

void AAuraEffectActor::OnOverlap(AActor* TargetActor)
{
	if (!bApplyEffectsToEnemies && TargetActor->ActorHasTag(FName("Enemy")))
	{
		return;
	}
	
	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
	if (TargetASC == nullptr) return;

	bool bHasInfinite = false;
	// GameplayEffects를 순회합니다.
	for (auto& Effect : GameplayEffects)
	{
		// Overlap할 때 Effect를 적용해야 하는 경우 들어가는 분기입니다.
		if (Effect.ApplicationPolicy == EEffectApplicationPolicy::ApplyOnOverlap)
		{
			check(Effect.GameplayEffectClass);

			// GameplayEffect가 어떻게 적용됐는지에 대한 정보를 가진 구조체 선언합니다.
			FGameplayEffectContextHandle EffectContextHandle = TargetASC->MakeEffectContext();
	
			// 어떤 객체에 의해 발생한 Effect인지 추가합니다.
			EffectContextHandle.AddSourceObject(this);
	
			// GameplayEffectSpecHandle 생성합니다.
			// Spec이란 Effect의 틀로서, 여러 곳에서 같은 효과를 적용하고 싶을 때 사용하면 좋습니다.
			FGameplayEffectSpecHandle EffectSpecHandle = TargetASC->MakeOutgoingSpec(Effect.GameplayEffectClass, ActorLevel, EffectContextHandle);

			// Spec 없이 재사용을 염두하지 않고 간편하게 사용하는 경우 ApplyGameplayEffectToSelf 호출하면 됩니다.
			// 또한 원하는 타이밍에 제거하고 싶은 경우 추적할 수 있도록 ActiveGameplayEffectSpecHandle을 캐싱해둡니다. 
			Effect.ActiveGameplayEffectHandle.Add(TargetASC, TargetASC->ApplyGameplayEffectSpecToSelf(*EffectSpecHandle.Data.Get()));

			// Infinite Effect를 하나라도 갖고 있다면 true로 변경
			if (!bHasInfinite && EffectSpecHandle.Data.Get())
			{
				bHasInfinite = EffectSpecHandle.Data.Get()->Def.Get()->DurationPolicy == EGameplayEffectDurationType::Infinite;
			}
		}
	}

	if (bDestroyOnEffectApplication && !bHasInfinite)
	{
		Destroy();
	}
}

void AAuraEffectActor::OnEndOverlap(AActor* TargetActor)
{
	if (!bApplyEffectsToEnemies && TargetActor->ActorHasTag(FName("Enemy")))
	{
		return;
	}
	
	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
	if (TargetASC == nullptr) return;
	
	bool bHasInfinite = false;
	for (auto& Effect : GameplayEffects)
	{
		// EndOverlap할 때 Effect를 제거해야 하는 경우
		if (Effect.RemovalPolicy == EEffectRemovalPolicy::RemoveOnEndOverlap)
		{
			check(Effect.GameplayEffectClass);

			// OnOverlap에서 추적할 수 있도록 할당해뒀으므로 제거 가능
			TargetASC->RemoveActiveGameplayEffect(Effect.ActiveGameplayEffectHandle.FindAndRemoveChecked(TargetASC), 1);
		}
		
		FGameplayEffectSpecHandle EffectSpecHandle;
		// EndOverlap할 때 Effect를 적용해야 하는 경우
		if (Effect.ApplicationPolicy == EEffectApplicationPolicy::ApplyOnEndOverlap)
		{
			check(Effect.GameplayEffectClass);
			FGameplayEffectContextHandle EffectContextHandle = TargetASC->MakeEffectContext();
			EffectContextHandle.AddSourceObject(this);
			EffectSpecHandle = TargetASC->MakeOutgoingSpec(Effect.GameplayEffectClass, ActorLevel, EffectContextHandle);
			Effect.ActiveGameplayEffectHandle.Add(TargetASC, TargetASC->ApplyGameplayEffectSpecToSelf(*EffectSpecHandle.Data.Get()));
		}
		
		if (!bHasInfinite && EffectSpecHandle.Data.Get())
		{
			bHasInfinite = EffectSpecHandle.Data.Get()->Def.Get()->DurationPolicy == EGameplayEffectDurationType::Infinite;
		}
	}

	if (bDestroyOnEffectApplication && !bHasInfinite)
	{
		Destroy();
	}
}
