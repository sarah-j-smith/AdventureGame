#include "AdvGameUtils.h"

#include "AdventureGame/Constants.h"
#include "AdventureGame/HotSpots/HotSpot.h"
#include "AdventureGame/Items/InventoryItem.h"
#include "Misc/Guid.h"

#include "AdventureGame/AdventureGame.h"

/**
 * Test to see if Current has changed more than DBL_EPSILON compared to Previous in either `X` or `Y`.
 * @param Current `FVector2D` Value to check to see if it differs
 * @param Previous `FVector2D` Value to check against as a baseline
 * @return true if `Current` is significantly different to `Previous`, false otherwise
 */
bool AdvGameUtils::HasChangedMuch(const FVector2D& Current, const FVector2D& Previous)
{
    return (fabs(Previous.X - Current.X) >= std::numeric_limits<double>::epsilon() || fabs(Previous.Y - Current.Y) >=
        std::numeric_limits<double>::epsilon());
}

uint32 AdvGameUtils::GetUUID()
{
    const FGuid NewGuid = FGuid::NewGuid();
    return NewGuid.A;
}

FText AdvGameUtils::GetScoreText(int32 Score)
{
    FFormatNamedArguments ScoreArgs;
    ScoreArgs.Add("ScoreValue", Score);
    return FText::Format(LOCTABLE(UI_STRINGS_KEY, G_SCORE_TEMPLATE_KEY), ScoreArgs);
}

FText AdvGameUtils::GetGivingItemText(const UInventoryItem* CurrentItem, const UInventoryItem* TargetItem,
                                      const AHotSpot* HotSpot)
{
    verifyf(CurrentItem, TEXT("GetGivingItemString expects CurrentItem to be non-null"));
    FText SubjectText = CurrentItem->ShortDescription;
    FFormatNamedArguments VerbArgs;
    VerbArgs.Add("Subject", SubjectText);
    if (HotSpot == nullptr && TargetItem == nullptr)
    {
        return FText::Format(LOCTABLE(ITEM_STRINGS_KEY, G_GIVE_TO_PREVIEW), VerbArgs);
    }
    if (TargetItem != nullptr && TargetItem->ItemKind == CurrentItem->ItemKind)
    {
        return FText::Format(LOCTABLE(ITEM_STRINGS_KEY, G_GIVE_TO_SELF_DEFAULT_KEY), VerbArgs);
    }
    FText TargetText = HotSpot ? HotSpot->ShortDescription : TargetItem->ShortDescription;
    VerbArgs.Add("Object", TargetText);
    return FText::Format(LOCTABLE(ITEM_DESCRIPTIONS_KEY, G_GIVE_SUBJECT_TO_OBJECT_KEY), VerbArgs);
}

FText AdvGameUtils::GetUsingItemText(const UInventoryItem* CurrentItem, const UInventoryItem* TargetItem,
                                     const AHotSpot* HotSpot)
{
    verifyf(CurrentItem, TEXT("GetGivingItemString expects CurrentItem to be non-null"));
    FText SubjectText = CurrentItem->ShortDescription;
    FFormatNamedArguments VerbArgs;
    VerbArgs.Add("Subject", SubjectText);
    if (HotSpot == nullptr && TargetItem == nullptr)
    {
        return FText::Format(LOCTABLE(ITEM_STRINGS_KEY, G_USE_ON_PREVIEW), VerbArgs);
    }
    if (TargetItem != nullptr && TargetItem->ItemKind == CurrentItem->ItemKind)
    {
        return FText::Format(LOCTABLE(ITEM_STRINGS_KEY, G_USE_ON_SELF_DEFAULT_KEY), VerbArgs);
    }
    FText TargetText = HotSpot ? HotSpot->ShortDescription : TargetItem->ShortDescription;
    VerbArgs.Add("Object", TargetText);
    return FText::Format(LOCTABLE(ITEM_DESCRIPTIONS_KEY, G_USE_SUBJECT_ON_OBJECT_KEY), VerbArgs);
}

FText AdvGameUtils::GetVerbWithItemText(const UInventoryItem* CurrentItem, const EVerbType Verb)
{
    verifyf(CurrentItem, TEXT("GetVerbWithItemString expects CurrentItem to be non-null"));
    FText ItemText = CurrentItem->ShortDescription;
    if (ItemText.IsEmpty()) ItemText = FItemKind::GetDescription(CurrentItem->ItemKind);
    FFormatNamedArguments VerbArgs;
    VerbArgs.Add("Verb", VerbGetDescriptiveString(Verb));
    VerbArgs.Add("Subject", ItemText);
    return FText::Format(LOCTABLE(ITEM_DESCRIPTIONS_KEY, G_VERB_SUBJECT_KEY), VerbArgs);
}

FText AdvGameUtils::GetVerbWithHotSpotText(const AHotSpot* HotSpot, const EVerbType Verb)
{
    verifyf(HotSpot, TEXT("GetVerbWithHotSpotString expects HotSpot to be non-null"));
    FFormatNamedArguments VerbArgs;
    VerbArgs.Add("Verb", VerbGetDescriptiveString(Verb));
    VerbArgs.Add("Subject", HotSpot->ShortDescription);
    return FText::Format(LOCTABLE(ITEM_DESCRIPTIONS_KEY, G_VERB_SUBJECT_KEY), VerbArgs);
}

TArray<FText> AdvGameUtils::NewLineSeperatedToArrayText(const FText& NewText)
{
    FString Line = NewText.ToString();
    if (!Line.Contains(NEW_LINE_SEPARATOR)) return TArray<FText>({ NewText });
    TArray<FString> NewLines = NewLineSeperatedToArrayString(Line);
    TArray<FText> OutText;
    Algo::Transform(NewLines, OutText, [](const FString& Str){ return FText::FromString(Str); });
    return OutText;
}

TArray<FString> AdvGameUtils::NewLineSeperatedToArrayString(const FString& NewString)
{
    if (NewString.Contains(NEW_LINE_SEPARATOR))
    {
        TArray<FString> NewLines;
        FString First, Rest, Buf = NewString;
        while (Buf.Split(NEW_LINE_SEPARATOR, &First, &Rest))
        {
            NewLines.Add(First.TrimStartAndEnd());
            Buf = Rest;
        }
        NewLines.Add(Buf.TrimStartAndEnd());
        return NewLines;
    }
    return TArray({ NewString });
}

TArray<FText> AdvGameUtils::WrapTextLinesToMaxCharacters(const FText& NewText, const int32 MaxLength)
{
    TArray<FText> WrappedLines;
    FString CurrentLine = NewText.ToString();
    while (CurrentLine.Len() > MaxLength)
    {
        int32 SplitPoint = MaxLength;
        while (CurrentLine[--SplitPoint] != ' ' && SplitPoint > 0) {}
        if (SplitPoint < 0) SplitPoint = MaxLength; // Did not find a space to split at
        WrappedLines.Add(FText::FromString(CurrentLine.Left(SplitPoint)));
        CurrentLine = CurrentLine.Mid(SplitPoint).TrimStartAndEnd();
    }
    WrappedLines.Add(FText::FromString(CurrentLine));
    return WrappedLines;
}
