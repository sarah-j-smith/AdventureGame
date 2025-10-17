#pragma once

#include "DoorState.h"
#include "ItemAssetType.h"
#include "NativeGameplayTags.h"

namespace AdventureGameplayTags
{
    /// Schema is:
    ///     Scope - Disposition
    ///
    ///     Door State
    ADVENTUREGAME_API  UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Opened);
    ADVENTUREGAME_API  UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Closed);
    ADVENTUREGAME_API  UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Locked);

    ///     HotSpot values
    ADVENTUREGAME_API  UE_DECLARE_GAMEPLAY_TAG_EXTERN(HotSpot_Hidden);
    ADVENTUREGAME_API  UE_DECLARE_GAMEPLAY_TAG_EXTERN(HotSpot_SpriteHidden);

    ADVENTUREGAME_API  UE_DECLARE_GAMEPLAY_TAG_EXTERN(Item_Treatment_Article);
    ADVENTUREGAME_API  UE_DECLARE_GAMEPLAY_TAG_EXTERN(Item_Treatment_Consumable);
    ADVENTUREGAME_API  UE_DECLARE_GAMEPLAY_TAG_EXTERN(Item_Treatment_Tool);
    ADVENTUREGAME_API  UE_DECLARE_GAMEPLAY_TAG_EXTERN(Item_Treatment_Key);
    
    ADVENTUREGAME_API extern void SetDoorState(EDoorState State, FGameplayTagContainer &Tags);

    ADVENTUREGAME_API extern EDoorState GetDoorState(const FGameplayTagContainer &Tags);

    ADVENTUREGAME_API extern TSet<EItemAssetType> GetItemAssetTypes(const FGameplayTagContainer &Tags);
};
