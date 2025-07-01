#include "SoundManagerSubsystem.h"

void USoundManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	const UDataTable* DataTable = TaggedSoundDataTable.LoadSynchronous();
	check(DataTable);
	
	const FString Context(TEXT("SoundManagerSubsystem"));
	TArray<FTaggedSoundRow*> Rows; 
	DataTable->GetAllRows<FTaggedSoundRow>(Context, Rows);

	for (const FTaggedSoundRow* Row : Rows)
	{
		if (!Row)
		{
			continue;
		}

		USoundBase* LoadedSound = Row->SoundAsset.LoadSynchronous();
		if (!LoadedSound)
		{
			continue;
		}

		SoundInfos.Add(Row->SoundTag, LoadedSound);
	}
}

USoundBase* USoundManagerSubsystem::GetSound(const FGameplayTag SoundTag)
{
	if (SoundInfos.Find(SoundTag)->IsValid())
	{
		return SoundInfos.Find(SoundTag)->Get();
	}
	
	return SoundInfos.Find(SoundTag)->LoadSynchronous();
}
