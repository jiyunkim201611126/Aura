#include "DebuffNiagaraComponent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Aura/Interaction/CombatInterface.h"
#include "Aura/Manager/FXManagerSubsystem.h"
#include "Net/UnrealNetwork.h"

UDebuffNiagaraComponent::UDebuffNiagaraComponent()
{
	SetIsReplicatedByDefault(true);
	bAutoActivate = false;
}

void UDebuffNiagaraComponent::BeginPlay()
{
	Super::BeginPlay();

	// 이 시점에 아직 ASC가 생성되지 않았을 수 있으니, InitActorInfo 시점에 호출되는 델리게이트에 콜백 함수를 바인드하는 람다식을 안전장치로 걸어놓습니다.
	if (GetOwner()->HasAuthority())
	{
		ICombatInterface* CombatInterface = Cast<ICombatInterface>(GetOwner());
		if (UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner()))
		{
			ASC->RegisterGameplayTagEvent(DebuffTag, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &ThisClass::DebuffTagChanged);
		}
		else if (CombatInterface)
		{
			CombatInterface->GetOnASCRegisteredDelegate().AddWeakLambda(this, [this](UAbilitySystemComponent* InASC)
			{
				InASC->RegisterGameplayTagEvent(DebuffTag, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &ThisClass::DebuffTagChanged);
			});
		}

		// Owner가 사망했을 때도 Destroy되도록 콜백 함수를 바인드합니다.
		if (CombatInterface)
		{
			CombatInterface->GetOnDeathDelegate().AddDynamic(this, &ThisClass::OnOwnerDeath);
		}
	}
}

void UDebuffNiagaraComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, DebuffTag);
}

void UDebuffNiagaraComponent::OnRep_DebuffTag()
{
	// 나이아가라 재생을 동기 로드로 시작합니다.
	if (UFXManagerSubsystem* FXManager = GetWorld()->GetGameInstance()->GetSubsystem<UFXManagerSubsystem>())
	{
		FXManager->AsyncGetNiagara(DebuffTag, [this](UNiagaraSystem* InNiagaraSystem)
		{
			if (!IsValid(this) || !InNiagaraSystem)
			{
				Deactivate();
				if (GetOwner()->HasAuthority())
				{
					DestroyComponent();
				}
				return;
			}

			SetAsset(InNiagaraSystem);
			Activate();
		});
	}
}

void UDebuffNiagaraComponent::DebuffTagChanged(const FGameplayTag CallbackTag, int32 NewCount)
{
	if (NewCount <= 0)
	{
		ClientDeactivateNiagara();
		DestroyComponent();
	}
}

void UDebuffNiagaraComponent::OnOwnerDeath(AActor* DeadActor)
{
	ClientDeactivateNiagara();
	DestroyComponent();
}

void UDebuffNiagaraComponent::ClientDeactivateNiagara_Implementation()
{
	Deactivate();
}
