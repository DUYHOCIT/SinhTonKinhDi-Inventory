// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Inventory/InventoryDataStructs.h"
#include "InventoryComponent.generated.h"

class AItemBase;
class UInventoryGirdWidget;
class UToolInventoryWidget;
class UInformationWidget;
class UChildActorComponent;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class SINHTONKINHDI_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInventoryComponent();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// ────── Kho riêng từng loại (Equipment / Food / Other) ──────
	UPROPERTY() TMap<AItemBase*, int32> EquipmentItemsMap;
	TSet<int32>                         EquipmentTiles;

	UPROPERTY() TMap<AItemBase*, int32> FoodItemsMap;
	TSet<int32>                         FoodTiles;

	UPROPERTY() TMap<AItemBase*, int32> OtherItemsMap;
	TSet<int32>                         OtherTiles;

	// ────── Tool belt ────────────────────────────────────────────
	UPROPERTY() TArray<AItemBase*> ToolSlots;

public:
	// ────── Grid config ──────────────────────────────────────────
	UPROPERTY(EditAnywhere, Category = "Inventory|Config") int32  Columns  = 10;
	UPROPERTY(EditAnywhere, Category = "Inventory|Config") int32  Rows     = 6;
	UPROPERTY(EditAnywhere, Category = "Inventory|Config") float  TileSize = 64.f;

	// ────── Drag state ───────────────────────────────────────────
	UPROPERTY() AItemBase* DraggedItem_Internal   = nullptr;
	int32 OriginalDragStartIndex                  = -1;
	int32 OriginalToolSlotIndex                   = -1;

	// ────── Grid API ─────────────────────────────────────────────
	bool TryAddItem(AItemBase* Item);
	bool TryAddItem_NoRotate(AItemBase* Item);
	void AddItemAt(AItemBase* Item, int32 TopLeftIndex);
	void RemoveFromGrid(AItemBase* Item);
	void DropItemToWorld(AItemBase* Item);
	bool IsRoomAvaiable(AItemBase* Item, int32 TopLeftIndex) const;

	/** Trả về danh sách item + vị trí tile của một loại cụ thể. */
	TMap<AItemBase*, FIntPoint> GetItemsForCategory(EItemCategory Cat) const;

	// ────── Tool Slot API ────────────────────────────────────────
	bool       TryAddToToolSlot(AItemBase* Item);
	bool       EquipToolAt(AItemBase* Item, int32 SlotIndex);
	void       UnequipTool(int32 SlotIndex);
	AItemBase* GetToolAt(int32 SlotIndex) const;
	AItemBase* RemoveFromToolSlot(int32 SlotIndex);
	void       PerformToolSlotDrop(AItemBase* IncomingItem, int32 FromSlot, int32 ToSlot);
	void       ResizeToolSlots(int32 NewCount);
	int32      GetToolSlotCount() const { return ToolSlots.Num(); }

	UPROPERTY() int32 ActiveToolSlotIndex = 0;
	void SetActiveToolSlot(int32 Index);
	void ScrollToolSlot(int32 Direction);

	// ────── Coordinate helpers ───────────────────────────────────
	FIntPoint IndexToTile(int32 Index) const;
	int32     TileToIndex(FIntPoint Tile) const;
	bool      IsTileValid(FIntPoint Tile) const;

	// ────── Widget refs ──────────────────────────────────────────

	/** Grid widget riêng cho từng loại. Đặt bởi NativeConstruct của grid. */
	UInventoryGirdWidget* GridWidgetRef_Equipment = nullptr;
	UInventoryGirdWidget* GridWidgetRef_Food      = nullptr;
	UInventoryGirdWidget* GridWidgetRef_Other     = nullptr;

	UPROPERTY() UToolInventoryWidget* ToolWidgetRef           = nullptr;
	            UInformationWidget*   InformationWidgetRef    = nullptr;
	            AItemBase*            ItemSelecting           = nullptr;
				UChildActorComponent* WeaponChild;
				void RefreshItemSelecting();

	void SetInventoryGridWidget(UInventoryGirdWidget* Widget, EItemCategory Category);
	void SetToolInventoryWidget(UToolInventoryWidget* Widget);

	/** Refresh tất cả 3 grid (dùng khi cần cập nhật toàn bộ). */
	void RefreshAllGrids();

	/** Xoá ItemSelected trên cả 3 grid (khi đóng kho). */
	void ClearAllGridSelections();

private:
	// Helpers trả về map / tileset / widget ref theo category
	TMap<AItemBase*, int32>&       GetMapRef  (EItemCategory Cat);
	const TMap<AItemBase*, int32>& GetMapRef  (EItemCategory Cat) const;
	TSet<int32>&                   GetTilesRef(EItemCategory Cat);
	const TSet<int32>&             GetTilesRef(EItemCategory Cat) const;
	UInventoryGirdWidget*          GetGridRef (EItemCategory Cat) const;

	void MarkTiles  (AItemBase* Item, int32 TopLeftIndex);
	void UnmarkTiles(AItemBase* Item, int32 TopLeftIndex);
};