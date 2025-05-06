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
	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
	if (TargetASC == nullptr) return;

	// GameplayEffects를 순회
	for (auto& Effect : GameplayEffects)
	{
		// Overlap할 때 Effect를 적용해야 하는 경우
		if (Effect.ApplicationPolicy == EEffectApplicationPolicy::ApplyOnOverlap)
		{
			check(Effect.GameplayEffectClass);

			// GameplayEffect가 어떻게 적용됐는지에 대한 정보를 가진 구조체 선언
			// 누가 때렸는지, 누가 맞았는지, 어떤 속성의 공격인지, 데미지는 몇인지 등 로그 같은 개념
			FGameplayEffectContextHandle EffectContextHandle = TargetASC->MakeEffectContext();
	
			// 어떤 객체에 의해 발생한 Effect인지 추가
			EffectContextHandle.AddSourceObject(this);
	
			// GameplayEffectSpecHandle 생성, Spec이란 Effect의 틀로서, 여러 곳에서 같은 효과를 적용하고 싶을 때 사용하면 좋음
			FGameplayEffectSpecHandle EffectSpecHandle = TargetASC->MakeOutgoingSpec(Effect.GameplayEffectClass, 1.f, EffectContextHandle);

			// Spec 없이 재사용을 염두하지 않고 간편하게 사용하는 경우 ApplyGameplayEffectToSelf 호출
			// 또한 원하는 타이밍에 제거하고 싶은 경우 추적할 수 있도록 구조체 내에 할당
			Effect.ActiveGameplayEffectHandle = TargetASC->ApplyGameplayEffectSpecToSelf(*EffectSpecHandle.Data.Get());
		}
	}
}

void AAuraEffectActor::OnEndOverlap(AActor* TargetActor)
{
	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
	if (TargetASC == nullptr) return;
	
	for (auto& Effect : GameplayEffects)
	{
		// EndOverlap할 때 Effect를 적용해야 하는 경우
		if (Effect.ApplicationPolicy == EEffectApplicationPolicy::ApplyOnEndOverlap)
		{
			check(Effect.GameplayEffectClass);
			FGameplayEffectContextHandle EffectContextHandle = TargetASC->MakeEffectContext();
			EffectContextHandle.AddSourceObject(this);
			FGameplayEffectSpecHandle EffectSpecHandle = TargetASC->MakeOutgoingSpec(Effect.GameplayEffectClass, 1.f, EffectContextHandle);
			Effect.ActiveGameplayEffectHandle = TargetASC->ApplyGameplayEffectSpecToSelf(*EffectSpecHandle.Data.Get());
		}
		
		if (Effect.RemovalPolicy == EEffectRemovalPolicy::RemoveOnEndOverlap)
		{
			check(Effect.GameplayEffectClass);

			// OnOverlap에서 추적할 수 있도록 할당해뒀으므로 제거 가능
			TargetASC->RemoveActiveGameplayEffect(Effect.ActiveGameplayEffectHandle, 1);
		}
	}
}
