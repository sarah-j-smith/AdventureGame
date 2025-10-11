// (c) 2025 Sarah Smith


#include "InventoryItem.h"

#include "AdventureGame/Constants.h"
#include "AdventureGame/AdventureGame.h"

#include "AdventureGame/Enums/VerbType.h"
#include "AdventureGame/Player/AdventurePlayerController.h"

#include "Internationalization/StringTableRegistry.h"

//////////////////////////////////
///
/// STATIC IMPLEMENTATIONS
///

void UInventoryItem::OnItemUseSuccess_Implementation()
{
    UE_LOG(LogAdventureGame, Log, TEXT("OnItemUseSuccess Success - default."));
    
    if (UItemDataAsset *ItemDataAsset = ItemDataAssetForAction(EVerbType::Use))
    {
        if (AAdventurePlayerController *Apc = GetAdventurePlayerController())
        {
            ItemDataAsset->OnItemUseSuccess();
            return;
        }
    }
    OnItemUseFailure();
}

void UInventoryItem::OnItemUseFailure_Implementation()
{
    BarkAndEnd(LOCTABLE(ITEM_STRINGS_KEY, "ItemUsedDefaultText"));
}

void UInventoryItem::OnItemGiveSuccess_Implementation()
{
    if (UItemDataAsset *ItemDataAsset = ItemDataAssetForAction(EVerbType::Give))
    {
        if (AAdventurePlayerController *Apc = GetAdventurePlayerController())
        {
            ItemDataAsset->OnItemGiveSuccess();
            return;
        }
    }
    OnItemGiveFailure();
}

void UInventoryItem::OnItemGiveFailure_Implementation()
{
    BarkAndEnd(LOCTABLE(ITEM_STRINGS_KEY, "ItemGivenDefaultText"));
}

UItemDataAsset* UInventoryItem::ItemDataAssetForAction(const EVerbType Verb) const
{
    // TODO - remove this bit of code once the deprecated OnUseSuccessItem and OnGiveSuccessItem are gone
    if (Verb == EVerbType::Use)
    {
        if (UItemDataAsset *UseItem = OnUseSuccessItem.LoadSynchronous())
        {
            UE_LOG(LogAdventureGame, Warning, TEXT("OnUseSuccessItem is deprecated in %s - use OnItemActivated instead"),
                *(ShortDescription.ToString()));
            return UseItem;
        }
    }
    else if (Verb == EVerbType::Give)
    {
        if (UItemDataAsset *UseItem = OnGiveSuccessItem.LoadSynchronous())
        {
            UE_LOG(LogAdventureGame, Warning, TEXT("OnGiveSuccessItem is deprecated in %s - use OnItemActivated instead"),
                *(ShortDescription.ToString()));
            return UseItem;
        }
    }
    return OnItemActivated.GetItemDataAssetForAction(Verb);
}

void UInventoryItem::OnClose_Implementation()
{
    IVerbInteractions::OnClose_Implementation();
    UE_LOG(LogAdventureGame, VeryVerbose, TEXT("On close"));
    BarkAndEnd(LOCTABLE(ITEM_STRINGS_KEY, "CloseDefaultText"));
}

void UInventoryItem::OnOpen_Implementation()
{
    IVerbInteractions::OnOpen_Implementation();
    UE_LOG(LogAdventureGame, VeryVerbose, TEXT("On open"));
    BarkAndEnd(LOCTABLE(ITEM_STRINGS_KEY, "OpenDefaultText"));
}

void UInventoryItem::OnGive_Implementation()
{
    IVerbInteractions::OnGive_Implementation();
    UE_LOG(LogAdventureGame, VeryVerbose, TEXT("On give inventory item defaultasdfasdfasdf"));
    BarkAndEnd(LOCTABLE(ITEM_STRINGS_KEY, "GiveDefaultText"));
}

void UInventoryItem::OnPickUp_Implementation()
{
    IVerbInteractions::OnPickUp_Implementation();
    UE_LOG(LogAdventureGame, VeryVerbose, TEXT("On Pickup"));
    BarkAndEnd(LOCTABLE(ITEM_STRINGS_KEY, "PickUpDefaultText"));
}

void UInventoryItem::OnTalkTo_Implementation()
{
    IVerbInteractions::OnTalkTo_Implementation();
    UE_LOG(LogAdventureGame, VeryVerbose, TEXT("On talk"));
    BarkAndEnd(LOCTABLE(ITEM_STRINGS_KEY, "TalkToDefaultText"));
}

void UInventoryItem::OnLookAt_Implementation()
{
    IVerbInteractions::OnLookAt_Implementation();
    UE_LOG(LogAdventureGame, VeryVerbose, TEXT("On look at"));
    if (Description.IsEmpty())
    {
        BarkAndEnd(LOCTABLE(ITEM_STRINGS_KEY, "LookAtDefaultText"));
    }
    else
    {
        BarkAndEnd(Description);
    }
}

void UInventoryItem::OnPull_Implementation()
{
    IVerbInteractions::OnPull_Implementation();
    UE_LOG(LogAdventureGame, VeryVerbose, TEXT("On pull"));
    BarkAndEnd(LOCTABLE(ITEM_STRINGS_KEY, "PullDefaultText"));
}

void UInventoryItem::OnPush_Implementation()
{
    IVerbInteractions::OnPush_Implementation();
    UE_LOG(LogAdventureGame, VeryVerbose, TEXT("On push"));
    BarkAndEnd(LOCTABLE(ITEM_STRINGS_KEY, "PushDefaultText"));
}

void UInventoryItem::OnUse_Implementation()
{
    UE_LOG(LogAdventureGame, Fatal, TEXT("SHOULD NEVER HAPPEN"
        " - when the player clicks the Use verb and then an item "
        "the AdventurePlayerController goes into targeting mode, looking for a Hotspot"
        " or another Item to use it on."));
}

void UInventoryItem::OnWalkTo_Implementation()
{
    IVerbInteractions::OnWalkTo_Implementation();
    BarkAndEnd(LOCTABLE(ITEM_STRINGS_KEY, "WalkToDefaultText"));
}

void UInventoryItem::OnItemUsed_Implementation()
{
    IVerbInteractions::OnItemUsed_Implementation();

    // **this** InventoryItem is the target and APC->SourceItem is the source of a Use
    // verb. Check that the Source can validly use on this.
    if (AAdventurePlayerController *AdventurePlayerController = GetAdventurePlayerController())
    {
        if (AdventurePlayerController->SourceItem->ItemKind == ItemKind)
        {
            // Item is used on itself - failure - this should not be necessary,
            // but needed in the case that during game design this item mistakenly
            // has its interactable item set to another with the same item kind.
            AdventurePlayerController->InterruptCurrentAction();
            OnItemUseFailure();
        }
        else if (CanInteractWith(AdventurePlayerController->SourceItem))
        {
            // This item has interactable item
            OnItemUseSuccess();
        }
        else if (const UItemDataAsset *ItemDataAsset = ItemDataAssetForAction(EVerbType::UseItem))
        {
            // We are the target, the second item clicked
            const EItemKind SrcKind = AdventurePlayerController->SourceItem->ItemKind;
            const EItemKind TgtKind = AdventurePlayerController->TargetItem->ItemKind;
            if (ItemDataAsset->SourceItem == SrcKind && ItemDataAsset->TargetItem == TgtKind)
            {
                OnItemUseSuccess();
            }
            else if (ItemDataAsset->SourceItem == TgtKind && ItemDataAsset->TargetItem == SrcKind && ItemDataAsset->CanSwapSourceAndTarget)
            {
                AdventurePlayerController->SwapSourceAndTarget();
                OnItemUseSuccess();
            }
        }
        else
        {
            // Item is not the one that can be used with this
            AdventurePlayerController->InterruptCurrentAction();
            OnItemUseFailure();
        }
    }
}

void UInventoryItem::OnItemGiven_Implementation()
{
    IVerbInteractions::OnItemGiven_Implementation();
    if (AAdventurePlayerController *AdventurePlayerController = GetAdventurePlayerController())
    {
        // TODO Giving items not yet implemented
        // AdventurePlayerController->GiveAnItem(ItemKind);
    }
    BarkAndEnd(LOCTABLE(ITEM_STRINGS_KEY, "ItemGivenDefaultText"));
}
