// (c) 2025 Sarah Smith


#include "AdvBlueprintFunctionLibrary.h"

#include "AdventureGameInstance.h"
#include "AdventureGameModeBase.h"
#include "AdventureGame/Player/AdventurePlayerController.h"
#include "AdventureGame/Enums/ItemKind.h"
#include "AdventureGame/Constants.h"
#include "AdventureGame/HotSpots/HotSpot.h"
#include "AdventureGame/Player/AdventureCharacter.h"
#include "AdventureGame/Player/ItemManager.h"

#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"

void UAdvBlueprintFunctionLibrary::AddToScore(const UObject* WorldContextObject, int32 ScoreIncrement)
{
    if (const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject,
                                                             EGetWorldErrorMode::LogAndReturnNull))
    {
        AGameModeBase *GameMode = UGameplayStatics::GetGameMode(WorldContextObject);
        if (AAdventureGameModeBase *AdventureGameMode = Cast<AAdventureGameModeBase>(GameMode))
        {
            AdventureGameMode->AddToScore(ScoreIncrement);
        }
    }
}

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

ACommandManager* UAdvBlueprintFunctionLibrary::GetCommandManager(const UObject* WorldContextObject)
{
    AActor* Actor = UGameplayStatics::GetActorOfClass(WorldContextObject, ACommandManager::StaticClass());
    if (ACommandManager* CommandManager = Cast<ACommandManager>(Actor))
    {
        return CommandManager;
    }
    // Could happen if the level is being torn down or a loading of a save game is in progress
    UE_LOG(LogAdventureGame, Display, TEXT("%hs - %s Command manager not available in"),
        __FUNCTION__, *(WorldContextObject->GetName()));
    return nullptr;
}

UItemManager* UAdvBlueprintFunctionLibrary::GetItemManager(const UObject* WorldContextObject)
{
    if (const ACommandManager *CommandManager = GetCommandManager(WorldContextObject))
    {
        if (UItemManager *ItemManager = CommandManager->ItemManager)
        {
            return ItemManager;
        }
        // Could happen if the level is being torn down or a loading of a save game is in progress
        UE_LOG(LogAdventureGame, Display, TEXT("%hs - %s Item Manager not available in"),
            __FUNCTION__, *(WorldContextObject->GetName()));
    }
    return nullptr;
}

void UAdvBlueprintFunctionLibrary::PlayerBark(const UObject* WorldContextObject, FText BarkText)
{
    if (ACommandManager *CommandManager = GetCommandManager(WorldContextObject))
    {
        CommandManager->Bark(BarkText);
    }
}

void UAdvBlueprintFunctionLibrary::ClearVerb(const UObject* WorldContextObject)
{
    if (ACommandManager *CommandManager = GetCommandManager(WorldContextObject))
    {
        CommandManager->InterruptCurrentAction();
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
                                                             const EItemKind ItemToAdd)
{
    if (UItemManager *ItemManager = GetItemManager(WorldContextObject))
    {
        return ItemManager->ItemAddToInventory(ItemToAdd);
    }
    return nullptr;
}

void UAdvBlueprintFunctionLibrary::RemoveFromInventory(const UObject* WorldContextObject, EItemKind ItemToRemove)
{
    if (UItemManager *ItemManager = GetItemManager(WorldContextObject))
    {
        ItemManager->ItemRemoveFromInventory(ItemToRemove);
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
    return IsCharacterCloseToLocation(HotSpot->WalkToPosition, AdventureCharacter, Tolerance);
}

bool UAdvBlueprintFunctionLibrary::IsCharacterCloseToLocation(FVector Location, AAdventureCharacter* AdventureCharacter,
    float Tolerance)
{
    if (Tolerance <= std::numeric_limits<float>::epsilon())
    {
        UE_LOG(LogAdventureGame, Error, TEXT("Tolerance is too small - %f"), Tolerance);
        return false;
    }
    return GetCharacterDistanceToLocation(Location, AdventureCharacter) < Tolerance;
}

float UAdvBlueprintFunctionLibrary::GetCharacterDistanceToLocation(FVector Location, AAdventureCharacter* AdventureCharacter)
{
    if (AdventureCharacter == nullptr)
    {
        UE_LOG(LogAdventureGame, Error, TEXT("%hs: AdventureCharacter == nullptr"), __FUNCTION__);
        return std::numeric_limits<float>::infinity();
    }
    if (UCapsuleComponent *Capsule = AdventureCharacter->GetCapsuleComponent())
    {
        FVector WorldLocation = Capsule->GetComponentLocation();
        if (WorldLocation != FVector::ZeroVector)
        {
            WorldLocation.Z = Location.Z; // Ignore the Z, only test X & Y.
            return FVector::Distance(Location, WorldLocation);
        }
        UE_LOG(LogAdventureGame, Warning, TEXT("%hs: Player position was (0, 0, 0)!"), __FUNCTION__);
    }
    else
    {
        UE_LOG(LogAdventureGame, Warning, TEXT("%hs: Player capsule component null"), __FUNCTION__);
    }
    return std::numeric_limits<float>::infinity();
}
