#include "SavedActorInterface.h"

#include "Aura/Manager/SaveManagerSubsystem.h"

void ISavedActorInterface::RegisterSavedActor(const UObject* WorldContextObject)
{
	USaveManagerSubsystem* SaveManagerSubsystem = WorldContextObject->GetWorld()->GetGameInstance()->GetSubsystem<USaveManagerSubsystem>();
	if (SaveManagerSubsystem)
	{
		SaveManagerSubsystem->AddSavedActor(this);
	}
}

void ISavedActorInterface::UnregisterSavedActor(const UObject* WorldContextObject)
{
	USaveManagerSubsystem* SaveManagerSubsystem = WorldContextObject->GetWorld()->GetGameInstance()->GetSubsystem<USaveManagerSubsystem>();
	if (SaveManagerSubsystem)
	{
		SaveManagerSubsystem->RemoveSavedActor(this);
	}
}
