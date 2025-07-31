#include "LevelUpInfo.h"

int32 ULevelUpInfo::FindLevelForXP(int32 XP) const
{
	// XP 보유량에 따른 Level 결과를 이진 탐색으로 찾습니다.
	int32 Low = 1;
	int32 High = LevelUpInformation.Num() - 1;
	
	while (Low <= High)
	{
		int32 Mid = (Low + High) / 2;
		if (XP >= LevelUpInformation[Mid].LevelUpRequirement)
		{
			Low = Mid + 1;
		}
		else
		{
			High = Mid - 1;
		}
	}
	return Low;
}
