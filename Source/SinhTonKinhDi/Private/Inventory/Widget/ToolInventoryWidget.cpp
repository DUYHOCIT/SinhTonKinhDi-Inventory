// Fill out your copyright notice in the Description page of Project Settings.
#include "Inventory/Widget/ToolInventoryWidget.h"
#include "Inventory/Widget/ToolSlotWidget.h"
#include "Inventory/Component/InventoryComponent.h"
#include "Inventory/Item/ItemBase.h"
#include "SinhTonKinhDi/SinhTonKinhDiCharacter.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Blueprint/WidgetBlueprintLibrary.h"

// ─────────────────────────────────────────────────────────────────
//  NativeConstruct
//
//  Vấn đề của bản trước: toàn bộ việc tạo slot để sang NativeTick,
//  nhưng InventoryWidget bắt đầu Collapsed → NativeTick không chạy
//  → ToolSlots mãi rỗng → không nhặt được, không scroll được.
//
//  Giải pháp tách 2 việc:
//    • NativeConstruct: đếm anchor → ResizeToolSlots(N) NGAY
//      (không cần geometry — chỉ cần số lượng)
//      → pickup + scroll hoạt động từ frame 0
//    • NativeTick: khi inventory mở lần đầu, geometry có sẵn
//      → tạo visual slot widgets với đúng vị trí
// ─────────────────────────────────────────────────────────────────

void UToolInventoryWidget::NativeConstruct()
{
	Super::NativeConstruct();

	CharacterRef = Cast<ASinhTonKinhDiCharacter>(GetOwningPlayerPawn());
	if (!CharacterRef) return;

	InventoryComp = CharacterRef->InventoryComponent;
	if (!InventoryComp) return;

	InventoryComp->SetToolInventoryWidget(this);

	// FIX: Cho phép widget nhận key event để xử lý Tab cancel-drag
	SetIsFocusable(true);

	// ── Đếm anchor và đồng bộ số slot NGAY (không cần geometry) ─
	if (SlotsCanvas)
	{
		const int32 AnchorCount = SlotsCanvas->GetChildrenCount();
		if (AnchorCount > 0)
		{
			// Sau bước này ToolSlots.Num() == AnchorCount
			// → pickup và scroll dùng đúng số slot thực
			InventoryComp->ResizeToolSlots(AnchorCount);
		}
	}

	// Visual slot widgets được tạo ở NativeTick lần đầu
	// khi SlotsCanvas có geometry thực (inventory đang mở)
}

// ─────────────────────────────────────────────────────────────────
//  NativeTick — chỉ chạy khi widget visible (inventory đang mở)
// ─────────────────────────────────────────────────────────────────

void UToolInventoryWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!bSlotsCreated && SlotsCanvas && InventoryComp && ToolSlotClass)
	{
		const FVector2D CanvasSize = SlotsCanvas->GetCachedGeometry().GetLocalSize();
		if (!CanvasSize.IsNearlyZero())
		{
			bSlotsCreated = true;
			CreateSlotsFromAnchors();
			Refresh();
		}
	}
}

// ─────────────────────────────────────────────────────────────────
//  NativeOnPreviewKeyDown
//
//  FIX: InventoryGirdWidget.NativeOnPreviewKeyDown không nhận được
//  key event khi tab Tool đang active vì Grid bị Collapsed.
//  ToolInventoryWidget phải tự xử lý Tab để:
//    1. Hủy drag operation đang diễn ra (nếu có).
//    2. Trả item về slot gốc.
//    3. Đóng kho đồ (ToggleInventory).
// ─────────────────────────────────────────────────────────────────

FReply UToolInventoryWidget::NativeOnPreviewKeyDown(const FGeometry& InGeo,
	const FKeyEvent& InKey)
{
	if (InKey.GetKey() == EKeys::Tab)
	{
		if (InventoryComp && InventoryComp->DraggedItem_Internal != nullptr)
		{
			// Snapshot trạng thái trước khi CancelDragDrop xóa hết
			AItemBase* ItemBeingDragged = InventoryComp->DraggedItem_Internal;
			const int32 OrigToolSlotIndex = InventoryComp->OriginalToolSlotIndex;
			const int32 OrigGridIndex = InventoryComp->OriginalDragStartIndex;

			// Hủy drag operation của Slate
			UWidgetBlueprintLibrary::CancelDragDrop();

			// ── Trả item về đúng nơi xuất phát ──────────────────
			if (IsValid(ItemBeingDragged))
			{
				if (OrigToolSlotIndex != -1)
				{
					// Kéo từ tool slot → trả về slot gốc
					InventoryComp->EquipToolAt(ItemBeingDragged, OrigToolSlotIndex);
				}
				else if (OrigGridIndex != -1)
				{
					// Kéo từ grid (không xảy ra ở tab Tool, nhưng bảo vệ phòng thủ)
					InventoryComp->AddItemAt(ItemBeingDragged, OrigGridIndex);
				}
			}

			// Dọn state
			InventoryComp->OriginalDragStartIndex = -1;
			InventoryComp->OriginalToolSlotIndex = -1;
			InventoryComp->DraggedItem_Internal = nullptr;
		}

		if (CharacterRef) CharacterRef->ToggleInventory();
		return FReply::Handled();
	}

	return Super::NativeOnPreviewKeyDown(InGeo, InKey);
}

// ─────────────────────────────────────────────────────────────────
//  CreateSlotsFromAnchors — gọi sau khi geometry sẵn sàng
// ─────────────────────────────────────────────────────────────────

void UToolInventoryWidget::CreateSlotsFromAnchors()
{
	if (!SlotsCanvas || !InventoryComp || !ToolSlotClass) return;

	Slots.Empty();

	TArray<FVector2D> AnchorPositions;
	const int32 AnchorCount = SlotsCanvas->GetChildrenCount();

	for (int32 i = 0; i < AnchorCount; ++i)
	{
		UWidget* Anchor = SlotsCanvas->GetChildAt(i);
		if (!Anchor) continue;

		AnchorPositions.Add(GetAnchorPosition(Anchor));
		Anchor->SetVisibility(ESlateVisibility::Hidden);
	}

	if (AnchorPositions.IsEmpty()) return;

	// ResizeToolSlots đã gọi ở NativeConstruct, không gọi lại
	// để tránh xử lý shrink không cần thiết

	for (int32 i = 0; i < AnchorPositions.Num(); ++i)
	{
		UToolSlotWidget* SlotWidget =
			CreateWidget<UToolSlotWidget>(GetWorld(), ToolSlotClass);
		if (!SlotWidget) continue;

		SlotWidget->Init(i, InventoryComp, this, IconScale, EmptySlotSize);

		UPanelSlot* PSlot = SlotsCanvas->AddChild(SlotWidget);
		if (UCanvasPanelSlot* CSlot = Cast<UCanvasPanelSlot>(PSlot))
		{
			CSlot->SetAlignment(FVector2D(0.5f, 0.5f));
			CSlot->SetPosition(AnchorPositions[i]);
			CSlot->SetAutoSize(true);
			CSlot->SetZOrder(10);
		}

		Slots.Add(SlotWidget);
	}
}

// ─────────────────────────────────────────────────────────────────
//  GetAnchorPosition
//
//  FIX: vị_trí_tâm = Anchor% × kích_thước_cha + Offset + Size*(0.5-Align)
//  Code cũ chỉ dùng Offset nên tất cả slot đều chồng lên nhau.
// ─────────────────────────────────────────────────────────────────

FVector2D UToolInventoryWidget::GetAnchorPosition(UWidget* AnchorWidget) const
{
	if (!AnchorWidget || !SlotsCanvas) return FVector2D::ZeroVector;

	if (UCanvasPanelSlot* CSlot = UWidgetLayoutLibrary::SlotAsCanvasSlot(AnchorWidget))
	{
		const FVector2D ParentSize = SlotsCanvas->GetCachedGeometry().GetLocalSize();
		const FVector2D AnchorFrac = FVector2D(CSlot->GetAnchors().Minimum.X,
			CSlot->GetAnchors().Minimum.Y);
		const FVector2D Offset = CSlot->GetPosition();
		const FVector2D Size = CSlot->GetSize();
		const FVector2D Align = CSlot->GetAlignment();

		const FVector2D TopLeft = AnchorFrac * ParentSize + Offset;
		return TopLeft + Size * (FVector2D(0.5f, 0.5f) - Align);
	}

	return FVector2D::ZeroVector;
}

// ─────────────────────────────────────────────────────────────────
//  Refresh
// ─────────────────────────────────────────────────────────────────

void UToolInventoryWidget::Refresh()
{

	// Nếu visual chưa tạo: bỏ qua phần icon (không crash),
	// pickup/scroll vẫn hoạt động nhờ ToolSlots đã được resize
	if (!bSlotsCreated) return;

	for (int32 i = 0; i < Slots.Num(); ++i)
	{
		if (!Slots[i] || !InventoryComp) continue;
		Slots[i]->SetItem(InventoryComp->GetToolAt(i));
		Slots[i]->SetActive(i == InventoryComp->ActiveToolSlotIndex);
	}
}