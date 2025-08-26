#include "AuraAbilityTypes.h"

#include "Manager/AuraGameplayTags.h"

bool FDebuffDataContext::NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
{
	uint8 DebuffTypeByte = static_cast<uint8>(DebuffType);
	Ar.SerializeBits(&DebuffTypeByte, 8);
	DebuffType = static_cast<EDebuffTypeContext>(DebuffTypeByte);

	Ar << DebuffDamage;
	Ar << DebuffDuration;
	Ar << DebuffFrequency;

	bOutSuccess = true;
	return true;
}

void FAuraGameplayEffectContext::SetDamageTypeContext(const EDamageTypeContext InDamageType)
{
	DamageType = InDamageType;
}

void FAuraGameplayEffectContext::SetDebuffDataContext(const FDebuffDataContext& DebuffDataContext)
{
	DebuffData = DebuffDataContext;
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
		if (DamageType != EDamageTypeContext::None)
		{
			RepBits |= 1 << 9;
		}
		if (DebuffData.DebuffType != EDebuffTypeContext::None)
		{
			RepBits |= 1 << 10;
		}
	}

	// uint32를 몽땅 쓰지 않고 필요한 만큼의 길이로 잘라내 패킷 최적화
	Ar.SerializeBits(&RepBits, 11);

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
	if (RepBits & (1 << 10))
	{
		bool bSuccess = false;
		DebuffData.NetSerialize(Ar, Map, bOutSuccess);
		bOutSuccess &= bSuccess;
	}

	if (Ar.IsLoading())
	{
		AddInstigator(Instigator.Get(), EffectCauser.Get());
	}	
	
	bOutSuccess = true;
	return true;
}
