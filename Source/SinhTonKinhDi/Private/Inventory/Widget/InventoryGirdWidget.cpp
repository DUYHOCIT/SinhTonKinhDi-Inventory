// Fill out your copyright notice in the Description page of Project Settings.
#include "Inventory/Widget/InventoryGirdWidget.h"
#include "SinhTonKinhDi/SinhTonKinhDiCharacter.h"
#include "Inventory/Component/InventoryComponent.h"
#include "Inventory/Item/ItemBase.h"
#include "Inventory/Widget/ItemWidget.h"
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
	InventoryComp->SetInventoryGridWidget(this);
	SetIsFocusable(true);
}

// ─────────────────────────────────────────────────────────────────
//  Xây đường kẻ grid (chỉ gọi 1 lần)
// ─────────────────────────────────────────────────────────────────

void UInventoryGirdWidget::BuildLineSegments()
{
	StartX.Reset(); StartY.Reset(); EndX.Reset(); EndY.Reset();

	// Đường dọc (Columns+1 đường)
	for (int32 i = 0; i <= Columns; ++i)
	{
		const float X = i * TileSize;
		StartX.Add(X);  StartY.Add(0.f);
		EndX.Add(X);    EndY.Add(Rows * TileSize);
	}
	// Đường ngang (Rows+1 đường)
	for (int32 j = 0; j <= Rows; ++j)
	{
		const float Y = j * TileSize;
		StartX.Add(0.f);            StartY.Add(Y);
		EndX.Add(Columns * TileSize); EndY.Add(Y);
	}
}

// ─────────────────────────────────────────────────────────────────
//  Vẽ đường kẻ
// ─────────────────────────────────────────────────────────────────

int32 UInventoryGirdWidget::NativePaint(
	const FPaintArgs& Args, const FGeometry& Geo,
	const FSlateRect& Cull, FSlateWindowElementList& Elements,
	int32 LayerId, const FWidgetStyle& Style, bool bEnabled) const
{
	LayerId = Super::NativePaint(Args, Geo, Cull, Elements, LayerId, Style, bEnabled);
	if (!GirdBorder || StartX.IsEmpty()) return LayerId;

	FPaintContext Ctx(Geo, Cull, Elements, LayerId, Style, bEnabled);
	const FLinearColor LineColor(0.5f, 0.5f, 0.5f, 0.5f);
	const FVector2D    Offset = GirdBorder->GetCachedGeometry()
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
//  Refresh widget
// ─────────────────────────────────────────────────────────────────

void UInventoryGirdWidget::Refresh()
{
	if (!GirdCanvasPanel || !CharacterRef || !InventoryComp
		|| !CharacterRef->ItemWidgetClass) return;

	GirdCanvasPanel->ClearChildren();

	for (auto& [Item, Tile] : InventoryComp->GetAllItems())
	{
		if (!IsValid(Item)) continue;

		UItemWidget* W = CreateWidget<UItemWidget>(GetWorld(), CharacterRef->ItemWidgetClass);
		if (!W) continue;

		UItemWidget* iw = Cast<UItemWidget>(W);
		if (iw)
		{
			iw->GirdWidget = this;
		}
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

void UInventoryGirdWidget::SetItemSelected(UItemWidget* NewSelected)
{
	ItemSelected = NewSelected;
}

void UInventoryGirdWidget::SetNullItemSelected()
{
	ItemSelected = nullptr;
}

// ─────────────────────────────────────────────────────────────────
//  Drag over — tính tile top-left của item đang kéo
// ─────────────────────────────────────────────────────────────────

bool UInventoryGirdWidget::NativeOnDragOver(const FGeometry& InGeo,
	const FDragDropEvent& InDDE,
	UDragDropOperation* InOp)
{
	if (!InOp || !InOp->Payload) return false;

	DraggedItem = Cast<AItemBase>(InOp->Payload);
	if (!DraggedItem) return false;

	const FVector2D GridOffset = GirdBorder->GetCachedGeometry().GetLocalPositionAtCoordinates(FVector2D::ZeroVector);
	const FVector2D AdjustedPos = InGeo.AbsoluteToLocal(InDDE.GetScreenSpacePosition()) - GridOffset;

	// Căn chỉnh half-tile để cursor ở giữa item
	const FMousePositionInTile InTile = GetMousePositionInTile(AdjustedPos);
	const FIntPoint HalfDim(
		InTile.Right ? DraggedItem->GetDimensions().X - 1 : DraggedItem->GetDimensions().X,
		InTile.Down ? DraggedItem->GetDimensions().Y - 1 : DraggedItem->GetDimensions().Y
	);

	DraggedItemTopLeftTile = FIntPoint(FMath::TruncToInt(AdjustedPos.X / TileSize), FMath::TruncToInt(AdjustedPos.Y / TileSize)) - HalfDim / 2;

	return true;
}

// ─────────────────────────────────────────────────────────────────
//  Drop
// ─────────────────────────────────────────────────────────────────

bool UInventoryGirdWidget::NativeOnDrop(const FGeometry& InGeo,
	const FDragDropEvent& InDDE,
	UDragDropOperation* InOp)
{
	// Reset trạng thái hiển thị
	DraggedItem = nullptr;

	if (!InOp || !InOp->Payload || !InventoryComp) return false;

	DroppedItem = Cast<AItemBase>(InOp->Payload);
	if (!DroppedItem) return false;

	const int32 OriginalIndex = InventoryComp->OriginalDragStartIndex;
	const int32 TargetIndex = InventoryComp->TileToIndex(DraggedItemTopLeftTile);

	// --- Thử thả vào vị trí mới ---
	if (InventoryComp->IsRoomAvaiable(DroppedItem, TargetIndex))
	{
		InventoryComp->AddItemAt(DroppedItem, TargetIndex);
		goto Cleanup;
	}

	// --- Vị trí mới không hợp lệ: trả về vị trí gốc ---
	if (OriginalIndex != -1)
	{
		if (InventoryComp->IsRoomAvaiable(DroppedItem, OriginalIndex))
		{
			InventoryComp->AddItemAt(DroppedItem, OriginalIndex);
			goto Cleanup;
		}

		// Vị trí gốc không vừa vì đã xoay → xoay lại rồi thêm
		DroppedItem->RotateItem();
		InventoryComp->AddItemAt(DroppedItem, OriginalIndex);
		goto Cleanup;
	}

	// Không thể xử lý → trả false để InventoryWidget bắt
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
//  Drag enter — lưu operation để dùng khi nhấn R
// ─────────────────────────────────────────────────────────────────

void UInventoryGirdWidget::NativeOnDragEnter(const FGeometry& InGeo,
	const FDragDropEvent& InDDE,
	UDragDropOperation* InOp)
{
	Super::NativeOnDragEnter(InGeo, InDDE, InOp);
	StoredDragOp = InOp;
}

// ─────────────────────────────────────────────────────────────────
//  Phím R = xoay item đang kéo
// ─────────────────────────────────────────────────────────────────

FReply UInventoryGirdWidget::NativeOnPreviewKeyDown(const FGeometry& InGeo,
	const FKeyEvent& InKey)
{
	if (InKey.GetKey() == EKeys::Tab)
	{
		if (DraggedItem || StoredDragOp)
		{
			AItemBase* Item = DraggedItem;

			int32 OriginalIndex = -1;
			if (InventoryComp)
			{
				OriginalIndex = InventoryComp->OriginalDragStartIndex;
			}

			UWidgetBlueprintLibrary::CancelDragDrop();

			if (Item && InventoryComp && OriginalIndex != -1)
			{
				InventoryComp->AddItemAt(Item, OriginalIndex);
			}
			DraggedItem = nullptr;
			StoredDragOp = nullptr;

			if (InventoryComp)
			{
				InventoryComp->OriginalDragStartIndex = -1;
				InventoryComp->DraggedItem_Internal = nullptr;
			}
		}

		if (CharacterRef)
		{
			CharacterRef->ToggleInventory();
		}

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
//  Helper
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
	return IsValid(Item) && InventoryComp && InventoryComp->IsRoomAvaiable(Item, InventoryComp->TileToIndex(TopLeft));
}