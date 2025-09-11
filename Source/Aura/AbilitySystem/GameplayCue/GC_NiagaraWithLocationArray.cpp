#include "GC_NiagaraWithLocationArray.h"

#include "Aura/AuraAbilityTypes.h"
#include "Aura/AbilitySystem/AuraAbilitySystemLibrary.h"
#include "Aura/Manager/FXManagerSubsystem.h"

bool UGC_NiagaraWithLocationArray::OnExecute_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const
{
	for (const FVector_NetQuantize& Location : UAuraAbilitySystemLibrary::GetLocationsFromContext(Parameters.EffectContext))
	{
		if (UFXManagerSubsystem* FXManager = GetWorld()->GetGameInstance()->GetSubsystem<UFXManagerSubsystem>())
		{
			FXManager->AsyncSpawnNiagaraAtLocation(Parameters.AggregatedSourceTags.GetByIndex(0), Location);
		}
	}
	
	return Super::OnExecute_Implementation(MyTarget, Parameters);
}
