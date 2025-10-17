// (c) 2025 Sarah Smith

#pragma once

#include "CoreMinimal.h"
#include "CommandManager.h"

#include "../Enums/PlayerCommand.h"
#include "../Enums/ChoiceState.h"
#include "../Enums/InteractionType.h"
#include "../Enums/VerbType.h"
#include "../Items/InventoryItem.h"
#include "../Enums/SaveGameStatus.h"

#include "AdventureGame/Constants.h"
#include "AdventureGame/Gameplay/AdventureGameInstance.h"

#include "GameFramework/PlayerController.h"
#include "AdventurePlayerController.generated.h"

class AAdventureCharacter;

namespace EPathFollowingResult
{
	enum Type : int;
}

class UItemSlot;
class UAdventureGameHUD;
class AHotSpot;
class APuck;

DECLARE_DELEGATE(FRunInterruptedActionDelegate);

DECLARE_MULTICAST_DELEGATE(FUserInteraction);

DECLARE_MULTICAST_DELEGATE(FBeginAction);
DECLARE_MULTICAST_DELEGATE(FInterruptAction);
DECLARE_MULTICAST_DELEGATE(FUpdateSaveGameText);
DECLARE_MULTICAST_DELEGATE_TwoParams(FUpdateSaveGameIndicator, ESaveGameStatus /* SaveStatus */, bool /* Success */);

DECLARE_MULTICAST_DELEGATE_ThreeParams(FEndAction, EInteractionType /* Interaction */, int32 /* UID */, bool /* Completed */);

/**
 * 
 */
UCLASS()
class ADVENTUREGAME_API AAdventurePlayerController : public APlayerController
{
	GENERATED_BODY()
public:

	AAdventurePlayerController();
	
	//////////////////////////////////
	///
	/// EVENT HANDLERS
	///
	
	FEndAction EndAction;

	virtual void BeginPlay() override;

	//////////////////////////////////
	///
	/// SAVE AND LOAD GAME
	///

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Save Game")
	FString DefaultSaveGameName = SAVE_GAME_NAME;

private:
	UFUNCTION()
	void OnSaveGameComplete(const FString &SlotName, const int32 UserIndex, bool Success);

	UFUNCTION()
	void OnLoadGameComplete(const FString &SlotName, const int32 UserIndex, USaveGame* LoadedGame);

	UFUNCTION(BlueprintCallable, Category="Save Game")
	void HandleSaveGame(const FString& GameName);

	UFUNCTION(BlueprintCallable, Category="Save Game")
	void HandleLoadGame(const FString& GameName);

	FAsyncSaveGameToSlotDelegate SaveGameToSlotDelegate;
	FAsyncLoadGameFromSlotDelegate LoadGameFromSlotDelegate;
	
public:	
	FUpdateSaveGameIndicator UpdateSaveGameIndicator;

	ESaveGameStatus SaveGameStatus = ESaveGameStatus::UpToDate;
	
	//////////////////////////////////
	///
	/// CLICK POSITION
	///
	
	bool GetMouseClickPosition(float &LocationX, float &LocationY);

	bool GetTouchPosition(float &LocationX, float &LocationY);
	
private:
	FVector2D LastMouseClick = FVector2D(std::numeric_limits<float>::max(), std::numeric_limits<float>::max());

public:
	//////////////////////////////////
	///
	/// COMMAND STATE
	///

	/// Player input, commands and state management are all delegated to this instance
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Commands")
	ACommandManager *Command = nullptr;

	/// <code>CommandManager</code> sub-class to spawn, if one is not found in the scene
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Commands")
	TSubclassOf<ACommandManager> CommandManagerToSpawn = ACommandManager::StaticClass();

	void SetupCommandManager();
	
	/// Has the player currently issued a command?
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Commands")
	EPlayerCommand CurrentCommand = EPlayerCommand::None;

	FUpdateInteractionText UpdateSaveGameIndicatorDelegate;
	
	
	//////////////////////////////////
	///
	/// ANIMATIONS
	///

	void PlayerClimb(int32 UID, EInteractTimeDirection InteractDirection);
	
	void PlayerInteract(int32 UID, EInteractTimeDirection InteractDirection);
	
	void PlayerSit(int32 UID, EInteractTimeDirection InteractDirection);

	/// Turn from front facing to the left, or left facing to the front if reversed.
	void PlayerTurnLeft(int32 UID, EInteractTimeDirection InteractDirection);

	/// Turn from front facing to the right, or right facing to the front if reversed.
	void PlayerTurnRight(int32 UID, EInteractTimeDirection InteractDirection);
	
private:
	UFUNCTION()
	void OnPlayerAnimationComplete(EInteractionType Interaction, bool Complete);
	
	UFUNCTION()
	void OnPlayerInteractComplete(bool Complete);
	int32 PlayerInteractUID = 0;
	
	UFUNCTION()
	void OnPlayerSitComplete(bool Complete);
	int32 PlayerSitUID = 0;

	UFUNCTION()
	void OnPlayerClimbComplete(bool Complete);
	int32 PlayerClimbUID = 0;

	UFUNCTION()
	void OnPlayerTurnLeftComplete(bool Complete);

	UFUNCTION()
	void OnPlayerTurnRightComplete(bool Complete);
	int32 PlayerTurnUID = 0;

	void EndTaskAction(EInteractionType InteractionType, int32 UID, bool Complete);

public:
	//////////////////////////////////
	///
	/// DETECT HOTSPOT INTERACTION
	///
	
	/// Get the Hotspot under the click location, or null if no hotspot was found
	AHotSpot *HotSpotClicked();

	/// Get the Hotspot under the tap location, or null if no hotspot was found
	AHotSpot *HotSpotTapped(float LocationX, float LocationY);
	
	//////////////////////////////////
	///
	/// INITIALISATION
	///

	/// The current `AAdventureCharacter` managed by this adventure controller
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	AAdventureCharacter *PlayerCharacter;

	/// An object of this class will be spawned and will be controlled by the AI pathfinder
	/// then the player will follow this puck. This is needed because the player character cannot
	/// receive input whilst possessed by the AI.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay")
	TSubclassOf<APuck> PuckClassToSpawn;
	
	APawn *SetupPuck(AAdventureCharacter *PlayerCharacter);

	void SetupAIController(APawn *AttachToPawn);
	
	UPROPERTY(EditAnywhere)
	TSubclassOf<UAdventureGameHUD> AdventureHUDClass;

	void SetupHUD();

	void SetupAnimationDelegates();
	
	//////////////////////////////////
	///
	/// HUD WIDGET MANAGEMENT
	/// 

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	UAdventureGameHUD *AdventureHUDWidget;

	bool IsMouseOverUI() const
	{
		return bIsMouseOverUI;
	}

	void SetMouseOverUI(bool IsMouseOverUI)
	{
		if (IsMouseOverUI == this->bIsMouseOverUI) return;
		UpdateMouseOverUI(IsMouseOverUI);
	}
	
private:
	bool bIsMouseOverUI = false;

	void UpdateMouseOverUI(bool NewMouseIsOverUI);
};
