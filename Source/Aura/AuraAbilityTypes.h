#pragma once

#include "GameplayEffectTypes.h"
#include "AuraAbilityTypes.Generated.h"

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
	bool IsBlockedHit() const { return bIsBlockedHit; }
	bool IsCriticalHit() const { return bIsCriticalHit; }

	void SetIsBlockedHit(bool bInIsBlockedHit) { bIsBlockedHit = bInIsBlockedHit; }
	void SetIsCriticalHit(bool bInIsCriticalHit) { bIsCriticalHit = bInIsCriticalHit; }
	
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
	bool bIsBlockedHit = false;

	UPROPERTY()
	bool bIsCriticalHit = false;
};

template<>
struct TStructOpsTypeTraits<FAuraGameplayEffectContext> : TStructOpsTypeTraitsBase2<FAuraGameplayEffectContext>
{
	enum
	{
		WithNetSerializer = true,
		WithCopy = true
	};
};