#include "AuraAbilityTypes.h"

#include "Manager/AuraGameplayTags.h"

void FAuraGameplayEffectContext::SetDamageType(const FGameplayTag& InDamageType)
{
	if (InDamageType == FAuraGameplayTags::Get().Damage_Fire)
	{
		DamageType = EDamageTypeData::Fire;
	}
	else if (InDamageType == FAuraGameplayTags::Get().Damage_Lightning)
	{
		DamageType = EDamageTypeData::Lightning;
	}
	else if (InDamageType == FAuraGameplayTags::Get().Damage_Arcane)
	{
		DamageType = EDamageTypeData::Arcane;
	}
	else if (InDamageType == FAuraGameplayTags::Get().Damage_Physical)
	{
		DamageType = EDamageTypeData::Physical;
	}
}

bool FAuraGameplayEffectContext::NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
{
	// 동기화해야 하는 정보가 늘어났기 때문에 기존 uint8 대신 uint32로 선언  
	uint32 RepBits = 0;
	if (Ar.IsSaving())
	{
		if (bReplicateInstigator && Instigator.IsValid())
		{
			RepBits |= 1 << 0;
		}
		if (bReplicateEffectCauser && EffectCauser.IsValid() )
		{
			RepBits |= 1 << 1;
		}
		if (AbilityCDO.IsValid())
		{
			RepBits |= 1 << 2;
		}
		if (bReplicateSourceObject && SourceObject.IsValid())
		{
			RepBits |= 1 << 3;
		}
		if (Actors.Num() > 0)
		{
			RepBits |= 1 << 4;
		}
		if (HitResult.IsValid())
		{
			RepBits |= 1 << 5;
		}
		if (bHasWorldOrigin)
		{
			RepBits |= 1 << 6;
		}
		if (bIsBlockedHit)
		{
			RepBits |= 1 << 7;
		}
		if (bIsCriticalHit)
		{
			RepBits |= 1 << 8;
		}
		if (DamageType != EDamageTypeData::None)
		{
			RepBits |= 1 << 9;
		}
	}

	// uint32를 몽땅 쓰지 않고 필요한 만큼의 길이로 잘라내 패킷 최적화
	Ar.SerializeBits(&RepBits, 10);

	if (RepBits & (1 << 0))
	{
		Ar << Instigator;
	}
	if (RepBits & (1 << 1))
	{
		Ar << EffectCauser;
	}
	if (RepBits & (1 << 2))
	{
		Ar << AbilityCDO;
	}
	if (RepBits & (1 << 3))
	{
		Ar << SourceObject;
	}
	if (RepBits & (1 << 4))
	{
		SafeNetSerializeTArray_Default<31>(Ar, Actors);
	}
	if (RepBits & (1 << 5))
	{
		if (Ar.IsLoading())
		{
			if (!HitResult.IsValid())
			{
				HitResult = TSharedPtr<FHitResult>(new FHitResult());
			}
		}
		HitResult->NetSerialize(Ar, Map, bOutSuccess);
	}
	if (RepBits & (1 << 6))
	{
		Ar << WorldOrigin;
		bHasWorldOrigin = true;
	}
	else
	{
		bHasWorldOrigin = false;
	}
	if (RepBits & (1 << 7))
	{
		Ar << bIsBlockedHit;
		bIsBlockedHit = true;
	}
	if (RepBits & (1 << 8))
	{
		Ar << bIsCriticalHit;
		bIsCriticalHit = true;
	}
	if (RepBits & (1 << 9))
	{
		Ar << DamageType;
	}

	if (Ar.IsLoading())
	{
		AddInstigator(Instigator.Get(), EffectCauser.Get());
	}	
	
	bOutSuccess = true;
	return true;
}
