// (c) 2025 Sarah Smith


#include "CommandManager.h"

#include "AdventureGame/HUD/AdventureGameHUD.h"
#include "AdventureGame/AdventureGame.h"
#include "AdventureGame/HotSpots/HotSpot.h"

#include "AdventureAIController.h"
#include "AdventureCharacter.h"
#include "AdventurePlayerController.h"
#include "InteractionNotifier.h"
#include "ItemManager.h"
#include "Puck.h"
#include "TestBarkController.h"

#include "GameFramework/PawnMovementComponent.h"
#include "Components/CapsuleComponent.h"

void ShowLocationDebug(float LocationX, float LocationY, const FString& LocationMessage)
{
#if WITH_EDITOR
    const FString Message = FString::Printf(TEXT("%s x: %f, y: %f"), *LocationMessage, LocationX, LocationY);
    GEngine->AddOnScreenDebugMessage(1, 5.0, FColor::Cyan,
                                     *Message, false, FVector2D(2.0, 2.0));
    UE_LOG(LogAdventureGame, Display, TEXT("%s"), *Message);
#endif
}

// Sets default values
ACommandManager::ACommandManager()
{
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;

    UE_LOG(LogAdventureGame, VeryVerbose, TEXT("Construct: ACommandManager"));

    InteractionNotifier = CreateDefaultSubobject<UInteractionNotifier>(TEXT("InteractionNotifier"));
    ItemManager = CreateDefaultSubobject<UItemManager>(TEXT("ItemManager"));
    PlayerBarkManager = CreateDefaultSubobject<UPlayerBarkManager>(TEXT("PlayerBark"));
}

// Called when the game starts or when spawned
void ACommandManager::BeginPlay()
{
    Super::BeginPlay();

    ConnectToMoveCompletedDelegate();
    if (bIsTesting && !bDisableHUD && AdventureGameHUD == nullptr)
    {
        SetupHUD();
    }
    if (bIsTesting && bDisablePlayerBarking)
    {
        PlayerBarkManager->DestroyComponent();
        TestBarkController = CreateDefaultSubobject<UTestBarkController>(TEXT("TestBarkController"));
    }
    
    UE_LOG(LogAdventureGame, VeryVerbose, TEXT("BeginPlay: ACommandManager"));
    if (!bDisableHUDUpdates) UpdateInteractionTextDelegate.Broadcast();
}

// Called every frame
void ACommandManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (ShouldInterruptCurrentActionOnNextTick())
    {
        InterruptCurrentAction();
        bShouldInterruptCurrentActionOnNextTick = false;
    }
    if (ShouldCompleteMovementNextTick)
    {
        HandleMovementComplete();
        ShouldCompleteMovementNextTick = false;
    }
}

void ACommandManager::SetInputLocked(bool bLocked)
{
    bInputLocked = bLocked;
}

bool ACommandManager::IsInputLocked() const
{
    return bInputLocked;
}

bool ACommandManager::ShouldHighlightInteractionText() const
{
    return CurrentCommand == EPlayerCommand::Active || CurrentCommand == EPlayerCommand::InstantActive;
}

void ACommandManager::HandleTouchInput()
{
    // Don't test input, start from HandleHotSpotClicked & HandleLocationClicked
    check(!bIsTesting); 
    InteractionNotifier->NotifyUserInteraction();

    if (IsInputLocked()) return;

    float LocationX, LocationY;
    AAdventurePlayerController* AdventurePlayerController = GetAdventurePlayerController();
    if (!AdventurePlayerController) return;
    if (!AdventurePlayerController->GetTouchPosition(LocationX, LocationY)) return;

    ShowLocationDebug(LocationX, LocationY, TEXT("Touch input"));

    if (AHotSpot* HotSpot = AdventurePlayerController->HotSpotTapped(LocationX, LocationY))
    {
        HandleHotSpotClicked(HotSpot);
    }
    else
    {
        FVector MouseWorldLocation, MouseWorldDirection;
        AdventurePlayerController->DeprojectScreenPositionToWorld(LocationX, LocationY, MouseWorldLocation,
                                                                  MouseWorldDirection);

        if (const AAdventureCharacter* PlayerCharacter = GetPlayerCharacter())
        {
            const FVector PlayerLocation = PlayerCharacter->GetCapsuleComponent()->GetComponentLocation();
            MouseWorldLocation.Z = PlayerLocation.Z;
        }
        HandleLocationClicked(MouseWorldLocation);
    }
}

void ACommandManager::HandlePointAndClickInput()
{
    // Don't test input, start from HandleHotSpotClicked & HandleLocationClicked
    check(!bIsTesting);
    
    InteractionNotifier->NotifyUserInteraction();

    AAdventurePlayerController* AdventurePlayerController = GetAdventurePlayerController();
    if (IsInputLocked() || !AdventurePlayerController || AdventurePlayerController->IsMouseOverUI()) return;

    if (AHotSpot* HotSpot = AdventurePlayerController->HotSpotClicked())
    {
        SetVerbAndCommandFromHotSpot(HotSpot);
        HandleHotSpotClicked(HotSpot);
    }
    else
    {
        float LocationX, LocationY;
        if (!AdventurePlayerController->GetMouseClickPosition(LocationX, LocationY)) return;

        ShowLocationDebug(LocationX, LocationY, TEXT("Click input"));

        FVector MouseWorldLocation, MouseWorldDirection;
        AdventurePlayerController->DeprojectScreenPositionToWorld(LocationX, LocationY, MouseWorldLocation,
                                                                  MouseWorldDirection);

        if (const AAdventureCharacter* PlayerCharacter = GetPlayerCharacter())
        {
            const FVector PlayerLocation = PlayerCharacter->GetCapsuleComponent()->GetComponentLocation();
            MouseWorldLocation.Z = PlayerLocation.Z;
        }
        HandleLocationClicked(MouseWorldLocation);
    }
}

void ACommandManager::HandleHotSpotClicked(AHotSpot* HotSpot)
{
    if (!IsValid(HotSpot)) return;

#if WITH_EDITOR
    UE_LOG(LogAdventureGame, VeryVerbose, TEXT("HandleHotSpotClicked - %s - command: %s"),
           *HotSpot->ShortDescription.ToString(),
           *UEnum::GetValueAsString(CurrentCommand));
#endif

    switch (CurrentCommand)
    {
    case EPlayerCommand::InstantActive:
        InterruptCurrentAction();
    case EPlayerCommand::None:
    case EPlayerCommand::Hover:
        CurrentHotSpot = HotSpot;
        ItemManager->SourceItem = nullptr;
        ItemManager->TargetItem = nullptr;
        PerformInstantAction();
        break;
    case EPlayerCommand::VerbPending:
    case EPlayerCommand::UsePending:
        CurrentCommand = EPlayerCommand::Active;
        CurrentHotSpot = HotSpot;
        ItemManager->ClearSourceItem();
        ItemManager->ClearTargetItem();
        if (!bDisableHUDUpdates) BeginAction.Broadcast();
        WalkToHotSpot(HotSpot);
        break;
    case EPlayerCommand::Targeting:
        CurrentCommand = EPlayerCommand::Active;
        CurrentHotSpot = HotSpot;
        ItemManager->ClearTargetItem();
        if (!bDisableHUDUpdates) BeginAction.Broadcast();
        WalkToHotSpot(HotSpot);
        break;
    default:
        break;
    }
}

void ACommandManager::HandleLocationClicked(const FVector& Location)
{
#if WITH_EDITOR
    UE_LOG(LogAdventureGame, VeryVerbose, TEXT("HandleLocationClicked - %s - command: %s"), *Location.ToString(),
           *UEnum::GetValueAsString(CurrentCommand));
#endif
    CurrentTargetLocation = Location;
    switch (CurrentCommand)
    {
    case EPlayerCommand::InstantActive:
        InterruptCurrentAction();
    case EPlayerCommand::None:
    case EPlayerCommand::Hover:
        ItemManager->ClearSourceItem();
        ItemManager->ClearTargetItem();
        PerformInstantAction();
        break;
    default:
        break;
    }
}

void ACommandManager::MouseEnterHotSpot(AHotSpot* HotSpot)
{
    if (CanBrowseHotspot())
    {
        CurrentHotSpot = HotSpot;
        if (!bDisableHUDUpdates)
        {
            if (UpdateInteractionTextDelegate.IsBound())
            {
                UpdateInteractionTextDelegate.Broadcast();
                UE_LOG(LogAdventureGame, Display, TEXT("Broadcasting to UpdateInteractionTextDelegate"))
            }
        }
        else
        {
            UE_LOG(LogAdventureGame, VeryVerbose, TEXT("MouseEnterHotSpot but bDisableHUDUpdates set: ignoring"));
        }
    }
}

void ACommandManager::MouseLeaveHotSpot()
{
    if (CanBrowseHotspot() && CurrentHotSpot)
    {
        CurrentHotSpot = nullptr;
        if (!bDisableHUDUpdates) UpdateInteractionTextDelegate.Broadcast();
    }
}

void ACommandManager::PerformInstantAction()
{
#if WITH_EDITOR
    FString DebugString;
    if (ItemManager->SourceItem) DebugString = ItemManager->SourceItem->ShortDescription.ToString();
    if (CurrentHotSpot && DebugString.IsEmpty()) DebugString = CurrentHotSpot->ShortDescription.ToString();
    UE_LOG(LogAdventureGame, Display, TEXT("PerformInstantAction %s - %s"),
           *VerbGetDescriptiveString(CurrentVerb).ToString(), *DebugString);
#endif

    CurrentCommand = EPlayerCommand::InstantActive;
    if (ItemManager->SourceItem)
    {
        // Clicking on something in your own inventory
        UInventoryItem::Execute_OnLookAt(ItemManager->SourceItem);
        ItemManager->UpdateInventoryText();
    }
    else if (CurrentHotSpot)
    {
        WalkToHotSpot(CurrentHotSpot);
        if (!bDisableHUDUpdates) UpdateInteractionTextDelegate.Broadcast();
    }
    else
    {
        WalkToLocation(CurrentTargetLocation);
        if (!bDisableHUDUpdates) UpdateInteractionTextDelegate.Broadcast();
    }
    if (!bDisableHUDUpdates) BeginAction.Broadcast();
}

void ACommandManager::PerformHotSpotInteraction()
{
    UE_LOG(LogAdventureGame, Verbose, TEXT("PerformHotSpotInteraction - verb %s for hotspot %s"),
           *VerbGetDescriptiveString(CurrentVerb).ToString(), *CurrentHotSpot->ShortDescription.ToString());
    // This `Execute_Verb` pattern will call C++ and Blueprint overrides.
    // The use of eg CurrentHotSpot->OnClose() does not work as BP's don't do
    // polymorphism and have to be dispatched in code.
    check(CurrentHotSpot);
    switch (CurrentVerb)
    {
    case EVerbType::Close:
        AHotSpot::Execute_OnClose(CurrentHotSpot);
        break;
    case EVerbType::Open:
        AHotSpot::Execute_OnOpen(CurrentHotSpot);
        break;
    case EVerbType::Give:
        AHotSpot::Execute_OnGive(CurrentHotSpot);
        break;
    case EVerbType::GiveItem:
        AHotSpot::Execute_OnItemGiven(CurrentHotSpot);
        break;
    case EVerbType::LookAt:
        AHotSpot::Execute_OnLookAt(CurrentHotSpot);
        break;
    case EVerbType::PickUp:
        AHotSpot::Execute_OnPickUp(CurrentHotSpot);
        break;
    case EVerbType::TalkTo:
        AHotSpot::Execute_OnTalkTo(CurrentHotSpot);
        break;
    case EVerbType::Pull:
        AHotSpot::Execute_OnPull(CurrentHotSpot);
        break;
    case EVerbType::Push:
        AHotSpot::Execute_OnPush(CurrentHotSpot);
        break;
    case EVerbType::Use:
        AHotSpot::Execute_OnUse(CurrentHotSpot);
        break;
    case EVerbType::UseItem:
        AHotSpot::Execute_OnItemUsed(CurrentHotSpot);
        break;
    case EVerbType::WalkTo:
        AHotSpot::Execute_OnWalkTo(CurrentHotSpot);
    default:
        break;
    }
}

void ACommandManager::ConnectToMoveCompletedDelegate()
{
    if (AAdventureAIController *AdventureAIController = GetAIController())
    {
        AdventureAIController->MoveCompletedDelegate.AddDynamic(
            this, &ACommandManager::HandleAIMovementCompleteNotify);
    }
}

void ACommandManager::HandleAIMovementCompleteNotify(EPathFollowingResult::Type Result)
{
    UE_LOG(LogAdventureGame, VeryVerbose, TEXT("HandleAIMovementCompleteNotify"));
    if (Result == EPathFollowingResult::Success)
    {
        if (AIStatus == EAIStatus::MakingRequest)
        {
            AIStatus = EAIStatus::AlreadyThere;
        }
        LastPathResult = EAIMoveResult::Success;
        ShouldCompleteMovementNextTick = true;
    }
    else
    {
        if (CurrentHotSpot && (Result == EPathFollowingResult::Blocked || Result == EPathFollowingResult::OffPath))
        {
            FVector HotSpotLocation = CurrentHotSpot->WalkToPoint->GetComponentLocation();
#if WITH_EDITOR
            const FString Message = FString::Printf(
                TEXT("Movement blocked to %s - %s - is walk to point on the nav mesh?"),
                *(CurrentHotSpot->ShortDescription.ToString()), *(HotSpotLocation.ToString()));
            GEngine->AddOnScreenDebugMessage(1, 5.0, FColor::Cyan,
                                             *Message, false, FVector2D(2.0, 2.0));
            UE_LOG(LogAdventureGame, Warning, TEXT("%s"), *Message);
#endif
        }
    }
}

void ACommandManager::HandleMovementComplete()
{
    AAdventureCharacter* PlayerCharacter = GetPlayerCharacter();
    check(PlayerCharacter);
    UE_LOG(LogAdventureGame, VeryVerbose, TEXT("HandleMovementComplete"));
    AIStatus = EAIStatus::Idle;
    if (CurrentHotSpot && LastPathResult == EAIMoveResult::Success)
    {
        UE_LOG(LogAdventureGame, VeryVerbose, TEXT("CurrentHotSpot && (LastPathResult == EAIMoveResult::Success)"));
        PlayerCharacter->SetFacingDirection(CurrentHotSpot->FacingDirection);
        PlayerCharacter->TeleportToLocation(CurrentHotSpot->WalkToPosition);
        PerformHotSpotInteraction();
        return;
    } else if (!TargetLocationForAI.IsZero() && LastPathResult == EAIMoveResult::Success)
    {
        UE_LOG(LogAdventureGame, VeryVerbose, TEXT("TargetLocationForAI && (LastPathResult == EAIMoveResult::Success)"));
        PlayerCharacter->TeleportToLocation(TargetLocationForAI);
    }
    InterruptCurrentAction();
}

void ACommandManager::WalkToLocation(const FVector& Location)
{
    StopAIMovement();
    if (AIStatus != EAIStatus::Idle) return;
    TargetLocationForAI = Location;

#if WITH_EDITOR
    if (bTeleportInsteadOfWalk)
    {
        TeleportToLocation(Location);
        return;
    }
#endif

    if (AAdventureAIController* AI = GetAIController())
    {
        AIStatus = EAIStatus::MakingRequest;
        switch (AI->MoveToLocation(Location, 1.0))
        {
        case EPathFollowingRequestResult::Type::Failed:
            UE_LOG(LogAdventureGame, VeryVerbose, TEXT("Path following request -> failed: %f %f"), Location.X, Location.Y);
            LastPathResult = EAIMoveResult::Fail;
            break;
        case EPathFollowingRequestResult::Type::RequestSuccessful:
            UE_LOG(LogAdventureGame, VeryVerbose, TEXT("Path following request -> success: %f %f"), Location.X, Location.Y);
            LastPathResult = EAIMoveResult::Moving;
            AIStatus = EAIStatus::Moving;
            break;
        case EPathFollowingRequestResult::Type::AlreadyAtGoal:
            UE_LOG(LogAdventureGame, VeryVerbose, TEXT("Path following request -> already there: %f %f"), Location.X,
                   Location.Y);
            break;
        }
    }
}

void ACommandManager::WalkToHotSpot(AHotSpot* HotSpot)
{
    const AAdventureCharacter* PlayerCharacter = GetPlayerCharacter();
    const UCapsuleComponent* Capsule = PlayerCharacter->GetCapsuleComponent();
    const FVector PlayerLocation = Capsule->GetComponentLocation();

    FVector HotSpotWalkToLocation = HotSpot->WalkToPosition;
    HotSpotWalkToLocation.Z = PlayerLocation.Z;

    CurrentHotSpot = nullptr;
    float Distance = FVector::Distance(HotSpotWalkToLocation, PlayerLocation);
    if (Distance < Capsule->GetScaledCapsuleRadius())
    {
        // Character is there already, or very close to. Teleport to location and carry on.
        TeleportToLocation(HotSpotWalkToLocation);
        CurrentHotSpot = HotSpot;
        return;
    }
    WalkToLocation(HotSpotWalkToLocation);
    switch (AIStatus)
    {
    case EAIStatus::Moving:
    case EAIStatus::AlreadyThere:
        // Don't set the hotspot unless we know the player can reach it.
        CurrentHotSpot = HotSpot;
    default:
        break;
    }
}

void ACommandManager::CommenceConversation()
{
    SetInputLocked(true);
    InteractionNotifier->PromptListOpenRequest.Broadcast();
}

void ACommandManager::EndConversation()
{
    SetInputLocked(false);
    InteractionNotifier->PromptListCloseRequest.Broadcast();
}

void ACommandManager::AssignVerb(EVerbType NewVerb)
{
    ItemManager->ClearSourceItem();
    ItemManager->ClearTargetItem();
    CurrentVerb = NewVerb;
    switch (NewVerb)
    {
    case EVerbType::Use:
        CurrentCommand = EPlayerCommand::UsePending;
        break;
    case EVerbType::Give:
        CurrentCommand = EPlayerCommand::GivePending;
        break;
    case EVerbType::WalkTo:
        break;
    default:
        CurrentCommand = EPlayerCommand::VerbPending;
    }
    if (!bDisableHUDUpdates) UpdateInteractionTextDelegate.Broadcast();
}

void ACommandManager::InterruptCurrentAction()
{
    UE_LOG(LogAdventureGame, VeryVerbose, TEXT("InterruptCurrentAction"));
    SetInputLocked(false);
    TargetLocationForAI = FVector::ZeroVector;
    if (AAdventureCharacter* PlayerCharacter = GetPlayerCharacter())
    {
        PlayerCharacter->GetMovementComponent()->StopActiveMovement();
    }
    CurrentVerb = EVerbType::WalkTo;
    CurrentCommand = EPlayerCommand::None;
    CurrentHotSpot = nullptr;
    ItemManager->Reset();
    if (!bDisableHUDUpdates) InterruptAction.Broadcast();
}

void ACommandManager::ClearCurrentPath()
{
    if (AAdventureAIController* AIController = GetAIController())
    {
        AIController->StopMovement();
    }
}

void ACommandManager::TeleportToLocation(const FVector& Location)
{
    AAdventureCharacter* PlayerCharacter = GetPlayerCharacter();
    FVector Dest = Location;
    Dest.Z = PlayerCharacter->GetCapsuleComponent()->GetComponentLocation().Z;
    PlayerCharacter->TeleportToLocation(Dest);
    LastPathResult = EAIMoveResult::Success;
    AIStatus = EAIStatus::AlreadyThere;
    ShouldCompleteMovementNextTick = true;
}

void ACommandManager::SetVerbAndCommandFromHotSpot(AHotSpot* HotSpot)
{
    // If the player has not selected a verb, but has clicked on a hotspot, polymorphically
    // check with the hotspot for a default command that the player might expect, such as "Look"
    // or "Open" for a closed door, or "Use" for an open door, or "Talk to" for an NPC.
    if (CurrentCommand != EPlayerCommand::None && CurrentCommand != EPlayerCommand::Hover) return;

    // Player clicked on a hotspot without specifying a verb first.
    if (CurrentVerb != EVerbType::WalkTo)
    {
        // If CurrentCommand is None/Hover then the verb _should be_ the default
        UE_LOG(LogAdventureGame, Warning, TEXT("HotSpot %s clicked with no command but verb %s unexpectedly set!!"),
               *HotSpot->ShortDescription.ToString(), *VerbGetDescriptiveString(CurrentVerb).ToString());
    }
    CurrentVerb = HotSpot->CheckForDefaultCommand();
    if (CurrentVerb != EVerbType::WalkTo)
    {
        CurrentCommand = CurrentVerb == EVerbType::Use ? EPlayerCommand::UsePending : EPlayerCommand::VerbPending;
    }
}

void ACommandManager::StopAIMovement()
{
    AAdventureCharacter* PlayerCharacter = GetPlayerCharacter();
    if (!IsValid(PlayerCharacter)) return;
    AAdventureAIController* AI = Cast<AAdventureAIController>(PlayerCharacter->GetController());
    if (!IsValid(AI))
    {
        UE_LOG(LogAdventureGame, VeryVerbose, TEXT("PlayerCharacter controller expected to be AIController"));
        return;
    }
    if (AIStatus == EAIStatus::Moving)
    {
        AI->StopMovement();
        PlayerCharacter->GetMovementComponent()->StopActiveMovement();
        AIStatus = EAIStatus::Idle;
    }
}

void ACommandManager::ConnectToPlayerHUD(UAdventureGameHUD* AdventureGameHUD)
{
    AdventureGameHUD->BindCommandHandlers(this);
    AdventureGameHUD->BlackScreen->SetVisibility(ESlateVisibility::Hidden);
}

void ACommandManager::SetupHUD()
{
    /////
    //// TESTING
    ////
    //// THIS FUNCTION IS FOR TESTING ONLY - IT EXISTS TO HELP VISUALISE TEST OUTPUT
    ///
    check(bIsTesting);
    check(!bDisableHUDUpdates);
    check(!bDisableHUD);
    APlayerController *PlayerController = UGameplayStatics::GetPlayerController(this, 0);
    AdventureGameHUD = UAdventureGameHUD::Create(PlayerController, AdventureHUDClass);
    AdventureGameHUD->AddToViewport();
    ConnectToPlayerHUD(AdventureGameHUD);
    if (PlayerBarkManager && IsValid(PlayerBarkManager))
    {
        PlayerBarkManager->SetAdventureGameHUD(AdventureGameHUD);
    }
    if (UAdventureGameInstance *AdventureGameInstance = GetAdventureGameInstance())
    {
        AdventureGameHUD->BindInventoryHandlers(AdventureGameInstance);
    }
}

UPlayerBarkManager* ACommandManager::GetBarkController() const
{
    if (TestBarkController) return TestBarkController;
    return PlayerBarkManager;
}

AAdventureCharacter* ACommandManager::GetPlayerCharacter()
{
    if (PlayerCharacter) return PlayerCharacter;
    if (bIsTesting)
    {
        ///   In testing we drag a CommandManager and an AdventureCharacter into the test level, and
        ///   hook them up manually so that the this->PlayerCharacter is set already. 
        UE_LOG(LogAdventureGame, Error, TEXT("PlayerCharacter is not set during testing!"));
        return nullptr;
    }
    return GetAdventureCharacter();
}

AAdventureAIController* ACommandManager::GetAIController()
{
    /// IMPORTANT NOTE:
    ///   In normal game play the AdventurePlayerController (APC) will drive the setup of these
    ///   handlers in its BeginPlay() function. But in functional tests the APC does not exist.
    if (const AAdventureCharacter* PlayerCharacter = GetPlayerCharacter())
    {
        AAdventureAIController* AI = Cast<AAdventureAIController>(PlayerCharacter->GetController());
        if (!AI)
        {
            UE_LOG(LogAdventureGame, Warning, TEXT("Player character expected to have AI controller - got %s"),
                *(AI == nullptr ? TEXT("NULL") : AI->GetName()));
        }
        return AI;
    }
    return nullptr;
}

UAdventureGameInstance* ACommandManager::GetAdventureGameInstance()
{
    if (UAdventureGameInstance* AdventureGameInstance = Cast<UAdventureGameInstance>(
    UGameplayStatics::GetGameInstance(this)))
    {
        return AdventureGameInstance;
    }
    UE_LOG(LogAdventureGame, Warning, TEXT("Could not get Adventure Game Instance"));
    return nullptr;
}

void ACommandManager::UpdateMouseOverUI(const bool NewMouseIsOverUI)
{
    if (NewMouseIsOverUI)
    {
        if (CurrentVerb == EVerbType::WalkTo && CurrentCommand == EPlayerCommand::None)
        {
            CurrentVerb = EVerbType::LookAt;
            if (!bDisableHUDUpdates) UpdateInteractionTextDelegate.Broadcast();
        }
    }
    else
    {
        if (CurrentVerb == EVerbType::LookAt && CurrentCommand == EPlayerCommand::None)
        {
            CurrentVerb = EVerbType::WalkTo;
            if (!bDisableHUDUpdates) UpdateInteractionTextDelegate.Broadcast();
        }
    }
}

void ACommandManager::AddInputHandlers(APuck* Puck)
{
    if (bDisableInputSystem) return;
    Puck->PointAndClickDelegate.AddUObject(this, &ACommandManager::HandlePointAndClickInput);
    Puck->TouchInputDelegate.AddUObject(this, &ACommandManager::HandleTouchInput);
}

void ACommandManager::AddUIHandlers(UAdventureGameHUD *AdventureGameHUD)
{
    if (!bDisableHUD) AdventureGameHUD->VerbsUI->OnVerbChanged.BindDynamic(this, &ACommandManager::AssignVerb);
    if (!bDisablePlayerBarking) PlayerBarkManager->SetAdventureGameHUD(AdventureGameHUD);
}

void ACommandManager::HandleInventoryItemClicked(UItemSlot* ItemSlot)
{
    if (!ItemSlot->HasItem)
    {
        // clicking an empty inventory slot clears everything out
        InterruptCurrentAction();
        return;
    }
    if (!ItemManager->MaybeHandleInventoryItemClicked(ItemSlot)) return;

    // This handler is only called if `HasItem` is true
    switch (CurrentCommand)
    {
    case EPlayerCommand::None:
    case EPlayerCommand::Hover:
        ItemManager->SetAndLockSourceItem(ItemSlot->InventoryItem);
        PerformInstantAction();
        break;
    case EPlayerCommand::VerbPending:
        ItemManager->SetAndLockSourceItem(ItemSlot->InventoryItem);
        CurrentCommand = EPlayerCommand::Active;
        ItemManager->PerformItemAction(CurrentVerb);
        if (!bDisableHUDUpdates) BeginAction.Broadcast();
        break;
    case EPlayerCommand::UsePending:
    case EPlayerCommand::GivePending:
        ItemManager->SetAndLockSourceItem(ItemSlot->InventoryItem);
        CurrentVerb = CurrentCommand == EPlayerCommand::GivePending ? EVerbType::GiveItem : EVerbType::UseItem;
        CurrentCommand = EPlayerCommand::Targeting;
        ItemManager->ClearTargetItem(); // Should be clear already
        ItemManager->UpdateInventoryText();
        break;
    case EPlayerCommand::Targeting:
        if (ItemManager->CanInteractWith(ItemSlot->InventoryItem->ItemKind))
        {
            ItemManager->SetAndLockTargetItem(ItemSlot->InventoryItem);
            CurrentCommand = EPlayerCommand::Active;
            ItemManager->PerformItemInteraction(CurrentVerb);
            if (!bDisableHUDUpdates) BeginAction.Broadcast();
        }
    default:
        UE_LOG(LogAdventureGame, Warning, TEXT("Ignoring inventory click"));
    }
}
