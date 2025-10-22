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

    ADVENTUREGAME_API  UE_DECLARE_GAMEPLAY_TAG_EXTERN(History_Triggered_Give);
    ADVENTUREGAME_API  UE_DECLARE_GAMEPLAY_TAG_EXTERN(History_Triggered_Open);
    ADVENTUREGAME_API  UE_DECLARE_GAMEPLAY_TAG_EXTERN(History_Triggered_Close);
    ADVENTUREGAME_API  UE_DECLARE_GAMEPLAY_TAG_EXTERN(History_Triggered_PickUp);
    ADVENTUREGAME_API  UE_DECLARE_GAMEPLAY_TAG_EXTERN(History_Triggered_LookAt);
    ADVENTUREGAME_API  UE_DECLARE_GAMEPLAY_TAG_EXTERN(History_Triggered_TalkTo);
    ADVENTUREGAME_API  UE_DECLARE_GAMEPLAY_TAG_EXTERN(History_Triggered_Use);
    ADVENTUREGAME_API  UE_DECLARE_GAMEPLAY_TAG_EXTERN(History_Triggered_Push);
    ADVENTUREGAME_API  UE_DECLARE_GAMEPLAY_TAG_EXTERN(History_Triggered_Pull);
    
    ADVENTUREGAME_API  UE_DECLARE_GAMEPLAY_TAG_EXTERN(Item_Succeeded_ItemData);
    
    ADVENTUREGAME_API  UE_DECLARE_GAMEPLAY_TAG_EXTERN(Item_Treatment_Article);
    ADVENTUREGAME_API  UE_DECLARE_GAMEPLAY_TAG_EXTERN(Item_Treatment_Consumable);
    ADVENTUREGAME_API  UE_DECLARE_GAMEPLAY_TAG_EXTERN(Item_Treatment_Tool);
    ADVENTUREGAME_API  UE_DECLARE_GAMEPLAY_TAG_EXTERN(Item_Treatment_Key);

    ADVENTUREGAME_API extern void SetDoorState(EDoorState State, FGameplayTagContainer &Tags);

    ADVENTUREGAME_API extern EDoorState GetDoorState(const FGameplayTagContainer &Tags);

    ADVENTUREGAME_API extern TSet<EItemAssetType> GetItemAssetTypes(const FGameplayTagContainer &Tags);

    ADVENTUREGAME_API extern FGameplayTagContainer HistoryGameplayTags();
};
