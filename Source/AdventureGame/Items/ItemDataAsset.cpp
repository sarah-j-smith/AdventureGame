// (c) 2025 Sarah Smith


#include "ItemDataAsset.h"

#include "ILocalizedAssetTools.h"
#include "InventoryItem.h"
#include "ItemList.h"
#include "../Constants.h"
#include "../Player/AdventurePlayerController.h"
#include "AdventureGame/Enums/AdventureGameplayTags.h"
#include "AdventureGame/HotSpots/Door.h"
#include "AdventureGame/Player/ItemManager.h"


void UItemDataAsset::OnItemGiveSuccess_Implementation()
{
    if (UItemManager *ItemManager = GetItemManager())
    {
        ItemManager->AddToScore(ScoreOnSuccess);
        ItemManager->ItemRemoveFromInventory(SourceItem);
    }
    StartTimer();
}

void UItemDataAsset::OnItemUseSuccess_Implementation()
{
    UItemManager *ItemManager = GetItemManager();
    const ACommandManager *CommandManager = GetCommandManager();
    if (!ItemManager || !CommandManager) return;
    ItemManager->AddToScore(ScoreOnSuccess);
    bool Success = true;
    TSet<EItemAssetType> ItemAssetTypes = AdventureGameplayTags::GetItemAssetTypes(SourceItemTreatmentTags);
    ItemAssetTypes.Add(SourceItemAssetType);
    for (const EItemAssetType& ItemAssetType : ItemAssetTypes)
    {
        HandleSourceItem(ItemAssetType, Success);
    }

    TSet<EItemAssetType> TargetItemAssetTypes = AdventureGameplayTags::GetItemAssetTypes(TargetItemTreatmentTags);
    TargetItemAssetTypes.Add(TargetItemAssetType);
    for (const EItemAssetType& ItemAssetType : TargetItemAssetTypes)
    {
        HandleTargetItem(ItemAssetType, Success);
    }
    BarkAndEnd(Success ? UseSuccessBarkText : UseFailureBarkText);
}

void UItemDataAsset::HandleSourceItem(const EItemAssetType ItemAssetType, bool &Success)
{
    UItemManager *ItemManager = GetItemManager();
    switch (ItemAssetType)
    {
    case EItemAssetType::Consumable:
        ItemManager->ItemRemoveFromInventory(SourceItem);
        break;
    case EItemAssetType::Tool:
        ItemManager->ItemAddToInventory(ToolResultItem);
        break;
    case EItemAssetType::Key:
        HandleKeyCase(Success);
        break;
    default:
        UE_LOG(LogAdventureGame, Error, TEXT("Item data asset has bad asset type: %s"),
            *UEnum::GetValueAsString(SourceItemAssetType));
        break;
    }
}

void UItemDataAsset::HandleTargetItem(const EItemAssetType ItemAssetType, bool& Success)
{
    UItemManager *ItemManager = GetItemManager();
    switch (ItemAssetType)
    {
    case EItemAssetType::Consumable:
        ItemManager->ItemRemoveFromInventory(TargetItem);
        break;
    default:
        break;
    }
}

void UItemDataAsset::HandleKeyCase(bool &Success)
{
    const ACommandManager *CommandManager = GetCommandManager();
    UItemManager *ItemManager = GetItemManager();
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
    {
        TimerRunning = false;
        GetWorld()->GetTimerManager().ClearTimer(ActionHighlightTimerHandle);
    }
}
