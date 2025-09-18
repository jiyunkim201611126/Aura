#include "AuraNiagaraComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Aura/Interaction/CombatInterface.h"
#include "Aura/Manager/FXManagerSubsystem.h"
#include "Net/UnrealNetwork.h"

UAuraNiagaraComponent::UAuraNiagaraComponent()
{
	SetIsReplicatedByDefault(true);
	SetUsingAbsoluteScale(true);
	bAutoActivate = false;
}

void UAuraNiagaraComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, NiagaraTag);
}

void UAuraNiagaraComponent::BeginPlay()
{
	Super::BeginPlay();

	// 이 시점에 아직 ASC가 생성되지 않았을 수 있으니, InitActorInfo 시점에 호출되는 델리게이트에 콜백 함수를 바인드하는 람다식을 안전장치로 걸어놓습니다.
	if (GetOwner()->HasAuthority())
	{
		ICombatInterface* CombatInterface = Cast<ICombatInterface>(GetOwner());
		if (UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner()))
		{
			ASC->RegisterGameplayTagEvent(NiagaraTag, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &ThisClass::NiagaraTagChanged);
		}
		else if (CombatInterface)
		{
			CombatInterface->GetOnASCRegisteredDelegate().AddWeakLambda(this, [this](UAbilitySystemComponent* InASC)
			{
				InASC->RegisterGameplayTagEvent(NiagaraTag, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &ThisClass::NiagaraTagChanged);
			});
		}

		// Owner가 사망했을 때도 Destroy되도록 콜백 함수를 바인드합니다.
		if (CombatInterface)
		{
			CombatInterface->GetOnDeathDelegate().AddDynamic(this, &ThisClass::OnOwnerDeath);
		}

		// 리슨 서버도 나이아가라 재생해야 하기 때문에 OnRep 함수를 그냥 호출합니다.
		OnRep_NiagaraTag();
	}
}

void UAuraNiagaraComponent::OnRep_NiagaraTag()
{
	// 나이아가라 재생을 동기 로드로 시작합니다.
	if (UFXManagerSubsystem* FXManager = GetWorld()->GetGameInstance()->GetSubsystem<UFXManagerSubsystem>())
	{
		FXManager->AsyncGetNiagara(NiagaraTag, [this](UNiagaraSystem* InNiagaraSystem)
		{
			if (!GetOwner() || ICombatInterface::Execute_IsDead(GetOwner()) || !InNiagaraSystem)
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

void UAuraNiagaraComponent::NiagaraTagChanged(const FGameplayTag CallbackTag, int32 NewCount)
{
	if (NewCount <= 0)
	{
		ClientDeactivateNiagara();
		DestroyComponent();
	}
}

void UAuraNiagaraComponent::OnOwnerDeath(AActor* DeadActor)
{
	ClientDeactivateNiagara();
	DestroyComponent();
}

void UAuraNiagaraComponent::ClientDeactivateNiagara_Implementation()
{
	Deactivate();
}
