// (c) 2025 Sarah Smith

#pragma once

#include "CoreMinimal.h"
#include "InteractionHUD.h"
#include "InventoryUI.h"
#include "PromptList.h"
#include "VerbsUI.h"
#include "AdventureGame/Dialog/BarkText.h"
#include "AdventureGame/Enums/SaveGameStatus.h"
#include "AdventureGame/Items/ItemManagerProvider.h"

#include "Blueprint/UserWidget.h"
#include "Components/WidgetSwitcher.h"
#include "AdventureGameHUD.generated.h"

class USphereComponent;

/**
 * 
 */
UCLASS()
class ADVENTUREGAME_API UAdventureGameHUD : public UUserWidget, public IItemManagerProvider
{
	GENERATED_BODY()
public:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	
	virtual void NativeOnInitialized() override;

	static UAdventureGameHUD *Create(APlayerController *PlayerController, TSubclassOf<UAdventureGameHUD> AdventureHUDClass);

	void BindCommandHandlers(ACommandManager *CommandManager);

	void BindInventoryHandlers(UAdventureGameInstance* AdventureGameInstance);

	/// Bindings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	UInteractionHUD *InteractionUI;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	UVerbsUI *VerbsUI;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	UInventoryUI *InventoryUI;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	UWidgetSwitcher *UISwitcher;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	UPromptList *PromptList;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	UBarkText *Bark;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	UImage *BlackScreen;

	UFUNCTION()
	void HandleInventoryChanged(EItemKind ItemKind, EItemDisposition Disposition);

	UFUNCTION(BlueprintCallable)
	void UpdateInteractionTextEvent();

	UFUNCTION(BlueprintCallable)
	void UpdateSaveGameIndicatorEvent(ESaveGameStatus SaveGameStatus, bool Success);

	UFUNCTION(BlueprintCallable)
	void UpdateInventoryTextEvent();

	UFUNCTION(BlueprintCallable)
	void BeginActionEvent();

	UFUNCTION(BlueprintCallable)
	void InterruptActionEvent();

	UFUNCTION(BlueprintCallable)
	void OnUserInteracted();
	
	void ShowBlackScreen();

	void HideBlackScreen();

	void SetInteractionText();

	void SetInventoryText();

	UFUNCTION(BlueprintCallable)
	void ShowPromptList();

	UFUNCTION(BlueprintCallable)
	void HidePromptList();

	void AddBarkText(const FText &BarkText, USphereComponent *Position,
		TOptional<FColor> TextColor = TOptional<FColor>());

	void AddBarkText(const TArray<FText> &BarkTextArray, USphereComponent* Position, TOptional<FColor> TextColor);

	void ClearBarkText();
	
private:
    bool IsMobileTouch = false;

	UPROPERTY()
	UWidget *DefaultWidget = nullptr;
};
