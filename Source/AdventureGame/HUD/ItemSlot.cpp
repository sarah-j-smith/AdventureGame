// (c) 2025 Sarah Smith


#include "ItemSlot.h"

#include "AdventureGame/Player/AdventurePlayerController.h"
#include "AdventureGame/Items/InventoryItem.h"
#include "AdventureGame/Player/ItemManager.h"
#include "Kismet/GameplayStatics.h"

void UItemSlot::NativeOnInitialized()
{
	ItemButton->OnClicked.AddDynamic(this, & UItemSlot::HandleOnClicked);
	ItemButton->OnHovered.AddDynamic(this, & UItemSlot::HandleOnHover);
	ItemButton->OnUnhovered.AddDynamic(this, & UItemSlot::HandleOnUnhover);

	if (!HasItem)
	{
		ItemSlot->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UItemSlot::AddItem(UInventoryItem* InventoryItem)
{
	if (InventoryItem != nullptr)
	{
		HasItem = true;
		this->InventoryItem = InventoryItem;
		SetButtonImageFromInventoryItem(InventoryItem);
		ItemSlot->SetVisibility(ESlateVisibility::Visible);
	}
}

void UItemSlot::RemoveItem()
{
	if (HasItem)
	{
		HasItem = false;
		this->InventoryItem = nullptr;
		ItemSlot->SetBrush(SavedStyle);
		ItemSlot->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UItemSlot::HandleOnClicked()
{
	if (ACommandManager *Command = GetCommandManager())
	{
		if (HasItem)
		{
			Command->HandleInventoryItemClicked(this);
		}
		else
		{
			// Clicked on an empty slot so cancel whatever it was doing.
			Command->InterruptCurrentAction();
		}
	}
}

void UItemSlot::HandleOnHover()
{
	if (HasItem)
	{
		if (UItemManager *ItemManager = GetItemManager())
		{
			ItemManager->MouseEnterInventoryItem(this);
		}		
	}
}

void UItemSlot::HandleOnUnhover()
{
	if (UItemManager *ItemManager = GetItemManager())
	{
		ItemManager->MouseLeaveInventoryItem();
	}		
}

void UItemSlot::SetButtonImageFromInventoryItem(const UInventoryItem* InventoryItem)
{
	SavedStyle = ItemSlot->GetBrush();
	FSlateBrush NewBrush = SavedStyle;
	NewBrush.DrawAs = ESlateBrushDrawType::Type::Image;
#if WITH_EDITOR
	NewBrush.SetResourceObject(InventoryItem->Thumbnail->GetSourceTexture());
#else
	NewBrush.SetResourceObject(InventoryItem->Thumbnail->GetBakedTexture());
#endif
	ItemSlot->SetBrush(NewBrush);
}

