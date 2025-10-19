#include "MainMenuHUD.h"
#include "Aura/UI/ViewModel/MVVM_LoadMenu.h"
#include "Aura/UI/Widget/MainMenu/LoadMenuWidget.h"
#include "Aura/UI/Widget/MainMenu/MainMenuWidget.h"


void AMainMenuHUD::BeginPlay()
{
	Super::BeginPlay();

	LoadMenuViewModel = NewObject<UMVVM_LoadMenu>(this, LoadMenuViewModelClass);
	LoadMenuViewModel->InitializeLoadSlot();

	MainMenuWidget = CreateWidget<UMainMenuWidget>(GetWorld(), MainMenuWidgetClass);
	MainMenuWidget->AddToViewport();
	MainMenuWidget->LoadMenu->BlueprintInitializeWidget();
}
