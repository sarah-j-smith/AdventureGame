// (c) 2025 Sarah Smith


#include "ItemDataAsset.h"

#include "InventoryItem.h"
#include "ItemList.h"
#include "../Constants.h"
#include "../Player/AdventurePlayerController.h"
#include "AdventureGame/HotSpots/Door.h"


void UItemDataAsset::OnItemGiveSuccess_Implementation()
{
    AAdventurePlayerController* AdventurePlayerController = GetAdventurePlayerController();
    check(AdventurePlayerController);
    AdventurePlayerController->ItemRemoveFromInventory(SourceItem);
    StartTimer();
}

void UItemDataAsset::OnItemUseSuccess_Implementation()
{
    AAdventurePlayerController* AdventurePlayerController = GetAdventurePlayerController();
    check(AdventurePlayerController);
    bool Success = true;
    UInventoryItem* NewItem;
    switch (SourceItemAssetType)
    {
    case EItemAssetType::Consumable:
        AdventurePlayerController->ItemRemoveFromInventory(SourceItem);
        break;
    case EItemAssetType::Tool:
        NewItem = AdventurePlayerController->ItemAddToInventory(ToolResultItem);
        break;
    case EItemAssetType::Key:
        if (AHotSpot* ThisHotSpot = AdventurePlayerController->CurrentHotSpot)
        {
            if (ADoor* Door = Cast<ADoor>(ThisHotSpot))
            {
                Success = Door->UnlockDoor();
                if (!Success && Door->DoorState != EDoorState::Locked)
                {
                    AdventurePlayerController->PlayerBark(LOCTABLE(ITEM_STRINGS_KEY, "AlreadyUnlocked"));
                }
            }
        }
        else if (CanUnlockDoorOrItem(AdventurePlayerController->TargetItem->DoorState))
        {
            Success = true;
            AdventurePlayerController->TargetItem->DoorState = EDoorState::Closed;
        }
        break;
    default:
        break;
    }

    switch (TargetItemAssetType)
    {
    case EItemAssetType::Consumable:
        AdventurePlayerController->ItemRemoveFromInventory(TargetItem);
        break;
    default:
        break;
    }
    AdventurePlayerController->PlayerBark(Success ? UseSuccessBarkText : UseFailureBarkText);
    AdventurePlayerController->ShouldInterruptCurrentActionOnNextTick = true;
}

void UItemDataAsset::OnItemGiveFailure_Implementation()
{
    if (AAdventurePlayerController* AdventurePlayerController = GetAdventurePlayerController())
    {
        AdventurePlayerController->PlayerBark(GiveFailureBarkText);
    }
}

void UItemDataAsset::OnItemUseFailure_Implementation()
{
    if (AAdventurePlayerController* AdventurePlayerController = GetAdventurePlayerController())
    {
        AdventurePlayerController->PlayerBark(UseFailureBarkText);
    }
}

void UItemDataAsset::OnInteractionTimeout()
{
    if (AAdventurePlayerController* AdventurePlayerController = GetAdventurePlayerController())
    {
        AdventurePlayerController->InterruptCurrentAction();
    }
}

void UItemDataAsset::StartTimer()
{
    if (TimerRunning) return;
    if (AAdventurePlayerController* AdventurePlayerController = GetAdventurePlayerController())
    {
        AdventurePlayerController->InterruptCurrentAction();
        TimerRunning = true;
        AdventurePlayerController->GetWorldTimerManager().SetTimer(
            ActionHighlightTimerHandle, this,
            &UItemDataAsset::OnInteractionTimeout,
            InteractionTimeout, false);
    }
}

void UItemDataAsset::StopTimer()
{
    if (!TimerRunning) return;
    if (AAdventurePlayerController* AdventurePlayerController = GetAdventurePlayerController())
    {
        TimerRunning = false;
        AdventurePlayerController->GetWorldTimerManager().ClearTimer(ActionHighlightTimerHandle);
    }
}
