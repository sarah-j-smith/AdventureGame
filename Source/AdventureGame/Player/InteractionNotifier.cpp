// (c) 2025 Sarah Smith


#include "InteractionNotifier.h"

void UInteractionNotifier::NotifyUserInteraction()
{
    if (!bUserInteractionActive && UserInteraction.IsBound())
    {
        UserInteraction.Broadcast();
        StartUserInteractionTimer();
    }
}

void UInteractionNotifier::StartUserInteractionTimer()
{
    GetWorld()->GetTimerManager().SetTimer(UserInteractionBroadcastTimer, this,
                                           &UInteractionNotifier::StopUserInteractionTimer, UserInteractionTime, false);
    bUserInteractionActive = true;
}

void UInteractionNotifier::ResetUserInteractionTimer()
{
    bUserInteractionActive = false;
    GetWorld()->GetTimerManager().ClearTimer(UserInteractionBroadcastTimer);
}

void UInteractionNotifier::StopUserInteractionTimer()
{
    bUserInteractionActive = false;
}
