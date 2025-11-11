#include "AuraSaveGame.h"

FSavedMap* UAuraSaveGame::GetSavedMapWithMapName(const FString& InMapName)
{
	for (FSavedMap& SavedMap : SavedMaps)
	{
		if (SavedMap.MapAssetName == InMapName)
		{
			return &SavedMap;
		}
	}
	return nullptr;
}
