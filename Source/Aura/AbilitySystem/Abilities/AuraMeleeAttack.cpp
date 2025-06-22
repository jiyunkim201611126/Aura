#include "AuraMeleeAttack.h"

#include "Aura/Interaction/CombatInterface.h"
#include "Kismet/KismetMathLibrary.h"

FTaggedMontage UAuraMeleeAttack::GetRandomAttackMontage()
{
	TArray<FTaggedMontage> TaggedMontages = ICombatInterface::Execute_GetAttackMontages(GetAvatarActorFromActorInfo());
	return TaggedMontages[UKismetMathLibrary::RandomInteger(TaggedMontages.Num())];
}
