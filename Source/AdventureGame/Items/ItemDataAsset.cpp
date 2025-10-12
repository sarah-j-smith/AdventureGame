// (c) 2025 Sarah Smith


#include "ItemDataAsset.h"

#include "InventoryItem.h"
#include "ItemList.h"
#include "../Constants.h"
#include "../Player/AdventurePlayerController.h"
#include "AdventureGame/HotSpots/Door.h"
#include "AdventureGame/Player/ItemManager.h"


void UItemDataAsset::OnItemGiveSuccess_Implementation()
{
    if (UItemManager *ItemManager = GetItemManager())
    {
        ItemManager->ItemRemoveFromInventory(SourceItem);
    }
    StartTimer();
}

void UItemDataAsset::OnItemUseSuccess_Implementation()
{
    UItemManager *ItemManager = GetItemManager();
    const ACommandManager *CommandManager = GetCommandManager();
    if (!ItemManager || !CommandManager) return;
    bool Success = true;
    switch (SourceItemAssetType)
    {
    case EItemAssetType::Consumable:
        ItemManager->ItemRemoveFromInventory(SourceItem);
        break;
    case EItemAssetType::Tool:
       ItemManager->ItemAddToInventory(ToolResultItem);
        break;
    case EItemAssetType::Key:
        if (AHotSpot* ThisHotSpot = CommandManager->CurrentHotSpot)
        {
            if (ADoor* Door = Cast<ADoor>(ThisHotSpot))
            {
                Success = Door->UnlockDoor();
                if (!Success && Door->DoorState != EDoorState::Locked)
                {
                    Bark(LOCTABLE(ITEM_STRINGS_KEY, "AlreadyUnlocked"));
                }
            }
        }
        else if (CanUnlockDoorOrItem(ItemManager->TargetItem->DoorState))
        {
            Success = true;
            ItemManager->TargetItem->DoorState = EDoorState::Closed;
        }
        break;
    default:
        break;
    }

    switch (TargetItemAssetType)
    {
    case EItemAssetType::Consumable:
        ItemManager->ItemRemoveFromInventory(TargetItem);
        break;
    default:
        break;
    }
    BarkAndEnd(Success ? UseSuccessBarkText : UseFailureBarkText);
}

void UItemDataAsset::OnItemGiveFailure_Implementation()
{
    BarkAndEnd(GiveFailureBarkText);
}

void UItemDataAsset::OnItemUseFailure_Implementation()
{
    BarkAndEnd(UseFailureBarkText);
}

void UItemDataAsset::OnInteractionTimeout()
{
    if (ACommandManager *CommandManager = GetCommandManager())
    {
        CommandManager->InterruptCurrentAction();
    }
}

void UItemDataAsset::StartTimer()
{
    if (TimerRunning) return;
    if (ACommandManager *CommandManager = GetCommandManager())
    {
        TimerRunning = true;
        GetWorld()->GetTimerManager().SetTimer(
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
