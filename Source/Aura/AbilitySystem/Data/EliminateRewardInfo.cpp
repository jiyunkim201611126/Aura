#include "EliminateRewardInfo.h"

FEliminateRewardDefaultInfo UEliminateRewardInfo::GetEliminateRewardInfoByRank(ECharacterRank CharacterRank)
{
	return EliminateRewardInformation.FindChecked(CharacterRank);
}