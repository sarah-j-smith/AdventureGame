// (c) 2025 Sarah Smith


#include "AdventureControllerProvider.h"

#include "AdventurePlayerController.h"
#include "AdventureGame/AdventureGame.h"

AAdventurePlayerController* IAdventureControllerProvider::GetAdventurePlayerController()
{
    static TWeakObjectPtr<AAdventurePlayerController> CachedAdventureController;
    if (AAdventurePlayerController* AdventureController = CachedAdventureController.Get()) return AdventureController;
    if (const UObject* WorldContextObject = dynamic_cast<UObject*>(this))
    {
        APlayerController* PlayerController = UGameplayStatics::GetPlayerController(WorldContextObject, 0);
        AAdventurePlayerController* AdventurePlayerController = Cast<AAdventurePlayerController>(PlayerController);
        if (AdventurePlayerController && IsValid(AdventurePlayerController))
        {
            CachedAdventureController = AdventurePlayerController;
            return AdventurePlayerController;
        }
        // Could happen if the level is being torn down or a loading of a save game is in progress
        UE_LOG(LogAdventureGame, Display, TEXT("Adventure player controller not available in %s"),
               *WorldContextObject->GetName());
        return nullptr;
    }
    // Programmer error - only add IItemManagerProvider interface to UObject subclasses
    UE_LOG(LogAdventureGame, Fatal,
           TEXT("%hs - %d IAdventureControllerProvider only supported in UObject subclasses!"),
           __FUNCTION__, __LINE__);
    return nullptr;
}

AAdventureCharacter *IAdventureControllerProvider::GetAdventureCharacter()
{
    // If there is an AdventureController then it should have a current handle to the AdventureCharacter
    // and if it doesn't then its probably not ready or available, so return it without checking.
    if (const AAdventurePlayerController *AdventureController = GetAdventurePlayerController())
    {
        return AdventureController->PlayerCharacter;
    }
    // If there is NOT an AdventureController then its likely because we are in testing mode, in which
    // case there is no AAdventurePlayerController at all. In that case an AAdventureCharacter reference
    // is likely to be set on the Command Manager so try to return that, but if it fails log an error.
    if (const UObject* WorldContextObject = dynamic_cast<UObject*>(this))
    {
        AActor* Actor = UGameplayStatics::GetActorOfClass(WorldContextObject, ACommandManager::StaticClass());
        if (ACommandManager* CommandManager = Cast<ACommandManager>(Actor))
        {
            if (CommandManager->PlayerCharacter)
            {
                return CommandManager->PlayerCharacter;
            }
        }
    }
        // Could happen if the level is being torn down or a loading of a save game is in progress
    UE_LOG(LogAdventureGame, Display, TEXT("Adventure player character not available in %hs - %d"),
           __FUNCTION__, __LINE__);
    return nullptr;
}
