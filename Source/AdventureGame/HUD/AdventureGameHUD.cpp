// (c) 2025 Sarah Smith


#include "AdventureGameHUD.h"

#include "AdventureGame/Constants.h"
#include "AdventureGame/AdventureGame.h"

#include "Kismet/GameplayStatics.h"
#include "AdvGameUtils.h"
#include "AdventureGame/Gameplay/AdventureGameInstance.h"
#include "AdventureGame/HotSpots/HotSpot.h"

#include "AdventureGame/Player/ItemManager.h"
#include "AdventureGame/Player/InteractionNotifier.h"
#include "AdventureGame/Player/CommandManager.h"
#include "Internationalization/StringTableRegistry.h"

void UAdventureGameHUD::NativeOnInitialized()
{
    if (UGameplayStatics::GetPlatformName() == "IOS" || UGameplayStatics::GetPlatformName() == "Android")
    {
        IsMobileTouch = true;
    }

    UE_LOG(LogAdventureGame, VeryVerbose, TEXT("UAdventureGameHUD::NativeOnInitialized"));
}

UAdventureGameHUD* UAdventureGameHUD::Create(APlayerController *PlayerController, TSubclassOf<UAdventureGameHUD> AdventureHUDClass)
{
    check(AdventureHUDClass);
    UAdventureGameHUD *AdventureHUDWidget = CreateWidget<UAdventureGameHUD>(PlayerController, AdventureHUDClass, TEXT("Adventure HUD"));
    if (AdventureHUDWidget)
    {
        AdventureHUDWidget->AddToViewport();
        UE_LOG(LogAdventureGame, VeryVerbose, TEXT("     AAdventureGameModeBase::SetupHUD - AddToViewport"));
    }
    return AdventureHUDWidget;
}

void UAdventureGameHUD::BindCommandHandlers(ACommandManager *CommandManager)
{
    if (CommandManager)
    {
        CommandManager->BeginAction.AddUObject(this, &UAdventureGameHUD::BeginActionEvent);
        CommandManager->UpdateInteractionTextDelegate.AddUObject(this, &UAdventureGameHUD::UpdateInteractionTextEvent);
        CommandManager->InterruptAction.AddUniqueDynamic(this, &UAdventureGameHUD::InterruptActionEvent);
        UE_LOG(LogAdventureGame, Display, TEXT("UAdventureGameHUD::BindCommandHandlers succeeded"));
        if (UInteractionNotifier *Notifier = CommandManager->InteractionNotifier)
        {
            Notifier->UserInteraction.AddUObject(this, &UAdventureGameHUD::OnUserInteracted);
            Notifier->PromptListOpenRequest.AddUObject(this, &UAdventureGameHUD::ShowPromptList);
            Notifier->PromptListCloseRequest.AddUObject(this, &UAdventureGameHUD::HidePromptList);
            UE_LOG(LogAdventureGame, Display, TEXT("UserInteraction & PromptList bind succeeded"));
        }
        if (UItemManager *ItemManager = CommandManager->ItemManager)
        {
            ItemManager->UpdateInventoryTextDelegate.AddUObject(this, &UAdventureGameHUD::UpdateInventoryTextEvent);
            UE_LOG(LogAdventureGame, Display, TEXT("UpdateInventoryTextDelegate bind succeeded"));
        }
        else
        {
            UE_LOG(LogAdventureGame, Warning, TEXT("Null CommandManager in BindCommandHandlers."))
        }
    }
    else
    {
        UE_LOG(LogAdventureGame, Warning, TEXT("Null CommandManager in BindCommandHandlers."))
    }
}

void UAdventureGameHUD::BindInventoryHandlers(UAdventureGameInstance* AdventureGameInstance)
{
    AdventureGameInstance->PlayerInventoryChanged.AddUniqueDynamic(this, &UAdventureGameHUD::HandleInventoryChanged);
}

void UAdventureGameHUD::ShowBlackScreen()
{
    BlackScreen->SetVisibility(ESlateVisibility::Visible);
}

void UAdventureGameHUD::HideBlackScreen()
{
    BlackScreen->SetVisibility(ESlateVisibility::Hidden);
}

void UAdventureGameHUD::SetInteractionText()
{
    const ACommandManager *Command = GetCommandManager();
    const UItemManager *ItemManager = GetItemManager();
    if (!Command || !ItemManager) return;
    auto Verb = Command->CurrentVerb;
    const UInventoryItem* SourceItem = ItemManager->SourceItem;
    if ((Verb == EVerbType::GiveItem || Verb == EVerbType::UseItem) && SourceItem == nullptr)
    {
        UE_LOG(LogAdventureGame, Warning, TEXT("Tried to %s with no source item"),
               *VerbGetDescriptiveString(Verb).ToString());
        return;
    }
    if (AHotSpot *CurrentHotspot = Command->CurrentHotSpot)
    {
        FText InteractionText;
        switch (Verb)
        {
        case EVerbType::UseItem:
            InteractionText = AdvGameUtils::GetUsingItemText(SourceItem, nullptr, CurrentHotspot);
            break;
        case EVerbType::Give:
            // Can't just give a hotspot - have to give _something_ to the hotspot - eg GiveItem
            InteractionText = LOCTABLE(ITEM_STRINGS_KEY, "GiveDefaultText");
            break;
        case EVerbType::GiveItem:
            InteractionText = AdvGameUtils::GetGivingItemText(SourceItem, nullptr, CurrentHotspot);
            break;
        default:
            InteractionText = AdvGameUtils::GetVerbWithHotSpotText(CurrentHotspot, Verb);
        }
        InteractionUI->SetText(InteractionText);
        UE_LOG(LogAdventureGame, Log, TEXT("Set interaction text to: %s"), *InteractionText.ToString());
        if (Command->ShouldHighlightInteractionText())
        {
            InteractionUI->HighlightText();
        }
    }
    else
    {
        FText VerbStr = VerbGetDescriptiveString(Verb);
        InteractionUI->SetText(VerbStr);
    }
}

void UAdventureGameHUD::SetInventoryText()
{
    const ACommandManager *Command = GetCommandManager();
    const UItemManager *ItemManager = GetItemManager();
    if (!Command || !ItemManager) return;
    const EVerbType Verb = Command->CurrentVerb;
    FText InventoryText;
    const UInventoryItem* SourceItem = ItemManager->SourceItem;
    const UInventoryItem* TargetItem = ItemManager->TargetItem;
    const AHotSpot* HotSpot = Command->CurrentHotSpot;
    if (SourceItem == nullptr)
    {
        InteractionUI->ResetText();
        return;
    }
    switch (Verb)
    {
    case EVerbType::Use:
    case EVerbType::UseItem:
        if (SourceItem == TargetItem)
        {
            InventoryText = AdvGameUtils::GetUsingOnSelfText(SourceItem);
        }
        else
        {
            InventoryText = (TargetItem || HotSpot)
                                ? AdvGameUtils::GetUsingItemText(SourceItem, TargetItem, HotSpot)
                                : AdvGameUtils::GetVerbWithItemText(SourceItem, Verb);
        }
        break;
    case EVerbType::GiveItem:
    case EVerbType::Give:
        if (SourceItem == TargetItem)
        {
            InventoryText = AdvGameUtils::GetGiveToSelfText(SourceItem);
        }
        else
        {
            InventoryText = (TargetItem || HotSpot)
                                ? AdvGameUtils::GetGivingItemText(SourceItem, TargetItem, HotSpot)
                                : AdvGameUtils::GetVerbWithItemText(SourceItem, Verb);
        }
        break;
    default:
        InventoryText = AdvGameUtils::GetVerbWithItemText(SourceItem, Verb);
    }
    InteractionUI->SetText(InventoryText);
    if (Command->ShouldHighlightInteractionText())
    {
        InteractionUI->HighlightText();
    }
}

void UAdventureGameHUD::ShowPromptList()
{
    if (UISwitcher->GetActiveWidget() != PromptList)
    {
        DefaultWidget = UISwitcher->GetActiveWidget();
        UISwitcher->SetActiveWidget(PromptList);
    }
}

void UAdventureGameHUD::HidePromptList()
{
    if (UISwitcher->GetActiveWidget() != DefaultWidget)
    {
        UISwitcher->SetActiveWidget(DefaultWidget);
    }
}

void UAdventureGameHUD::AddBarkText(const TArray<FText>& BarkTextArray, USphereComponent* Position,
                                    TOptional<FColor> TextColor)
{
    Bark->AddBarkRequest(FBarkRequest::CreateNPCMultilineRequest(BarkTextArray, 0, Position,
                                                                 TextColor.Get(
                                                                     G_Player_Default_Text_Colour.ToFColor(true))));
}

void UAdventureGameHUD::AddBarkText(const FText& BarkText, USphereComponent* Position, TOptional<FColor> TextColor)
{
    FColor Col = TextColor.Get(G_Player_Default_Text_Colour.ToFColor(true));
    Bark->AddBarkRequest(FBarkRequest::CreateNPCRequest(BarkText, 0, Position, Col));
}

void UAdventureGameHUD::ClearBarkText()
{
    Bark->ClearText();
}

void UAdventureGameHUD::HandleInventoryChanged(EItemKind /*ItemKind*/, EItemDisposition /*Disposition*/)
{
    // TO-DO: Possibly handle changes instead of destroying and re-importing.
    InventoryUI->PopulateInventory(true);
}

void UAdventureGameHUD::UpdateInteractionTextEvent()
{
    SetInteractionText();
}

void UAdventureGameHUD::UpdateSaveGameIndicatorEvent(const ESaveGameStatus SaveGameStatus, bool Success)
{
    switch (SaveGameStatus)
    {
    case ESaveGameStatus::Saved:
        InteractionUI->EndSaving(Success);
        break;
    case ESaveGameStatus::Saving:
        InteractionUI->StartSaving();
        break;
    case ESaveGameStatus::Loading:
        InteractionUI->StartLoading();
        break;
    case ESaveGameStatus::Loaded:
        InteractionUI->EndLoading(Success);
        break;
    default:
        // Do nothing
        break;
    }
}

void UAdventureGameHUD::UpdateInventoryTextEvent()
{
    SetInventoryText();
}

void UAdventureGameHUD::BeginActionEvent()
{
    InteractionUI->HighlightText();
}

void UAdventureGameHUD::InterruptActionEvent()
{
    InteractionUI->ResetText();
    VerbsUI->ClearActiveButton();
}

void UAdventureGameHUD::OnUserInteracted()
{
    VerbsUI->ClearActiveButton();
    Bark->OnUserInteracted();
}

void UAdventureGameHUD::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    if (!IsMobileTouch)
    {
        if (ACommandManager *Command = GetCommandManager())
        {
            bool MouseIsOverUI = IsHovered();
            Command->UpdateMouseOverUI(MouseIsOverUI);
        }
    }
}
