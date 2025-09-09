#pragma once

#include "GameplayEffectTypes.h"
#include "AuraAbilityTypes.Generated.h"

/**
 * Tag와 똑같은 역할을 하는 enum을 굳이 선언한 이유는, FName보다 uint8이 패킷면에서 더 저렴하기 때문입니다.
 * 편의성은 당연히 감소하나, 디자이너에 의해 관리되는 Tag가 아니기 때문에 조금 번거롭더라도 패킷 최적화를 거치는 편이 좋다고 생각합니다.
 */
UENUM(BlueprintType)
enum class EDamageTypeContext : uint8
{
	None, // 무속성 데미지가 아니고, 데미지 부여 실패를 의미합니다.
	Fire,
	Lightning,
	Arcane,
	Physical,
	Max
};

USTRUCT(BlueprintType)
struct FDamageDataContext
{
	GENERATED_BODY()

	UPROPERTY()
	EDamageTypeContext DamageType = EDamageTypeContext::None;
	
	UPROPERTY()
	bool bIsBlockedHit = false;

	UPROPERTY()
	bool bIsCriticalHit = false;

	UPROPERTY()
	FVector_NetQuantize DeathImpulse = FVector_NetQuantize::ZeroVector;

	UPROPERTY()
	FVector_NetQuantize KnockbackForce = FVector_NetQuantize::ZeroVector;

	bool NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess);
};

UENUM(BlueprintType)
enum class EDebuffTypeContext : uint8
{
	None,
	Burn,
	Stun,
	Confuse,
	Max
};

USTRUCT(BlueprintType)
struct FDebuffDataContext
{
	GENERATED_BODY()
	
	UPROPERTY()
	EDebuffTypeContext DebuffType = EDebuffTypeContext::None;

	UPROPERTY()
	float DebuffDamage = 0.f;

	UPROPERTY()
	float DebuffDuration = 0.f;

	UPROPERTY()
	float DebuffFrequency = 0.f;

	bool NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess);
};

template<>
struct TStructOpsTypeTraits<FDebuffDataContext> : public TStructOpsTypeTraitsBase2<FDebuffDataContext>
{
	enum
	{
		WithNetSerialize = true
	};
};

/**
 * GE가 어떻게 적용되었는지에 대해 추적하는 데에 사용할 수 있으며, 서버와 클라 간 데이터가 공유되는 클래스
 * GE가 적용될 때 Ability로 적용한 건지, 그렇다면 어떤 Ability인지, GE를 적용시킨 건 누구인지 등등을 확인할 수 있음
 * 이외에도 커스텀해 멤버 변수를 추가해 추적 가능
 * '런타임 중 로직 작성을 위해 활용할 수 있는 로그'라고 생각하면 됨
 * 내부에 Instigator와 EffectCauser는 각각 GE를 발생시킨 ASC의 OwnerActor와 AvatarActor로 할당되어있음
 */

USTRUCT(BlueprintType)
struct FAuraGameplayEffectContext : public FGameplayEffectContext
{
	GENERATED_BODY()

public:
	const FDamageDataContext& GetDamageData() const { return DamageData; }
	const FDebuffDataContext& GetDebuffData() const { return DebuffData; }
	const TArray<FVector_NetQuantize>& GetLocations() const { return Locations; }

	void SetDamageDataContext(const EDamageTypeContext DamageType, bool bIsBlocked, bool bIsCritical);
	void SetDeathImpulse(const FVector& Impulse);
	void SetKnockbackForce(const FVector& Force);
	void SetDebuffDataContext(const FDebuffDataContext& DebuffDataContext);
	void SetLocations(const TArray<FVector_NetQuantize>& InLocations);
	
	virtual UScriptStruct* GetScriptStruct() const override
	{
		return StaticStruct();
	}

	virtual FAuraGameplayEffectContext* Duplicate() const override
	{
		FAuraGameplayEffectContext* NewContext = new FAuraGameplayEffectContext();
		*NewContext = *this;
		if (GetHitResult())
		{
			// 주소는 복사해봤자 의미 없기 때문에 주소를 통해 값까지 찾아가서 깊은 복사
			NewContext->AddHitResult(*GetHitResult(), true);
		}
		return NewContext;
	}

	// Context를 서버와 클라이언트 간 공유하기 위해 데이터를 직렬화 및 역직렬화하는 함수
	virtual bool NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess) override;

protected:
	UPROPERTY()
	FDamageDataContext DamageData;

	UPROPERTY()
	FDebuffDataContext DebuffData;

	UPROPERTY()
	TArray<FVector_NetQuantize> Locations;
};

/**
 * USTRUCT 자체만으로는 Replicate 시스템 같은 걸 이용할 수 없습니다.
 * 따라서 그것과 비슷하게 직렬화, 역직렬화 등의 기능을 활용해 서버와 클라이언트간 struct로 이루어진 데이터를 공유해야 합니다.
 * 아래 구문이 그걸 가능하게 해줍니다.
 * 작성된 2개의 열거자 외에도 다양한 열거자가 있으며, 서버와 클라이언트간 데이터 공유 시 유용하게 사용할 수 있는 기능들을 on/off합니다.
 */

template<>
struct TStructOpsTypeTraits<FAuraGameplayEffectContext> : TStructOpsTypeTraitsBase2<FAuraGameplayEffectContext>
{
	enum
	{
		WithNetSerializer = true,	// 네트워크 직렬화 지원
		WithCopy = true				// 복사 연산 지원
	};
};