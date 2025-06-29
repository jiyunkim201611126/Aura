#include "AuraGameplayAbility.h"

FTaggedMontage UAuraGameplayAbility::GetRandomMontage()
{
	const int RandomIndex = FMath::RandRange(0, TaggedMontages.Num() - 1);
	FTaggedMontage TaggedMontage = TaggedMontages[RandomIndex];

	return TaggedMontage;
}
