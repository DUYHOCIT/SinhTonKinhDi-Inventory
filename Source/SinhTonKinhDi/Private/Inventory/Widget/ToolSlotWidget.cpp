// Fill out your copyright notice in the Description page of Project Settings.
#include "Inventory/Widget/ToolSlotWidget.h"
#include "Inventory/Component/InventoryComponent.h"
#include "Inventory/Item/ItemBase.h"
#include "Inventory/Widget/ToolInventoryWidget.h"
#include "Blueprint/DragDropOperation.h"
#include "Components/Image.h"
#include "Components/Border.h"
#include "Components/SizeBox.h"
#include "Components/CanvasPanelSlot.h"
#include "Blueprint/WidgetLayoutLibrary.h"

void UToolSlotWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (SlotHighlight)
		SlotHighlight->SetBrushColor(NormalColor);

	// Tự động thiết lập trạng thái trống hoặc đầy ngay từ lúc khởi tạo
	SetItem(SlottedItem);
}

// ─────────────────────────────────────────────────────────────────
//  Init
// ─────────────────────────────────────────────────────────────────

void UToolSlotWidget::Init(int32 InSlotIndex,
	UInventoryComponent* InComp,
	UToolInventoryWidget* InToolWidget,
	float InIconScale,
	float InEmptySlotSize)
{
	SlotIndex      = InSlotIndex;
	InventoryComp  = InComp;
	ToolWidget     = InToolWidget;
	IconScale      = InIconScale;
	EmptySlotSize  = InEmptySlotSize;
}

// ─────────────────────────────────────────────────────────────────
//  SetItem — cập nhật icon + kích thước container
// ─────────────────────────────────────────────────────────────────

void UToolSlotWidget::SetItem(AItemBase* Item)
{
	SlottedItem = Item;

	if (!ItemContainer) return;

	if (IsValid(Item))
	{
		// CÓ ĐỒ: Tính toán kích thước dựa trên số ô (Grid) x Scale
		const FIntPoint Dim = Item->GetDimensions();
		const float W = Dim.X * IconScale;
		const float H = Dim.Y * IconScale;

		UCanvasPanelSlot* CanvasSlot = UWidgetLayoutLibrary::SlotAsCanvasSlot(ItemContainer);
		if (CanvasSlot)
		{
			CanvasSlot->SetSize(FVector2D(W, H));
		}

		if (ItemIcon)
		{
			UMaterialInterface* Icon = Item->GetIsRotated()
				? Item->GetRotatedIcon()
				: Item->GetIcon();

			if (Icon)
				ItemIcon->SetBrushFromMaterial(Icon);

			// XÓA PHẦN UCanvasPanelSlot Ở ĐÂY - Hãy để Blueprint tự động Fill (lấp đầy)
			ItemIcon->SetVisibility(ESlateVisibility::HitTestInvisible);
		}

		// Ẩn icon tròn trống vì slot đã có đồ
		if (EmptySlotIcon)
		{
			EmptySlotIcon->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
	else
	{
		UCanvasPanelSlot* CanvasSlot = UWidgetLayoutLibrary::SlotAsCanvasSlot(ItemContainer);
		if (CanvasSlot)
		{
			CanvasSlot->SetSize(FVector2D(EmptySlotSize, EmptySlotSize));
		}

		// Ẩn hình ảnh item
		if (ItemIcon)
		{
			ItemIcon->SetVisibility(ESlateVisibility::Collapsed);
		}

		// Hiện icon vòng tròn trống
		if (EmptySlotIcon)
		{
			EmptySlotIcon->SetVisibility(ESlateVisibility::HitTestInvisible);
		}
	}
}

// ─────────────────────────────────────────────────────────────────
//  SetActive
// ─────────────────────────────────────────────────────────────────

void UToolSlotWidget::SetActive(bool bActive)
{
	bIsActive = bActive;
	if (SlotHighlight)
		SlotHighlight->SetBrushColor(bActive ? ActiveColor : NormalColor);
}

// ─────────────────────────────────────────────────────────────────
//  Mouse Down
// ─────────────────────────────────────────────────────────────────

FReply UToolSlotWidget::NativeOnMouseButtonDown(const FGeometry& InGeo,
	const FPointerEvent& InME)
{
	if (InME.GetEffectingButton() == EKeys::RightMouseButton && IsValid(SlottedItem))
	{
		if (InventoryComp)
			InventoryComp->UnequipTool(SlotIndex);
		return FReply::Handled();
	}

	if (InME.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		bDragStarted = false;

		if (!IsValid(SlottedItem))
		{
			if (InventoryComp)
				InventoryComp->SetActiveToolSlot(SlotIndex);
			return FReply::Handled();
		}

		return FReply::Handled().DetectDrag(TakeWidget(), EKeys::LeftMouseButton);
	}

	return Super::NativeOnMouseButtonDown(InGeo, InME);
}

// ─────────────────────────────────────────────────────────────────
//  Mouse Up — không kéo → chọn slot active
// ─────────────────────────────────────────────────────────────────

FReply UToolSlotWidget::NativeOnMouseButtonUp(const FGeometry& InGeo,
	const FPointerEvent& InME)
{
	if (InME.GetEffectingButton() == EKeys::LeftMouseButton && !bDragStarted)
	{
		if (InventoryComp)
			InventoryComp->SetActiveToolSlot(SlotIndex);
		return FReply::Handled();
	}
	return Super::NativeOnMouseButtonUp(InGeo, InME);
}

// ─────────────────────────────────────────────────────────────────
//  Drag Detected
//
//  LỖI CŨ: RemoveFromToolSlot → Refresh() → SetItem(nullptr) trên
//           widget này → icon ẩn đi → drag visual trống.
//
//  FIX: Sau khi RemoveFromToolSlot (đã xóa khỏi component + gọi
//       Refresh()), gọi lại SetItem(DragItem) để khôi phục icon
//       trên *chính widget này* làm drag visual.
//       SlottedItem trong component vẫn là nullptr; chỉ visual bị
//       ghi đè tạm thời cho đến khi drag kết thúc và Refresh() tiếp
//       theo sẽ đặt lại đúng trạng thái.
// ─────────────────────────────────────────────────────────────────

void UToolSlotWidget::NativeOnDragDetected(const FGeometry& InGeo,
	const FPointerEvent& InME,
	UDragDropOperation*& OutOp)
{
	Super::NativeOnDragDetected(InGeo, InME, OutOp);

	if (!IsValid(SlottedItem) || !InventoryComp)
	{
		OutOp = nullptr;
		return;
	}

	bDragStarted = true;

	// Ghi slot gốc trước khi lấy ra
	InventoryComp->OriginalToolSlotIndex = SlotIndex;
	InventoryComp->OriginalDragStartIndex = -1;
	InventoryComp->DraggedItem_Internal = SlottedItem;

	// Lấy item ra khỏi component — hàm này đã tự động gọi Refresh() 
	// làm cho slot gốc này trở thành nullptr (ẩn icon đi).
	AItemBase* DragItem = InventoryComp->RemoveFromToolSlot(SlotIndex);
	if (!DragItem)
	{
		OutOp = nullptr;
		return;
	}

	// ── FIX MỚI: Tạo một Widget bản sao chỉ để làm hình ảnh kéo (Drag Visual) ──
	// Thay vì gán lại SetItem(DragItem) vào slot này, ta gán nó cho bản sao.
	UToolSlotWidget* DragVisualWidget = CreateWidget<UToolSlotWidget>(GetWorld(), GetClass());
	if (DragVisualWidget)
	{
		DragVisualWidget->Init(SlotIndex, InventoryComp, ToolWidget, IconScale, EmptySlotSize);
		DragVisualWidget->SetItem(DragItem);

		if (DragVisualWidget->SlotHighlight)
		{
			DragVisualWidget->SlotHighlight->SetVisibility(ESlateVisibility::Collapsed);
		}

		// THÊM: Làm mờ nhẹ cái ảnh khi đang cầm trên chuột (Tuỳ chọn)
		DragVisualWidget->SetRenderOpacity(0.8f);
	}

	UDragDropOperation* DragOp = NewObject<UDragDropOperation>();
	DragOp->Payload = DragItem;
	DragOp->DefaultDragVisual = DragVisualWidget; // Dùng widget bản sao làm hình bay theo chuột
	OutOp = DragOp;

	// XÓA DÒNG SetItem(DragItem); cũ ở đây đi.
	// Lúc này "this" (slot gốc) sẽ trống hoàn toàn như ý bạn muốn.
}

// ─────────────────────────────────────────────────────────────────
//  Drop — nhận từ slot khác (swap) hoặc từ grid (equip)
// ─────────────────────────────────────────────────────────────────

bool UToolSlotWidget::NativeOnDrop(const FGeometry& InGeo,
	const FDragDropEvent& InDDE,
	UDragDropOperation* InOp)
{
	if (!InOp || !InOp->Payload || !InventoryComp) return false;

	AItemBase* DroppedItem = Cast<AItemBase>(InOp->Payload);
	if (!DroppedItem) return false;

	const int32 FromToolSlot = InventoryComp->OriginalToolSlotIndex;

	// ── Trường hợp 1: kéo từ tool slot sang tool slot → SWAP ────
	if (FromToolSlot != -1 && DroppedItem->ItemCategory == EItemCategory::Tool)
	{
		InventoryComp->PerformToolSlotDrop(DroppedItem, FromToolSlot, SlotIndex);
		return true;
	}

	// ── Trường hợp 2: kéo từ grid sang tool slot ─────────────────
	if (DroppedItem->ItemCategory == EItemCategory::Tool)
	{
		return InventoryComp->EquipToolAt(DroppedItem, SlotIndex);
	}

	// ── Trường hợp 3: sai category → trả về vị trí gốc ──────────
	const int32 OrigIdx = InventoryComp->OriginalDragStartIndex;
	if (OrigIdx != -1 && InventoryComp->IsRoomAvaiable(DroppedItem, OrigIdx))
		InventoryComp->AddItemAt(DroppedItem, OrigIdx);
	else
		InventoryComp->TryAddItem(DroppedItem);

	InventoryComp->OriginalDragStartIndex = -1;
	InventoryComp->OriginalToolSlotIndex  = -1;
	InventoryComp->DraggedItem_Internal   = nullptr;
	return true;
}

// ─────────────────────────────────────────────────────────────────
//  Hover
// ─────────────────────────────────────────────────────────────────

void UToolSlotWidget::NativeOnMouseEnter(const FGeometry&, const FPointerEvent&)
{
	if (SlotHighlight && !bIsActive)
		SlotHighlight->SetBrushColor(HoverColor);
}

void UToolSlotWidget::NativeOnMouseLeave(const FPointerEvent&)
{
	if (SlotHighlight)
		SlotHighlight->SetBrushColor(bIsActive ? ActiveColor : NormalColor);
}