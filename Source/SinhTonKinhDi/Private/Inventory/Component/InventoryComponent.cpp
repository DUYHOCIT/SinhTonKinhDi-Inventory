// Fill out your copyright notice in the Description page of Project Settings.
#include "Inventory/Component/InventoryComponent.h"
#include "Inventory/Item/ItemBase.h"
#include "Inventory/Widget/InventoryGirdWidget.h"

UInventoryComponent::UInventoryComponent()
{
	// Không cần Tick nữa — debug print đã bị xoá
	PrimaryComponentTick.bCanEverTick = false;
}

void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UInventoryComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	// Destroy tất cả item còn trong inventory
	for (auto& [Item, _] : ItemsMap)
	{
		if (IsValid(Item))
			Item->Destroy();
	}
	ItemsMap.Empty();
	OccupiedTiles.Empty();
}

// ─────────────────────────────────────────────────────────────────
//  Tiện ích toạ độ
// ─────────────────────────────────────────────────────────────────

FIntPoint UInventoryComponent::IndexToTile(int32 Index) const
{
	return FIntPoint(Index % Columns, Index / Columns);
}

int32 UInventoryComponent::TileToIndex(FIntPoint Tile) const
{
	return Tile.Y * Columns + Tile.X;
}

bool UInventoryComponent::IsTileValid(FIntPoint Tile) const
{
	return Tile.X >= 0 && Tile.X < Columns && Tile.Y >= 0 && Tile.Y < Rows;
}

// ─────────────────────────────────────────────────────────────────
//  Đánh dấu / bỏ đánh dấu tile
// ─────────────────────────────────────────────────────────────────

void UInventoryComponent::MarkTiles(AItemBase* Item, int32 TopLeftIndex)
{
	FIntPoint Origin = IndexToTile(TopLeftIndex);
	FIntPoint Dim = Item->GetDimensions();

	for (int32 dx = 0; dx < Dim.X; ++dx)
		for (int32 dy = 0; dy < Dim.Y; ++dy)
			OccupiedTiles.Add(TileToIndex(Origin + FIntPoint(dx, dy)));
}

void UInventoryComponent::UnmarkTiles(AItemBase* Item, int32 TopLeftIndex)
{
	FIntPoint Origin = IndexToTile(TopLeftIndex);
	FIntPoint Dim = Item->GetDimensions();

	for (int32 dx = 0; dx < Dim.X; ++dx)
		for (int32 dy = 0; dy < Dim.Y; ++dy)
			OccupiedTiles.Remove(TileToIndex(Origin + FIntPoint(dx, dy)));
}

// ─────────────────────────────────────────────────────────────────
//  Kiểm tra chỗ trống
// ─────────────────────────────────────────────────────────────────

bool UInventoryComponent::IsRoomAvaiable(AItemBase* Item, int32 TopLeftIndex) const
{
	if (!IsValid(Item)) return false;

	FIntPoint Origin = IndexToTile(TopLeftIndex);
	FIntPoint Dim = Item->GetDimensions();

	// Kiểm tra nằm trong biên
	if (Origin.X + Dim.X > Columns || Origin.Y + Dim.Y > Rows)
		return false;

	// Kiểm tra từng tile — O(Dim.X * Dim.Y), lookup O(1) nhờ TSet
	for (int32 dx = 0; dx < Dim.X; ++dx)
	{
		for (int32 dy = 0; dy < Dim.Y; ++dy)
		{
			FIntPoint Tile = Origin + FIntPoint(dx, dy);
			if (!IsTileValid(Tile)) return false;
			if (OccupiedTiles.Contains(TileToIndex(Tile))) return false;
		}
	}
	return true;
}

// ─────────────────────────────────────────────────────────────────
//  Thêm item
// ─────────────────────────────────────────────────────────────────

bool UInventoryComponent::TryAddItem_NoRotate(AItemBase* Item)
{
	if (!IsValid(Item)) return false;

	const int32 Total = Columns * Rows;
	for (int32 i = 0; i < Total; ++i)
	{
		if (IsRoomAvaiable(Item, i))
		{
			AddItemAt(Item, i);
			return true;
		}
	}
	return false;
}

bool UInventoryComponent::TryAddItem(AItemBase* Item)
{
	if (!IsValid(Item)) return false;

	// Thử trạng thái xoay hiện tại
	if (TryAddItem_NoRotate(Item)) return true;

	// Thử xoay
	Item->RotateItem();
	if (TryAddItem_NoRotate(Item)) return true;

	// Thất bại: khôi phục xoay gốc
	Item->RotateItem();
	return false;
}

void UInventoryComponent::AddItemAt(AItemBase* Item, int32 TopLeftIndex)
{
	if (!IsValid(Item)) return;

	ItemsMap.Add(Item, TopLeftIndex);
	MarkTiles(Item, TopLeftIndex);

	if (GridWidgetRef)
		GridWidgetRef->Refresh();
}

// ─────────────────────────────────────────────────────────────────
//  Xoá item
// ─────────────────────────────────────────────────────────────────

void UInventoryComponent::RemoveFromGrid(AItemBase* Item)
{
	// Chỉ xoá khỏi map/set, KHÔNG Destroy — dùng khi bắt đầu kéo
	if (!IsValid(Item)) return;

	if (const int32* TopLeft = ItemsMap.Find(Item))
	{
		UnmarkTiles(Item, *TopLeft);
		ItemsMap.Remove(Item);
	}

	if (GridWidgetRef)
		GridWidgetRef->Refresh();
}

void UInventoryComponent::DropItemToWorld(AItemBase* Item)
{
	// Xoá khỏi grid và Destroy actor — dùng khi vứt ra thế giới
	if (!IsValid(Item)) return;

	RemoveFromGrid(Item);
	Item->Destroy();
}

// ─────────────────────────────────────────────────────────────────
//  Lấy dữ liệu
// ─────────────────────────────────────────────────────────────────

TMap<AItemBase*, FIntPoint> UInventoryComponent::GetAllItems() const
{
	TMap<AItemBase*, FIntPoint> Result;
	Result.Reserve(ItemsMap.Num());

	for (auto& [Item, Index] : ItemsMap)
	{
		if (IsValid(Item))
			Result.Add(Item, IndexToTile(Index));
	}
	return Result;
}

// ─────────────────────────────────────────────────────────────────
//  Widget
// ─────────────────────────────────────────────────────────────────

void UInventoryComponent::SetInventoryGridWidget(UInventoryGirdWidget* Widget)
{
	GridWidgetRef = Widget;
}