#include "SpellMenuWidgetController.h"
#include "Aura/AbilitySystem/AuraAbilitySystemComponent.h"

void USpellMenuWidgetController::BroadcastInitialValue()
{
}

void USpellMenuWidgetController::BindCallbacksToDependencies()
{
	// Ability가 부여될 때, SpellMenuWidget이 이를 알 수 있도록 함수를 바인드합니다.
	// HUD에 대한 초기화가 모두 이루어지고 나서 GameAbility를 부여하기 때문에, 여기서 바인드하면 정상 작동합니다.
	if (GetAuraASC())
	{
		GetAuraASC()->AbilitiesGivenDelegate.AddUObject(this, &ThisClass::OnAbilitiesGiven);
	}
}
