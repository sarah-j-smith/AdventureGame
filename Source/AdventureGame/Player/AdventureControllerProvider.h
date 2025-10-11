// (c) 2025 Sarah Smith

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "AdventureControllerProvider.generated.h"

class AAdventurePlayerController;
class AAdventureCharacter;


// This class does not need to be modified.
UINTERFACE(NotBlueprintable)
class UAdventureControllerProvider : public UInterface
{
    GENERATED_BODY()
};

/**
 * 
 */
class ADVENTUREGAME_API IAdventureControllerProvider
{
    GENERATED_BODY()

    // Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
    UFUNCTION(BlueprintCallable, Category = "AdventureController")
    virtual AAdventurePlayerController *GetAdventurePlayerController();

    UFUNCTION(BlueprintCallable, Category = "AdventureController")
    virtual AAdventureCharacter *GetAdventureCharacter();

    /// Player barks a message and then immediately ends any action sequence
    /// they were doing. Use when the blueprint event logic should end with a bark.
    /// @param Text FText for the player to bark. Should be translatable.
    UFUNCTION(BlueprintCallable, Category = "AdventureController")
    virtual void BarkAndEnd(FText Text);

    /// Player barks a message and continues on any action sequence
    /// they were doing. Use when the blueprint event logic should continue
    /// and block user interaction.
    /// @param Text FText for the player to bark. Should be translatable.
    UFUNCTION(BlueprintCallable, Category = "AdventureController")
    virtual void Bark(FText Text);
};
