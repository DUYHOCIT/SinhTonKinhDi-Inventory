// Fill out your copyright notice in the Description page of Project Settings.
#include "Inventory/Widget/ItemWidget.h"
#include "Inventory/Item/ItemBase.h"
#include "Inventory/Component/InventoryComponent.h"
#include "Components/Image.h"
#include "Components/SizeBox.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Border.h"
#include "Blueprint/DragDropOperation.h"
#include "Inventory/Widget/InventoryGirdWidget.h"
#include "Blueprint/WidgetLayoutLibrary.h"

// ─────────────────────────────────────────────────────────────────
//  Refresh
// ─────────────────────────────────────────────────────────────────

void UItemWidget::Refresh(AItemBase* InItem, UInventoryComponent* InInventory)
{
	Item = InItem;
	Inventory = InInventory;

	if (!Item || !Inventory || !ItemImage || !BackgroundSize) return;

	// Chọn icon theo trạng thái xoay
	UMaterialInterface* Icon = Item->GetIsRotated() ? Item->GetRotatedIcon() : Item->GetIcon();
	if (Icon)
		ItemImage->SetBrushFromMaterial(Icon);

	// Kích thước widget
	const FVector2D Size(
		Item->GetDimensions().X * Inventory->TileSize,
		Item->GetDimensions().Y * Inventory->TileSize
	);

	BackgroundSize->SetWidthOverride(Size.X);
	BackgroundSize->SetHeightOverride(Size.Y);

	if (UCanvasPanelSlot* ImageSlot = UWidgetLayoutLibrary::SlotAsCanvasSlot(ItemImage))
		ImageSlot->SetSize(Size);
}


// ─────────────────────────────────────────────────────────────────
//  Mouse hover
// ─────────────────────────────────────────────────────────────────

void UItemWidget::NativeOnMouseEnter(const FGeometry&, const FPointerEvent&)
{
	if (BackgroundBorder)
		BackgroundBorder->SetBrushColor(FLinearColor(0.f, 0.f, 0.f, 0.5f));
}

void UItemWidget::NativeOnMouseLeave(const FPointerEvent&)
{
	if (BackgroundBorder)
		BackgroundBorder->SetBrushColor(FLinearColor(0.f, 0.f, 0.f, 0.2f));
}

// ─────────────────────────────────────────────────────────────────
//  Drag
// ─────────────────────────────────────────────────────────────────

void UItemWidget::NativeOnDragDetected(const FGeometry& InGeometry,
	const FPointerEvent& InMouseEvent,
	UDragDropOperation*& OutOperation)
{
	Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);

	if (!Item || !Inventory)
	{
		OutOperation = nullptr;
		return;
	}

	// --- Lưu vị trí gốc TRƯỚC KHI xoá khỏi grid ---
	TMap<AItemBase*, FIntPoint> AllItems = Inventory->GetAllItems();
	if (FIntPoint* OriginalTile = AllItems.Find(Item))
	{
		Inventory->OriginalDragStartIndex = Inventory->TileToIndex(*OriginalTile);
	}
	else
	{
		Inventory->OriginalDragStartIndex = -1;
	}
	Inventory->DraggedItem_Internal = Item;

	// Xoá khỏi grid KHÔNG Destroy — item actor vẫn còn sống
	Inventory->RemoveFromGrid(Item);

	// Tạo drag operation
	UDragDropOperation* DragOp = NewObject<UDragDropOperation>();
	DragOp->DefaultDragVisual = this;
	DragOp->Payload = Item;

	OutOperation = DragOp;
	RemoveFromParent();
}

FReply UItemWidget::NativeOnMouseButtonDown(
	const FGeometry& InGeometry,
	const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		return FReply::Handled().DetectDrag(TakeWidget(), EKeys::LeftMouseButton);
	}

	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

FReply UItemWidget::NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	UE_LOG(LogTemp, Warning, TEXT("MouseUp Item"));
	if (GirdWidget && InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		GirdWidget->SetItemSelected(this);
	}
	return Super::NativeOnMouseButtonUp(InGeometry, InMouseEvent);
}
