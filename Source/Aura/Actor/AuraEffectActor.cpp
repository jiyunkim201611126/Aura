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

void AAuraEffectActor::ApplyEffectToTarget(AActor* TargetActor, TSubclassOf<UGameplayEffect> GameplayEffectClass)
{
	// FunctionLibrary를 통해 Target의 AbilitySystemComponent 가져오기
	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
	if (TargetASC == nullptr) return;
	
	check(GameplayEffectClass);
	
	// GameplayEffect가 어떻게 적용됐는지에 대한 정보를 가진 구조체 선언
	// 누가 때렸는지, 누가 맞았는지, 어떤 속성의 공격인지, 데미지는 몇인지 등 로그 같은 개념
	FGameplayEffectContextHandle EffectContextHandle = TargetASC->MakeEffectContext();
	
	// 어떤 객체에 의해 발생한 Effect인지 추가
	EffectContextHandle.AddSourceObject(this);
	
	// GameplayEffectSpecHandle 생성, Spec이란 Effect의 틀로서, 여러 곳에서 같은 효과를 적용하고 싶을 때 사용하면 좋음
	// Spec 없이 재사용을 염두하지 않고 간편하게 사용하는 경우 ApplyGameplayEffectToSelf 호출
	FGameplayEffectSpecHandle EffectSpecHandle = TargetASC->MakeOutgoingSpec(GameplayEffectClass, 1.f, EffectContextHandle);
	TargetASC->ApplyGameplayEffectSpecToSelf(*EffectSpecHandle.Data.Get());
}
