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
        if (!IsValid(AdventurePlayerController))
        {
            // Could happen if the level is being torn down or a loading of a save game is in progress
            UE_LOG(LogAdventureGame, Warning, TEXT("Adventure player controller not available in %hs - %d"),
                   __FUNCTION__, __LINE__);
            return nullptr;
        }
        CachedAdventureController = AdventurePlayerController;
        return AdventurePlayerController;
    }
    // Programmer error - only add IItemManagerProvider interface to UObject subclasses
    UE_LOG(LogAdventureGame, Fatal,
           TEXT("%hs - %d IAdventureControllerProvider only supported in UObject subclasses!"),
           __FUNCTION__, __LINE__);
    return nullptr;
}

AAdventureCharacter *IAdventureControllerProvider::GetAdventureCharacter()
{
    if (const AAdventurePlayerController *AdventureController = GetAdventurePlayerController())
    {
        return AdventureController->PlayerCharacter;
    }
    // Could happen if the level is being torn down or a loading of a save game is in progress
    UE_LOG(LogAdventureGame, Warning, TEXT("Adventure player character not available in %hs - %d"),
           __FUNCTION__, __LINE__);
    return nullptr;
}


void IAdventureControllerProvider::BarkAndEnd(FText Text)
{
    if (AAdventurePlayerController *AdventureController = GetAdventurePlayerController())
    {
        AdventureController->PlayerBark(Text);
        if (AdventureController->Command)
            AdventureController->Command->bShouldInterruptCurrentActionOnNextTick = true;
    }
}

void IAdventureControllerProvider::Bark(FText Text)
{
    if (AAdventurePlayerController *AdventureController = GetAdventurePlayerController())
    {
        AdventureController->PlayerBark(Text);
    }
}