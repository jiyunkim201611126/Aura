#include "AuraAssetManager.h"
#include "AuraGameplayTags.h"
#include "AbilitySystemGlobals.h"

UAuraAssetManager& UAuraAssetManager::Get()
{
	check(GEngine);
	UAuraAssetManager* AuraAssetManager = Cast<UAuraAssetManager>(GEngine->AssetManager);
	return *AuraAssetManager;
}

void UAuraAssetManager::StartInitialLoading()
{
	Super::StartInitialLoading();

	// 전역으로 선언되어있는 GameplayTags 인스턴스를 초기화
	FAuraGameplayTags::InitializeNativeGameplayTags();

	// TargetData를 사용하기 위해 반드시 호출해줘야 하는 함수
	UAbilitySystemGlobals::Get().InitGlobalData();
}
