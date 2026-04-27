// Fill out your copyright notice in the Description page of Project Settings.
#include "Inventory/Component/InventoryComponent.h"
#include "Inventory/Item/ItemBase.h"
#include "Inventory/Widget/InventoryGirdWidget.h"
#include "Inventory/Widget/ToolInventoryWidget.h"

UInventoryComponent::UInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();
	ToolSlots.Empty();
}

void UInventoryComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	auto DestroyMap = [](TMap<AItemBase*, int32>& Map)
		{
			for (auto& [Item, _] : Map)
				if (IsValid(Item)) Item->Destroy();
			Map.Empty();
		};

	DestroyMap(EquipmentItemsMap);
	DestroyMap(FoodItemsMap);
	DestroyMap(OtherItemsMap);

	EquipmentTiles.Empty();
	FoodTiles.Empty();
	OtherTiles.Empty();

	for (AItemBase* Item : ToolSlots)
		if (IsValid(Item)) Item->Destroy();
	ToolSlots.Empty();
}

// ─────────────────────────────────────────────────────────────────
//  Category helpers
// ─────────────────────────────────────────────────────────────────

TMap<AItemBase*, int32>& UInventoryComponent::GetMapRef(EItemCategory Cat)
{
	switch (Cat)
	{
	case EItemCategory::Food:  return FoodItemsMap;
	case EItemCategory::Other: return OtherItemsMap;
	default:                   return EquipmentItemsMap; // Equipment
	}
}

const TMap<AItemBase*, int32>& UInventoryComponent::GetMapRef(EItemCategory Cat) const
{
	switch (Cat)
	{
	case EItemCategory::Food:  return FoodItemsMap;
	case EItemCategory::Other: return OtherItemsMap;
	default:                   return EquipmentItemsMap;
	}
}

TSet<int32>& UInventoryComponent::GetTilesRef(EItemCategory Cat)
{
	switch (Cat)
	{
	case EItemCategory::Food:  return FoodTiles;
	case EItemCategory::Other: return OtherTiles;
	default:                   return EquipmentTiles;
	}
}

const TSet<int32>& UInventoryComponent::GetTilesRef(EItemCategory Cat) const
{
	switch (Cat)
	{
	case EItemCategory::Food:  return FoodTiles;
	case EItemCategory::Other: return OtherTiles;
	default:                   return EquipmentTiles;
	}
}

UInventoryGirdWidget* UInventoryComponent::GetGridRef(EItemCategory Cat) const
{
	switch (Cat)
	{
	case EItemCategory::Food:  return GridWidgetRef_Food;
	case EItemCategory::Other: return GridWidgetRef_Other;
	default:                   return GridWidgetRef_Equipment;
	}
}

// ─────────────────────────────────────────────────────────────────
//  Coordinate helpers
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
//  Tile marking (dùng đúng tileset theo category của item)
// ─────────────────────────────────────────────────────────────────

void UInventoryComponent::MarkTiles(AItemBase* Item, int32 TopLeftIndex)
{
	TSet<int32>& Tiles = GetTilesRef(Item->ItemCategory);
	FIntPoint Origin = IndexToTile(TopLeftIndex);
	FIntPoint Dim = Item->GetDimensions();
	for (int32 dx = 0; dx < Dim.X; ++dx)
		for (int32 dy = 0; dy < Dim.Y; ++dy)
			Tiles.Add(TileToIndex(Origin + FIntPoint(dx, dy)));
}

void UInventoryComponent::UnmarkTiles(AItemBase* Item, int32 TopLeftIndex)
{
	TSet<int32>& Tiles = GetTilesRef(Item->ItemCategory);
	FIntPoint Origin = IndexToTile(TopLeftIndex);
	FIntPoint Dim = Item->GetDimensions();
	for (int32 dx = 0; dx < Dim.X; ++dx)
		for (int32 dy = 0; dy < Dim.Y; ++dy)
			Tiles.Remove(TileToIndex(Origin + FIntPoint(dx, dy)));
}

// ─────────────────────────────────────────────────────────────────
//  Room check (dùng tileset riêng theo category)
// ─────────────────────────────────────────────────────────────────

bool UInventoryComponent::IsRoomAvaiable(AItemBase* Item, int32 TopLeftIndex) const
{
	if (!IsValid(Item)) return false;

	const TSet<int32>& Tiles = GetTilesRef(Item->ItemCategory);
	FIntPoint Origin = IndexToTile(TopLeftIndex);
	FIntPoint Dim = Item->GetDimensions();

	if (Origin.X + Dim.X > Columns || Origin.Y + Dim.Y > Rows) return false;

	for (int32 dx = 0; dx < Dim.X; ++dx)
		for (int32 dy = 0; dy < Dim.Y; ++dy)
		{
			FIntPoint Tile = Origin + FIntPoint(dx, dy);
			if (!IsTileValid(Tile))               return false;
			if (Tiles.Contains(TileToIndex(Tile))) return false;
		}
	return true;
}

// ─────────────────────────────────────────────────────────────────
//  Grid add (mỗi loại vào kho riêng)
// ─────────────────────────────────────────────────────────────────

bool UInventoryComponent::TryAddItem_NoRotate(AItemBase* Item)
{
	if (!IsValid(Item)) return false;
	const int32 Total = Columns * Rows;
	for (int32 i = 0; i < Total; ++i)
		if (IsRoomAvaiable(Item, i)) { AddItemAt(Item, i); return true; }
	return false;
}

bool UInventoryComponent::TryAddItem(AItemBase* Item)
{
	if (!IsValid(Item)) return false;

	if (Item->ItemCategory == EItemCategory::Tool)
		return TryAddToToolSlot(Item);

	if (TryAddItem_NoRotate(Item)) return true;
	Item->RotateItem();
	if (TryAddItem_NoRotate(Item)) return true;
	Item->RotateItem(); // khôi phục rotation gốc
	return false;
}

void UInventoryComponent::AddItemAt(AItemBase* Item, int32 TopLeftIndex)
{
	if (!IsValid(Item)) return;

	TMap<AItemBase*, int32>& Map = GetMapRef(Item->ItemCategory);
	Map.Add(Item, TopLeftIndex);
	MarkTiles(Item, TopLeftIndex);

	// Refresh đúng grid của loại item này
	if (UInventoryGirdWidget* Grid = GetGridRef(Item->ItemCategory))
		Grid->Refresh();
}

// ─────────────────────────────────────────────────────────────────
//  Grid remove
// ─────────────────────────────────────────────────────────────────

void UInventoryComponent::RemoveFromGrid(AItemBase* Item)
{
	if (!IsValid(Item)) return;

	TMap<AItemBase*, int32>& Map = GetMapRef(Item->ItemCategory);
	if (const int32* TopLeft = Map.Find(Item))
	{
		UnmarkTiles(Item, *TopLeft);
		Map.Remove(Item);
	}

	if (UInventoryGirdWidget* Grid = GetGridRef(Item->ItemCategory))
		Grid->Refresh();
}

void UInventoryComponent::DropItemToWorld(AItemBase* Item)
{
	if (!IsValid(Item)) return;
	RemoveFromGrid(Item);
	Item->Destroy();
}

TMap<AItemBase*, FIntPoint> UInventoryComponent::GetItemsForCategory(EItemCategory Cat) const
{
	const TMap<AItemBase*, int32>& Map = GetMapRef(Cat);
	TMap<AItemBase*, FIntPoint> Result;
	Result.Reserve(Map.Num());
	for (auto& [Item, Index] : Map)
		if (IsValid(Item)) Result.Add(Item, IndexToTile(Index));
	return Result;
}

// ─────────────────────────────────────────────────────────────────
//  Tool slot
// ─────────────────────────────────────────────────────────────────

bool UInventoryComponent::TryAddToToolSlot(AItemBase* Item)
{
	if (!IsValid(Item)) return false;
	for (int32 i = 0; i < ToolSlots.Num(); ++i)
	{
		if (!IsValid(ToolSlots[i]))
		{
			ToolSlots[i] = Item;
			Item->SetInInventory(true);
			
			if (ToolWidgetRef) ToolWidgetRef->Refresh();
			ItemSelecting = GetToolAt(ActiveToolSlotIndex);
			RefreshItemSelecting();
			return true;
		}
	}
	return false;
}

bool UInventoryComponent::EquipToolAt(AItemBase* Item, int32 SlotIndex)
{
	if (!IsValid(Item)) return false;
	if (SlotIndex < 0 || SlotIndex >= ToolSlots.Num()) return false;

	if (AItemBase* OldItem = ToolSlots[SlotIndex])
	{
		ToolSlots[SlotIndex] = nullptr;
		if (!TryAddItem(OldItem))
			OldItem->SetInInventory(false);
	}

	ToolSlots[SlotIndex] = Item;
	Item->SetInInventory(true);

	OriginalDragStartIndex = -1;
	OriginalToolSlotIndex = -1;
	DraggedItem_Internal = nullptr;

	if (ToolWidgetRef) ToolWidgetRef->Refresh();
	RefreshAllGrids();
	return true;
}

void UInventoryComponent::UnequipTool(int32 SlotIndex)
{
	if (SlotIndex < 0 || SlotIndex >= ToolSlots.Num()) return;
	AItemBase* Item = ToolSlots[SlotIndex];
	if (!IsValid(Item)) return;

	ToolSlots[SlotIndex] = nullptr;
	if (!TryAddItem(Item)) Item->SetInInventory(false);

	if (ToolWidgetRef) ToolWidgetRef->Refresh();
	RefreshAllGrids();
}

AItemBase* UInventoryComponent::GetToolAt(int32 SlotIndex) const
{
	if (SlotIndex < 0 || SlotIndex >= ToolSlots.Num()) return nullptr;
	return ToolSlots[SlotIndex];
}

AItemBase* UInventoryComponent::RemoveFromToolSlot(int32 SlotIndex)
{
	if (SlotIndex < 0 || SlotIndex >= ToolSlots.Num()) return nullptr;
	AItemBase* Item = ToolSlots[SlotIndex];
	if (!IsValid(Item)) return nullptr;
	ToolSlots[SlotIndex] = nullptr;
	if (Item == ItemSelecting)
	{
		ItemSelecting = nullptr;
		RefreshItemSelecting();
	}
	if (ToolWidgetRef) ToolWidgetRef->Refresh();
	return Item;
}

void UInventoryComponent::PerformToolSlotDrop(AItemBase* IncomingItem,
	int32 FromSlot, int32 ToSlot)
{
	if (!IsValid(IncomingItem)) return;
	if (ToSlot < 0 || ToSlot >= ToolSlots.Num()) return;

	AItemBase* DisplacedItem = ToolSlots[ToSlot];
	ToolSlots[ToSlot] = IncomingItem;
	IncomingItem->SetInInventory(true);

	if (IsValid(DisplacedItem) && FromSlot >= 0 && FromSlot < ToolSlots.Num())
	{
		ToolSlots[FromSlot] = DisplacedItem;
		DisplacedItem->SetInInventory(true);
	}
	
	OriginalToolSlotIndex = -1;
	OriginalDragStartIndex = -1;
	DraggedItem_Internal = nullptr;
	RefreshItemSelecting();
	if (ToolWidgetRef) ToolWidgetRef->Refresh();
}

void UInventoryComponent::ResizeToolSlots(int32 NewCount)
{
	const int32 OldCount = ToolSlots.Num();
	if (NewCount == OldCount) return;

	if (NewCount < OldCount)
	{
		for (int32 i = NewCount; i < OldCount; ++i)
		{
			if (IsValid(ToolSlots[i]))
			{
				AItemBase* Item = ToolSlots[i];
				ToolSlots[i] = nullptr;
				if (!TryAddItem_NoRotate(Item))
				{
					Item->RotateItem();
					if (!TryAddItem_NoRotate(Item))
					{
						Item->RotateItem();
						Item->SetInInventory(false);
					}
				}
			}
		}
	}
	ToolSlots.SetNum(NewCount);
}

// ─────────────────────────────────────────────────────────────────
//  Active tool slot
// ─────────────────────────────────────────────────────────────────

void UInventoryComponent::SetActiveToolSlot(int32 Index)
{
	if (Index < 0 || Index >= ToolSlots.Num()) return;
	ActiveToolSlotIndex = Index;
	if (ToolWidgetRef) ToolWidgetRef->Refresh();
}

void UInventoryComponent::ScrollToolSlot(int32 Direction)
{
	const int32 Count = ToolSlots.Num();
	if (Count == 0) return;
	ActiveToolSlotIndex = (ActiveToolSlotIndex - Direction + Count) % Count;
	ItemSelecting = GetToolAt(ActiveToolSlotIndex);
	RefreshItemSelecting();
	if (ToolWidgetRef) ToolWidgetRef->Refresh();
}

// ─────────────────────────────────────────────────────────────────
//  Widget registration
// ─────────────────────────────────────────────────────────────────

void UInventoryComponent::RefreshItemSelecting()
{
	if (WeaponChild)
	{
		ItemSelecting = GetToolAt(ActiveToolSlotIndex);
		if (ItemSelecting)
		{
			WeaponChild->SetChildActorClass(ItemSelecting->GetClass());
			AItemBase* Itemtg = Cast<AItemBase>(WeaponChild->GetChildActor());

			if (Itemtg)
			{
				Itemtg->SetActorEnableCollision(false);
			}
		}
		else
		{
			WeaponChild->SetChildActorClass(nullptr);
		}
	}
}

void UInventoryComponent::SetInventoryGridWidget(UInventoryGirdWidget* Widget, EItemCategory Category)
{
	switch (Category)
	{
	case EItemCategory::Food:  GridWidgetRef_Food = Widget; break;
	case EItemCategory::Other: GridWidgetRef_Other = Widget; break;
	default:                   GridWidgetRef_Equipment = Widget; break;
	}
}

void UInventoryComponent::SetToolInventoryWidget(UToolInventoryWidget* Widget)
{
	ToolWidgetRef = Widget;
}

void UInventoryComponent::RefreshAllGrids()
{
	if (GridWidgetRef_Equipment) GridWidgetRef_Equipment->Refresh();
	if (GridWidgetRef_Food)      GridWidgetRef_Food->Refresh();
	if (GridWidgetRef_Other)     GridWidgetRef_Other->Refresh();
}

void UInventoryComponent::ClearAllGridSelections()
{
	if (GridWidgetRef_Equipment) GridWidgetRef_Equipment->SetNullItemSelected();
	if (GridWidgetRef_Food)      GridWidgetRef_Food->SetNullItemSelected();
	if (GridWidgetRef_Other)     GridWidgetRef_Other->SetNullItemSelected();
}