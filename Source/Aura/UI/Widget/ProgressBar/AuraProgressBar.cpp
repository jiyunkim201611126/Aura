#include "AuraProgressBar.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/ProgressBar.h"

void UAuraProgressBar::SetBarPercent_Implementation(const float Value, const float MaxValue)
{
	const float Percent = UKismetMathLibrary::SafeDivide(Value, MaxValue);
	FrontProgressBar->SetPercent(Percent);

	FTimerHandle PercentSetTimerHandle;
	GetWorld()->GetTimerManager().SetTimer(
		PercentSetTimerHandle,
		this,
		&UAuraProgressBar::BarPercentSet,
		GhostStartDelay,
		false);
}

void UAuraProgressBar::BarPercentSet_Implementation()
{
	GhostPercentTarget = FrontProgressBar->GetPercent();

	GetWorld()->GetTimerManager().ClearTimer(GhostPercentSetTimerHandle);
	GetWorld()->GetTimerManager().SetTimer(
		GhostPercentSetTimerHandle,
		this,
		&UAuraProgressBar::InterpGhostBar,
		GhostInterpDelay,
		true);
}

void UAuraProgressBar::InterpGhostBar_Implementation()
{
	const float CurrentGhostBarPercent = GhostProgressBar->GetPercent();
	const float NextGhostBarPercent = UKismetMathLibrary::FInterpTo(CurrentGhostBarPercent, GhostPercentTarget, GetWorld()->GetDeltaSeconds(), GhostInterpSpeed);
	GhostProgressBar->SetPercent(NextGhostBarPercent);

	if (UKismetMathLibrary::NearlyEqual_FloatFloat(NextGhostBarPercent, GhostPercentTarget))
	{
		GetWorld()->GetTimerManager().ClearTimer(GhostPercentSetTimerHandle);
	}
}
