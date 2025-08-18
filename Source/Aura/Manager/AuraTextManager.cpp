#include "AuraTextManager.h"

FName FAuraTextManager::GetPath(EStringTableTextType Type)
{
	switch (Type)
	{
	case EStringTableTextType::UI:
		return TEXT("/Game/Blueprint/UI/Data/ST_UI");
	}

	return FName();
}
