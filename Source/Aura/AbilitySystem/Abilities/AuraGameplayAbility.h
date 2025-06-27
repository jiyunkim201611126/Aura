#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "AuraGameplayAbility.generated.h"

USTRUCT(BlueprintType)
struct FTaggedMontage
{
	GENERATED_BODY()
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UAnimMontage* Montage = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag MontageTag;

	/**
	 * 근접 공격 캐릭터에겐 공격 판정 위치, 원거리 공격 캐릭터에겐 투사체 발사 위치
	 * 현재 목록
	 * TipSocket (Weapon에서 사용)
	 * LeftHandSocket
	 * RightHandSocket
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FName SocketName = FName("TipSocket");
};

UCLASS()
class AURA_API UAuraGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	/**
	 * 플레이어의 캐릭터만 사용하는 태그입니다.
	 * Input과 관련된 태그들은 AuraInputConfig를 통해 InputAction과 이어져있습니다.
	 * AuraInputComponent를 통해 InputAction에 바인드된 함수는 호출 시 자동으로 연결된 태그가 매개변수로 들어갑니다.
	 * 해당 매개변수로 사용자가 가진 Ability 중 태그가 일치하는 Ability를 가져와 호출하는 로직입니다.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	FGameplayTag StartupInputTag;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Montage")
	TArray<FTaggedMontage> TaggedMontage;
};
