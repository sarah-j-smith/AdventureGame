// (c) 2025 Sarah Smith


#include "AdvBlueprintFunctionLibrary.h"

#include "AdventureGameInstance.h"
#include "AdventureGame/Player/AdventurePlayerController.h"
#include "AdventureGame/Enums/ItemKind.h"
#include "AdventureGame/Constants.h"
#include "AdventureGame/HotSpots/HotSpot.h"
#include "AdventureGame/Player/AdventureCharacter.h"

#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"

AAdventurePlayerController* UAdvBlueprintFunctionLibrary::GetAdventureController(const UObject* WorldContextObject)
{
    if (const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject,
                                                                 EGetWorldErrorMode::LogAndReturnNull))
    {
        APlayerController* PlayerController = UGameplayStatics::GetPlayerController(World, 0);
        return Cast<AAdventurePlayerController>(PlayerController);
    }
    return nullptr;
}

void UAdvBlueprintFunctionLibrary::PlayerBark(const UObject* WorldContextObject, FText BarkText)
{
    if (AAdventurePlayerController* AdventurePlayerController = GetAdventureController(WorldContextObject))
    {
        AdventurePlayerController->PlayerBark(BarkText);
    }
}

void UAdvBlueprintFunctionLibrary::ClearVerb(const UObject* WorldContextObject)
{
    if (AAdventurePlayerController* AdventurePlayerController = GetAdventureController(WorldContextObject))
    {
        AdventurePlayerController->InterruptCurrentAction();
    }
}

int32 UAdvBlueprintFunctionLibrary::PIEInstance(const UObject* WorldContextObject)
{
    // https://blog.jamesbrooks.net/posts/pieinstance-in-unreal/
    const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);

    // If we are in the editor, return the PIE instance ID, otherwise return -1 (not in PIE and we won't be using this value anyway)
    if (World->IsEditorWorld())
    {
        // Get world context
        if (const FWorldContext* WorldContext = GEngine->GetWorldContextFromWorld(World))
        {
            return WorldContext->PIEInstance;
        }
    }
    return -1;
}

UInventoryItem* UAdvBlueprintFunctionLibrary::AddToInventory(const UObject* WorldContextObject,
                                                             EItemKind ItemToAdd)
{
    if (AAdventurePlayerController* AdventurePlayerController = GetAdventureController(WorldContextObject))
    {
        UInventoryItem* Item = AdventurePlayerController->ItemAddToInventory(ItemToAdd);
        return Item;
    }
    return nullptr;
}

void UAdvBlueprintFunctionLibrary::RemoveFromInventory(const UObject* WorldContextObject, EItemKind ItemToRemove)
{
    if (AAdventurePlayerController* AdventurePlayerController = GetAdventureController(WorldContextObject))
    {
        AdventurePlayerController->ItemRemoveFromInventory(ItemToRemove);
    }
}

FString UAdvBlueprintFunctionLibrary::GetProjectVersion()
{
    FString ProjectVersion;

    GConfig->GetString(
        TEXT("/Script/EngineSettings.GeneralProjectSettings"),
        TEXT("ProjectVersion"),
        ProjectVersion,
        GGameIni
    );

    return ProjectVersion;
}

float UAdvBlueprintFunctionLibrary::GetBarkTime(const FString& BarkText)
{
    int LineCount = 1;
    if (BarkText.Contains(NEW_LINE_SEPARATOR))
    {
        for (auto Element : BarkText)
        {
            if (Element == '\n')
            {
                ++LineCount;
            }
        }
    }
    float LineDelay = LineCount * BARK_LINE_DELAY;
    const int LetterCount = BarkText.Len();
    if (LetterCount <= SHORT_LETTER_COUNT) return LineDelay + SHORT_BARK_TIME;
    if (LetterCount <= MEDIUM_LETTER_COUNT) return LineDelay + MEDIUM_BARK_TIME;
    if (LetterCount <= LONG_LETTER_COUNT) return LineDelay + LONG_BARK_TIME;
    if (LetterCount <= EXTRA_LONG_LETTER_COUNT) return LineDelay + EXTRA_LONG_BARK_TIME;
    return LineDelay * 2.0;
}

UAdventureGameInstance* UAdvBlueprintFunctionLibrary::GetAdventureInstance(const UObject* WorldContextObject)
{
    if (UAdventureGameInstance* AdventureGameInstance = Cast<UAdventureGameInstance>(
        UGameplayStatics::GetGameInstance(WorldContextObject)))
    {
        return AdventureGameInstance;
    }
    UE_LOG(LogAdventureGame, Error, TEXT("Could not access Adventure Game Instance"));
    return nullptr;
}

bool UAdvBlueprintFunctionLibrary::IsCharacterCloseToHotSpot(AHotSpot* HotSpot, AAdventureCharacter* AdventureCharacter,
                                                             float Tolerance)
{
    if (HotSpot == nullptr)
    {
        UE_LOG(LogAdventureGame, Error, TEXT("HotSpot == nullptr"));
        return false;
    }
    if (AdventureCharacter == nullptr)
    {
        UE_LOG(LogAdventureGame, Error, TEXT("AdventureCharacter == nullptr"));
        return false;
    }
    const float Distance = FVector::Distance(HotSpot->WalkToPosition,
                             AdventureCharacter->GetCapsuleComponent()->GetComponentLocation());
    if (Distance < Tolerance)
    {
        UE_LOG(LogAdventureGame, Display, TEXT("Test passed"));
    }
    return Distance < Tolerance;
}

bool UAdvBlueprintFunctionLibrary::IsCharacterCloseToLocation(FVector Location, AAdventureCharacter* AdventureCharacter,
    float Tolerance)
{
    if (Tolerance <= std::numeric_limits<float>::epsilon())
    {
        UE_LOG(LogAdventureGame, Error, TEXT("Tolerance is too small - %f"), Tolerance);
        return false;
    }
    if (AdventureCharacter == nullptr)
    {
        UE_LOG(LogAdventureGame, Error, TEXT("AdventureCharacter == nullptr"));
        return false;
    }
    if (UCapsuleComponent *Capsule = AdventureCharacter->GetCapsuleComponent())
    {
        FVector WorldLocation = Capsule->GetComponentLocation();
        if (WorldLocation != FVector::ZeroVector)
        {
            return FVector::Distance(Location, WorldLocation) < Tolerance;
        }
        UE_LOG(LogAdventureGame, Warning, TEXT("Player position was (0, 0, 0)!"));
        return false;
    }
    UE_LOG(LogAdventureGame, Error, TEXT("Could not get player character capsule component!"));
    return false;
}
