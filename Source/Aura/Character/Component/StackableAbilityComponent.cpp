#include "StackableAbilityComponent.h"
#include "Net/UnrealNetwork.h"

///////////////////////////////////////////////////////////////
// Ability Stack Item

void FAbilityStackItem::PostReplicatedAdd(const FAbilityStackArray& InArraySerializer)
{
	if (!InArraySerializer.OwnerComp)
	{
		return;
	}

	// 클라이언트도 Ability가 Array의 몇 번째 인덱스에 등록되어있는지 TMap에 캐싱합니다.
	int32 Index = INDEX_NONE;
	for (int32 i = 0; i < InArraySerializer.Items.Num(); ++i)
	{
		if (&InArraySerializer.Items[i] == this)
		{
			Index = i;
			break;
		}
	}
	InArraySerializer.OwnerComp->TagToIndex.Add(AbilityTag, Index);

	InArraySerializer.OwnerComp->OnStackCountChanged.ExecuteIfBound(AbilityTag, CurrentStack);

	if (bShouldTimerStart)
	{
		InArraySerializer.OwnerComp->OnStackTimerStarted.ExecuteIfBound(AbilityTag, RechargeTime);
	}
}

void FAbilityStackItem::PostReplicatedChange(const FAbilityStackArray& InArraySerializer)
{
	if (!InArraySerializer.OwnerComp)
	{
		return;
	}
	
	InArraySerializer.OwnerComp->OnStackCountChanged.ExecuteIfBound(AbilityTag, CurrentStack);

	if (bShouldTimerStart)
	{
		InArraySerializer.OwnerComp->OnStackTimerStarted.ExecuteIfBound(AbilityTag, RechargeTime);
	}
}

void FAbilityStackItem::PostReplicatedRemove(const FAbilityStackArray& InArraySerializer)
{
	if (!InArraySerializer.OwnerComp)
	{
		return;
	}

	InArraySerializer.OwnerComp->TagToIndex.Remove(AbilityTag);
}

///////////////////////////////////////////////////////////////
// Stackable Ability Component

UStackableAbilityComponent::UStackableAbilityComponent()
{
	SetIsReplicatedByDefault(true);
	PrimaryComponentTick.bCanEverTick = false;

	AbilityStacks.OwnerComp = this;
}

void UStackableAbilityComponent::BeginPlay()
{
	Super::BeginPlay();

	if (GetOwner()->HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("Server: UStackableAbilityComponent::BeginPlay"));
	}
	else
	{
		if (APawn* Pawn = Cast<APawn>(GetOwner()))
		{
			if (Pawn->IsLocallyControlled())
			{
				UE_LOG(LogTemp, Warning, TEXT("Client, NotMine: UStackableAbilityComponent::BeginPlay"));
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("Client, Mine: UStackableAbilityComponent::BeginPlay"));
			}
		}
	}
}

void UStackableAbilityComponent::DestroyComponent(bool bPromoteChildren)
{
	OnStackCountChanged.Unbind();
	OnStackTimerStarted.Unbind();
	Super::DestroyComponent(bPromoteChildren);
}

void UStackableAbilityComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// 현재는 몇 개의 스택이 쌓였는지 자신만 알면 되므로 OwnerOnly.
	// 추후 다른 사람의 스택이나 충전 시간 등을 알아야 하는 경우 CONDITION을 제거합니다.
	DOREPLIFETIME_CONDITION(UStackableAbilityComponent, AbilityStacks, COND_OwnerOnly);
}

void UStackableAbilityComponent::RegisterAbility(FGameplayTag AbilityTag, int32 CurrentStack, int32 MaxStack, float RechargeTime)
{
	if (!ensure(GetOwner() != nullptr) || !GetOwner()->HasAuthority())
	{
		return;
	}

	if (FAbilityStackItem* ExistingItem = FindItemMutable(AbilityTag))
	{
		// 이미 같은 Ability가 등록되어있는 경우 들어오는 분기
		ExistingItem->CurrentStack = CurrentStack;
		ExistingItem->MaxStack = MaxStack;
		ExistingItem->RechargeTime = RechargeTime;

		// 값이 변경되었으므로 클라이언트에게 알려줍니다.
		AbilityStacks.MarkItemDirty(*ExistingItem);
	}
	else
	{
		// 배열의 새로운 Element를 선언 및 초기화합니다.
		FAbilityStackItem NewItem;
		NewItem.AbilityTag = AbilityTag;
		NewItem.CurrentStack = CurrentStack;
		NewItem.MaxStack = MaxStack;
		NewItem.RechargeTime = RechargeTime;

		// 배열에 추가합니다.
		const int32 NewIndex = AbilityStacks.Items.Add(MoveTemp(NewItem));
		// TMap에 캐싱합니다.
		TagToIndex.Add(AbilityTag, NewIndex);

		// 값이 추가되었으므로 클라이언트에게 알려줍니다.
		AbilityStacks.MarkItemDirty(AbilityStacks.Items[NewIndex]);

		// 서버의 로컬 HUD에 현재 충전 횟수를 알립니다.
		OnStackCountChanged.ExecuteIfBound(AbilityTag, AbilityStacks.Items[NewIndex].CurrentStack);
	}

	// 충전 로직을 시작합니다.
	StartRecharge(AbilityTag);
}

void UStackableAbilityComponent::UnregisterAbility(FGameplayTag AbilityTag)
{
	if (!ensure(GetOwner() != nullptr) || !GetOwner()->HasAuthority())
	{
		return;
	}
	
	// 충전 로직을 중단, Ability를 엔트리에서 제외합니다.
	const int32 Index = FindIndexByTag(AbilityTag);
	if (AbilityStacks.Items.IsValidIndex(Index))
	{
		StopRecharge(AbilityTag);

		AbilityStacks.Items.RemoveAtSwap(Index);
		AbilityStacks.MarkArrayDirty();

		TagToIndex.Empty(AbilityStacks.Items.Num());
		for (int32 i = 0; i < AbilityStacks.Items.Num(); i++)
		{
			TagToIndex.Add(AbilityStacks.Items[i].AbilityTag, i);
		}
	}
}

bool UStackableAbilityComponent::CheckCost(FGameplayTag AbilityTag) const
{
	const FAbilityStackItem* Item = FindItem(AbilityTag);
	return Item && Item->CurrentStack > 0;
}

void UStackableAbilityComponent::ApplyCost(FGameplayTag AbilityTag)
{
	if (!ensure(GetOwner() != nullptr) || !GetOwner()->HasAuthority())
	{
		return;
	}

	if (FAbilityStackItem* Item = FindItemMutable(AbilityTag))
	{
		if (Item->CurrentStack > 0)
		{
			Item->CurrentStack = FMath::Max(0, Item->CurrentStack - 1);

			// 스택을 소모했으므로 서버의 로컬 HUD에 알립니다.
			OnStackCountChanged.ExecuteIfBound(AbilityTag, Item->CurrentStack);

			// 클라이언트에게 Timer를 시작할 필요는 없음을 기록합니다.
			// Timer를 시작해야 한다면 StartRecharge에서 다시 true로 바꿔 알려줍니다.
			Item->bShouldTimerStart = false;

			// 클라이언트에게 복제합니다.
			AbilityStacks.MarkItemDirty(*Item);
			
			StartRecharge(AbilityTag);
		}
	}
}

int32 UStackableAbilityComponent::GetCurrentStack(FGameplayTag AbilityTag) const
{
	const FAbilityStackItem* Item = FindItem(AbilityTag);
	return Item ? Item->CurrentStack : 0;
}

bool UStackableAbilityComponent::CheckHasAbility(FGameplayTag AbilityTag) const
{
	return FindItem(AbilityTag)	!= nullptr;
}

void UStackableAbilityComponent::StartRecharge(FGameplayTag AbilityTag)
{
	if (!ensure(GetOwner() != nullptr) || !GetOwner()->HasAuthority())
	{
		return;
	}

	if (FAbilityStackItem* Item = FindItemMutable(AbilityTag))
	{
		// 최대 충전치인 경우 충전을 중단합니다.
		if (Item->CurrentStack >= Item->MaxStack)
		{
			StopRecharge(AbilityTag);
			return;
		}

		if (UWorld* World = GetWorld())
		{
			// 최대 충전 상태가 아니고, 타이머가 돌고 있지 않은 경우 타이머를 작동시킵니다.
			if (!World->GetTimerManager().IsTimerActive(Item->RechargeTimerHandle))
			{
				World->GetTimerManager().SetTimer(
					Item->RechargeTimerHandle,
					FTimerDelegate::CreateUObject(this, &ThisClass::Recharge, AbilityTag),
					Item->RechargeTime,
					true
				);

				// 충전이 시작됐음을 서버의 로컬 HUD에 알립니다.
				OnStackTimerStarted.ExecuteIfBound(AbilityTag, Item->RechargeTime);

				// 클라이언트가 알 수 있도록 충전이 시작됐음을 기록합니다.
				Item->bShouldTimerStart = true;
				
				// 클라이언트에게 복제합니다.
				AbilityStacks.MarkItemDirty(*Item);
			}
		}
	}
}

void UStackableAbilityComponent::Recharge(FGameplayTag AbilityTag)
{
	if (!ensure(GetOwner() != nullptr) || !GetOwner()->HasAuthority())
	{
		return;
	}

	if (FAbilityStackItem* Item = FindItemMutable(AbilityTag))
	{
		Item->CurrentStack++;

		// 충전되었으므로 서버의 로컬 HUD에 알립니다.
		OnStackCountChanged.ExecuteIfBound(AbilityTag, Item->CurrentStack);

		// 최대 충전치에 도달한 경우 충전을 중단합니다.
		if (Item->CurrentStack >= Item->MaxStack)
		{
			StopRecharge(AbilityTag);
			return;
		}

		// 충전이 시작됐음을 서버의 로컬 HUD에 알립니다.
		OnStackTimerStarted.ExecuteIfBound(AbilityTag, Item->RechargeTime);

		// 클라이언트가 알 수 있도록 충전이 시작됐음을 기록합니다.
		Item->bShouldTimerStart = true;

		// 클라이언트에게 복제합니다.
		AbilityStacks.MarkItemDirty(*Item);
	}
}

void UStackableAbilityComponent::StopRecharge(FGameplayTag AbilityTag)
{
	if (!ensure(GetOwner() != nullptr) || !GetOwner()->HasAuthority())
	{
		return;
	}

	if (FAbilityStackItem* Item = FindItemMutable(AbilityTag))
	{
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(Item->RechargeTimerHandle);
		
			// 클라이언트가 알 수 있도록 충전이 종료되었음을 기록합니다. 
			Item->bShouldTimerStart = false;

			// 클라이언트에게 복제합니다.
			AbilityStacks.MarkItemDirty(*Item);
		}
	}
}

int32 UStackableAbilityComponent::FindIndexByTag(const FGameplayTag AbilityTag) const
{
	if (const int32* FoundIndex = TagToIndex.Find(AbilityTag))
	{
		return *FoundIndex;
	}

	for (int32 Index = 0; Index < AbilityStacks.Items.Num(); Index++)
	{
		if (AbilityStacks.Items[Index].AbilityTag == AbilityTag)
		{
			return Index;
		}
	}

	return INDEX_NONE;
}

FAbilityStackItem* UStackableAbilityComponent::FindItemMutable(const FGameplayTag AbilityTag)
{
	const int32 Index = FindIndexByTag(AbilityTag);
	return AbilityStacks.Items.IsValidIndex(Index) ? &AbilityStacks.Items[Index] : nullptr;
}

const FAbilityStackItem* UStackableAbilityComponent::FindItem(const FGameplayTag AbilityTag) const
{
	const int32 Index = FindIndexByTag(AbilityTag);
	return AbilityStacks.Items.IsValidIndex(Index) ? &AbilityStacks.Items[Index] : nullptr;
}
