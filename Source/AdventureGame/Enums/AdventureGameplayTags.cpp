#include "AdventureGameplayTags.h"

#include "ItemAssetType.h"
#include "AdventureGame/AdventureGame.h"

namespace AdventureGameplayTags
{
    // If none of these tags is present for an item or hotspot then it's door state is "Unknown"
    // and cannot be locked, unlocked, opened, closed or walked through.
    UE_DEFINE_GAMEPLAY_TAG_COMMENT(State_Opened, "State.Opened", "The door or item is opened and accessible");
    UE_DEFINE_GAMEPLAY_TAG_COMMENT(State_Closed, "State.Closed", "The door or item is closed and unlocked");
    UE_DEFINE_GAMEPLAY_TAG_COMMENT(State_Locked, "State.Locked", "The door or item is closed and locked");

    // Status for various items in the objects that appear in the game. The default state is when the
    // tag is not present. For example if its not Hidden then its Visible.
    UE_DEFINE_GAMEPLAY_TAG_COMMENT(HotSpot_Hidden, "HotSpot.Hidden", "The entire HotSpot actor is hidden");
    UE_DEFINE_GAMEPLAY_TAG_COMMENT(HotSpot_SpriteHidden, "HotSpot.SpriteHidden", "The PickUp sprite is hidden");

    UE_DEFINE_GAMEPLAY_TAG_COMMENT(Item_Treatment_Article, "Item.Treatment.Article", "Object that can be held, examined and given");
    UE_DEFINE_GAMEPLAY_TAG_COMMENT(Item_Treatment_Consumable, "Item.Treatment.Consumable", "After being successfully used it is destroyed");
    UE_DEFINE_GAMEPLAY_TAG_COMMENT(Item_Treatment_Tool, "Item.Treatment.Tool", "Can be used on another item to create a brand new item");
    UE_DEFINE_GAMEPLAY_TAG_COMMENT(Item_Treatment_Key, "Item.Treatment.Key", "Can be used on a hotspot to lock or unlock it");

    void SetDoorState(EDoorState State, FGameplayTagContainer &Tags)
    {
        Tags.RemoveTag(AdventureGameplayTags::State_Closed);
        Tags.RemoveTag(AdventureGameplayTags::State_Opened);
        Tags.RemoveTag(AdventureGameplayTags::State_Locked);
        switch (State)
        {
        case EDoorState::Unknown:
            break;
        case EDoorState::Closed:
            Tags.AddTag(AdventureGameplayTags::State_Closed);
            break;
        case EDoorState::Opened:
            Tags.AddTag(AdventureGameplayTags::State_Opened);
            break;
        case EDoorState::Locked:
            Tags.AddTag(AdventureGameplayTags::State_Locked);
            break;
        }
    }

    TSet<EItemAssetType> GetItemAssetTypes(const FGameplayTagContainer &Tags)
    {
        TSet<EItemAssetType> Result;
        if (Tags.HasTag(Item_Treatment_Article)) Result.Add(EItemAssetType::Article);
        if (Tags.HasTag(Item_Treatment_Consumable)) Result.Add(EItemAssetType::Consumable);
        if (Tags.HasTag(Item_Treatment_Tool)) Result.Add(EItemAssetType::Tool);
        if (Tags.HasTag(Item_Treatment_Key)) Result.Add(EItemAssetType::Key);
        return Result;
    }

    EDoorState GetDoorState(const FGameplayTagContainer &Tags)
    {
        if (Tags.HasTag(AdventureGameplayTags::State_Closed)) return EDoorState::Closed;
        if (Tags.HasTag(AdventureGameplayTags::State_Opened)) return EDoorState::Opened;
        if (Tags.HasTag(AdventureGameplayTags::State_Locked)) return EDoorState::Locked;
        return EDoorState::Unknown;
    }
};