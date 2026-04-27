// Fill out your copyright notice in the Description page of Project Settings.
#include "Inventory/Widget/InventoryGirdWidget.h"
#include "SinhTonKinhDi/SinhTonKinhDiCharacter.h"
#include "Inventory/Component/InventoryComponent.h"
#include "Inventory/Item/ItemBase.h"
#include "Inventory/Widget/ItemWidget.h"
#include "Inventory/Widget/InformationWidget.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Blueprint/WidgetBlueprintLibrary.h"

// ─────────────────────────────────────────────────────────────────
//  Construct
// ─────────────────────────────────────────────────────────────────

void UInventoryGirdWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
}

void UInventoryGirdWidget::NativeConstruct()
{
	Super::NativeConstruct();

	CharacterRef = Cast<ASinhTonKinhDiCharacter>(GetOwningPlayerPawn());
	if (!CharacterRef) return;

	InventoryComp = CharacterRef->InventoryComponent;
	if (!InventoryComp) return;

	Columns = InventoryComp->Columns;
	Rows = InventoryComp->Rows;
	TileSize = InventoryComp->TileSize;

	if (UCanvasPanelSlot* BorderSlot = UWidgetLayoutLibrary::SlotAsCanvasSlot(GirdBorder))
		BorderSlot->SetSize(FVector2D(Columns * TileSize, Rows * TileSize));

	BuildLineSegments();

	// Đăng ký đúng slot theo GridCategory — mỗi grid instance tự quản lý category riêng
	InventoryComp->SetInventoryGridWidget(this, GridCategory);
	SetIsFocusable(true);
}

// ─────────────────────────────────────────────────────────────────
//  Grid lines (built once)
// ─────────────────────────────────────────────────────────────────

void UInventoryGirdWidget::BuildLineSegments()
{
	StartX.Reset(); StartY.Reset(); EndX.Reset(); EndY.Reset();

	for (int32 i = 0; i <= Columns; ++i)
	{
		const float X = i * TileSize;
		StartX.Add(X); StartY.Add(0.f);
		EndX.Add(X);   EndY.Add(Rows * TileSize);
	}
	for (int32 j = 0; j <= Rows; ++j)
	{
		const float Y = j * TileSize;
		StartX.Add(0.f);              StartY.Add(Y);
		EndX.Add(Columns * TileSize); EndY.Add(Y);
	}
}

// ─────────────────────────────────────────────────────────────────
//  Paint grid lines
// ─────────────────────────────────────────────────────────────────

int32 UInventoryGirdWidget::NativePaint(
	const FPaintArgs& Args, const FGeometry& Geo,
	const FSlateRect& Cull, FSlateWindowElementList& Elements,
	int32 LayerId, const FWidgetStyle& Style, bool bEnabled) const
{
	LayerId = Super::NativePaint(Args, Geo, Cull, Elements, LayerId, Style, bEnabled);
	if (!GirdBorder || StartX.IsEmpty()) return LayerId;

	FPaintContext       Ctx(Geo, Cull, Elements, LayerId, Style, bEnabled);
	const FLinearColor  LineColor(0.5f, 0.5f, 0.5f, 0.5f);
	const FVector2D     Offset = GirdBorder->GetCachedGeometry()
		.GetLocalPositionAtCoordinates(FVector2D::ZeroVector);

	for (int32 i = 0; i < StartX.Num(); ++i)
	{
		UWidgetBlueprintLibrary::DrawLine(
			Ctx,
			FVector2D(StartX[i], StartY[i]) + Offset,
			FVector2D(EndX[i], EndY[i]) + Offset,
			LineColor
		);
	}
	return LayerId + 1;
}

// ─────────────────────────────────────────────────────────────────
//  Refresh — chỉ hiển thị item thuộc GridCategory của grid này
// ─────────────────────────────────────────────────────────────────

void UInventoryGirdWidget::Refresh()
{
	if (!GirdCanvasPanel || !CharacterRef || !InventoryComp
		|| !CharacterRef->ItemWidgetClass) return;

	GirdCanvasPanel->ClearChildren();

	// Lấy đúng danh sách item của category này (kho riêng biệt)
	for (auto& [Item, Tile] : InventoryComp->GetItemsForCategory(GridCategory))
	{
		if (!IsValid(Item)) continue;

		UItemWidget* W = CreateWidget<UItemWidget>(GetWorld(), CharacterRef->ItemWidgetClass);
		if (!W) continue;

		W->GirdWidget = this;
		W->SetOwningPlayer(GetOwningPlayer());
		W->Refresh(Item, InventoryComp);

		PanelSlot = GirdCanvasPanel->AddChild(W);

		if (UCanvasPanelSlot* ChildSlot = Cast<UCanvasPanelSlot>(PanelSlot))
		{
			ChildSlot->SetAutoSize(true);
			ChildSlot->SetPosition(FVector2D(Tile.X * TileSize, Tile.Y * TileSize));
		}
	}
}

// ─────────────────────────────────────────────────────────────────
//  Item selection + hiển thị InformationWidget
// ─────────────────────────────────────────────────────────────────

void UInventoryGirdWidget::SetItemSelected(UItemWidget* NewSelected)
{
	ItemSelected = NewSelected;

	if (!InventoryComp || !InventoryComp->InformationWidgetRef) return;

	UInformationWidget* InfoWidget = InventoryComp->InformationWidgetRef;

	if (NewSelected && IsValid(NewSelected->GetItem()))
	{
		AItemBase* SelectedItem = NewSelected->GetItem();
		InfoWidget->setInformation(SelectedItem->GetName(), SelectedItem->GetDescription());
		InfoWidget->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		InfoWidget->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UInventoryGirdWidget::SetNullItemSelected()
{
	ItemSelected = nullptr;

	// Ẩn InformationWidget khi bỏ chọn
	if (InventoryComp && InventoryComp->InformationWidgetRef)
		InventoryComp->InformationWidgetRef->SetVisibility(ESlateVisibility::Collapsed);
}

// ─────────────────────────────────────────────────────────────────
//  Drag Over
// ─────────────────────────────────────────────────────────────────

bool UInventoryGirdWidget::NativeOnDragOver(const FGeometry& InGeo,
	const FDragDropEvent& InDDE, UDragDropOperation* InOp)
{
	if (!InOp || !InOp->Payload) return false;

	DraggedItem = Cast<AItemBase>(InOp->Payload);
	if (!DraggedItem) return false;

	// Chỉ cho kéo item đúng category vào grid này
	if (DraggedItem->ItemCategory != GridCategory)
	{
		DraggedItem = nullptr;
		return false;
	}

	const FVector2D GridOffset = GirdBorder->GetCachedGeometry()
		.GetLocalPositionAtCoordinates(FVector2D::ZeroVector);
	const FVector2D AdjustedPos =
		InGeo.AbsoluteToLocal(InDDE.GetScreenSpacePosition()) - GridOffset;

	const FMousePositionInTile InTile = GetMousePositionInTile(AdjustedPos);
	const FIntPoint HalfDim(
		InTile.Right ? DraggedItem->GetDimensions().X - 1 : DraggedItem->GetDimensions().X,
		InTile.Down ? DraggedItem->GetDimensions().Y - 1 : DraggedItem->GetDimensions().Y
	);

	DraggedItemTopLeftTile = FIntPoint(
		FMath::TruncToInt(AdjustedPos.X / TileSize),
		FMath::TruncToInt(AdjustedPos.Y / TileSize)
	) - HalfDim / 2;

	return true;
}

// ─────────────────────────────────────────────────────────────────
//  Drop
// ─────────────────────────────────────────────────────────────────

bool UInventoryGirdWidget::NativeOnDrop(const FGeometry& InGeo,
	const FDragDropEvent& InDDE, UDragDropOperation* InOp)
{
	DraggedItem = nullptr;

	if (!InOp || !InOp->Payload || !InventoryComp) return false;

	DroppedItem = Cast<AItemBase>(InOp->Payload);
	if (!DroppedItem) return false;

	// ── Từ chối item không đúng category — trả về vị trí gốc ────
	if (DroppedItem->ItemCategory != GridCategory)
	{
		const int32 OriginalIndex = InventoryComp->OriginalDragStartIndex;
		if (OriginalIndex != -1)
			InventoryComp->AddItemAt(DroppedItem, OriginalIndex);

		DroppedItem = nullptr;
		StoredDragOp = nullptr;
		InventoryComp->OriginalDragStartIndex = -1;
		InventoryComp->DraggedItem_Internal = nullptr;
		return true; // consume event — tránh spawn ra world
	}

	const int32 OriginalIndex = InventoryComp->OriginalDragStartIndex;
	const int32 TargetIndex = InventoryComp->TileToIndex(DraggedItemTopLeftTile);

	if (InventoryComp->IsRoomAvaiable(DroppedItem, TargetIndex))
	{
		InventoryComp->AddItemAt(DroppedItem, TargetIndex);
		goto Cleanup;
	}

	if (OriginalIndex != -1)
	{
		if (InventoryComp->IsRoomAvaiable(DroppedItem, OriginalIndex))
		{
			InventoryComp->AddItemAt(DroppedItem, OriginalIndex);
			goto Cleanup;
		}

		DroppedItem->RotateItem();
		InventoryComp->AddItemAt(DroppedItem, OriginalIndex);
		goto Cleanup;
	}

	DroppedItem = nullptr;
	StoredDragOp = nullptr;
	InventoryComp->OriginalDragStartIndex = -1;
	InventoryComp->DraggedItem_Internal = nullptr;
	return false;

Cleanup:
	DroppedItem = nullptr;
	StoredDragOp = nullptr;
	InventoryComp->OriginalDragStartIndex = -1;
	InventoryComp->DraggedItem_Internal = nullptr;
	return true;
}

// ─────────────────────────────────────────────────────────────────
//  Drag Enter
// ─────────────────────────────────────────────────────────────────

void UInventoryGirdWidget::NativeOnDragEnter(const FGeometry& InGeo,
	const FDragDropEvent& InDDE, UDragDropOperation* InOp)
{
	Super::NativeOnDragEnter(InGeo, InDDE, InOp);
	StoredDragOp = InOp;
}

// ─────────────────────────────────────────────────────────────────
//  Key: Tab = đóng kho, R = xoay item đang kéo
// ─────────────────────────────────────────────────────────────────

FReply UInventoryGirdWidget::NativeOnPreviewKeyDown(const FGeometry& InGeo,
	const FKeyEvent& InKey)
{
	if (InKey.GetKey() == EKeys::Tab)
	{
		const bool bIsDragging = (DraggedItem != nullptr || StoredDragOp != nullptr
			|| (InventoryComp && InventoryComp->DraggedItem_Internal != nullptr));

		if (bIsDragging && InventoryComp)
		{
			AItemBase* ItemBeingDragged = InventoryComp->DraggedItem_Internal;
			const int32 OrigGridIndex = InventoryComp->OriginalDragStartIndex;
			const int32 OrigToolSlotIndex = InventoryComp->OriginalToolSlotIndex;

			UWidgetBlueprintLibrary::CancelDragDrop();

			if (IsValid(ItemBeingDragged))
			{
				if (OrigToolSlotIndex != -1)
					InventoryComp->EquipToolAt(ItemBeingDragged, OrigToolSlotIndex);
				else if (OrigGridIndex != -1)
					InventoryComp->AddItemAt(ItemBeingDragged, OrigGridIndex);
			}

			InventoryComp->OriginalDragStartIndex = -1;
			InventoryComp->OriginalToolSlotIndex = -1;
			InventoryComp->DraggedItem_Internal = nullptr;
		}

		DraggedItem = nullptr;
		StoredDragOp = nullptr;

		if (CharacterRef) CharacterRef->ToggleInventory();
		return FReply::Handled();
	}

	if (InKey.GetKey() == EKeys::R && DraggedItem && StoredDragOp)
	{
		DraggedItem->RotateItem();
		if (UItemWidget* Visual = Cast<UItemWidget>(StoredDragOp->DefaultDragVisual))
			Visual->Refresh(DraggedItem, InventoryComp);
		return FReply::Handled();
	}

	return Super::NativeOnPreviewKeyDown(InGeo, InKey);
}

FReply UInventoryGirdWidget::NativeOnMouseButtonDown(const FGeometry& InGeo,
	const FPointerEvent& InME)
{
	return FReply::Handled();
}

// ─────────────────────────────────────────────────────────────────
//  Helpers
// ─────────────────────────────────────────────────────────────────

FMousePositionInTile UInventoryGirdWidget::GetMousePositionInTile(FVector2D LocalPos) const
{
	FMousePositionInTile Result;
	Result.Right = FMath::Fmod(LocalPos.X, TileSize) > (TileSize * 0.5f);
	Result.Down = FMath::Fmod(LocalPos.Y, TileSize) > (TileSize * 0.5f);
	return Result;
}

bool UInventoryGirdWidget::IsRoomAvailableAt(AItemBase* Item, FIntPoint TopLeft) const
{
	return IsValid(Item) && InventoryComp &&
		InventoryComp->IsRoomAvaiable(Item, InventoryComp->TileToIndex(TopLeft));
}