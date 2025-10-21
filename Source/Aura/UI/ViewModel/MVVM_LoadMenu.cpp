#include "MVVM_LoadMenu.h"

#include "MVVM_LoadSlot.h"

void UMVVM_LoadMenu::InitializeLoadSlot()
{
	LoadSlot_0 = NewObject<UMVVM_LoadSlot>(this, LoadSlotViewModelClass);
	LoadSlots.Add(LoadSlot_0);
	LoadSlot_1 = NewObject<UMVVM_LoadSlot>(this, LoadSlotViewModelClass);
	LoadSlots.Add(LoadSlot_1);
	LoadSlot_2 = NewObject<UMVVM_LoadSlot>(this, LoadSlotViewModelClass);
	LoadSlots.Add(LoadSlot_2);
}

UMVVM_LoadSlot* UMVVM_LoadMenu::GetLoadSlotViewModelByIndex(int32 Index)
{
	return LoadSlots[Index];
}

void UMVVM_LoadMenu::NewGameButtonPressed(int32 Slot)
{
	LoadSlots[Slot]->SetWidgetSwitcherIndex.Broadcast(1);
}

void UMVVM_LoadMenu::NewSlotButtonPressed(int32 Slot, const FText& EnteredName)
{
}

void UMVVM_LoadMenu::SelectSlotButtonPressed(int32 Slot)
{
}
