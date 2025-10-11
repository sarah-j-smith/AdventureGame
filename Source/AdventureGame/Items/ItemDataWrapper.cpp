// (c) 2025 Sarah Smith


#include "ItemDataWrapper.h"

#include "ItemDataAsset.h"
#include "AdventureGame/AdventureGame.h"

UItemDataAsset *FItemDataWrapper::UnwrapItemDataAsset() const
{
    if (UItemDataAsset *UnwrappedItemDataAsset = ItemDataAsset.LoadSynchronous())
        return UnwrappedItemDataAsset;
    UE_LOG(LogAdventureGame, Warning, TEXT("ItemDataAsset for %s is not loaded"), *ItemDataTitle);
    return nullptr;
}
