#include "AuraDecal.h"
#include "Components/DecalComponent.h"

AAuraDecal::AAuraDecal()
{
	DecalComponent = CreateDefaultSubobject<UDecalComponent>("DecalComponent");
	DecalComponent->SetupAttachment(GetRootComponent());
}
