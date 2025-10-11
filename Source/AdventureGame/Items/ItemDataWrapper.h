// (c) 2025 Sarah Smith

#pragma once

#include "CoreMinimal.h"
#include "AdventureGame/Enums/VerbType.h"
#include "UObject/Object.h"
#include "ItemDataWrapper.generated.h"

class UItemDataAsset;

USTRUCT(BlueprintType, Blueprintable)
struct ADVENTUREGAME_API FItemDataWrapper
{
    GENERATED_USTRUCT_BODY()

    /// Informative title for how this asset is used
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="ItemAction")
    FString ItemDataTitle;
    
    /// Select the verb which when actioned successfully on the Hotspot
    /// or Item this is attached to will cause the ItemDataAsset to be actioned,
    /// applying the <code>ScoreOnSuccess</code> and so on. 
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="ActiveVerb")
    EVerbType ActiveVerb;

    /// Item data asset to activate on the given verb. This must be a <code>ItemDataAsset</code>
    /// <b>Instance</b>. An instance appears in the editor as a circle with a pie-slice shape
    /// taken out of it. It must not be a <b>Class</b>. 
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ItemHandling")
    TSoftObjectPtr<UItemDataAsset> ItemDataAsset;

    UItemDataAsset *UnwrapItemDataAsset() const;
};
