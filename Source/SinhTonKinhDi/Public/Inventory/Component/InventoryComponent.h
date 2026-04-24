// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InventoryComponent.generated.h"

class AItemBase;
class UInventoryGirdWidget;
class UInformationWidget;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class SINHTONKINHDI_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInventoryComponent();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// --- STORAGE MỚI: Map item -> top-left index, Set tile đang bị chiếm ---
	// Không còn TArray với pointer trùng lặp nữa
	UPROPERTY()
	TMap<AItemBase*, int32> ItemsMap;   // item -> chỉ số top-left
	TSet<int32>              OccupiedTiles; // tile nào đang bị chiếm (lookup O(1))

public:
	// Grid config
	UPROPERTY(EditAnywhere, Category = "Inventory|Config")
	int32 Columns = 10;
	UPROPERTY(EditAnywhere, Category = "Inventory|Config")
	int32 Rows = 6;
	UPROPERTY(EditAnywhere, Category = "Inventory|Config")
	float TileSize = 64.f;

	// Drag state (set bởi ItemWidget, đọc bởi GridWidget)
	UPROPERTY()
	AItemBase* DraggedItem_Internal = nullptr;
	int32       OriginalDragStartIndex = -1;

	// --- API chính ---

	/** Thêm item vào vị trí bất kỳ còn trống, tự thử xoay nếu cần. */
	bool TryAddItem(AItemBase* Item);

	/** Thêm item vào vị trí bất kỳ còn trống, KHÔNG xoay. */
	bool TryAddItem_NoRotate(AItemBase* Item);

	/** Đặt item vào đúng ô TopLeftIndex (đã biết là trống). */
	void AddItemAt(AItemBase* Item, int32 TopLeftIndex);

	/** Chỉ xoá khỏi grid, KHÔNG Destroy actor — dùng khi bắt đầu kéo. */
	void RemoveFromGrid(AItemBase* Item);

	/** Xoá khỏi grid VÀ Destroy actor — dùng khi vứt đồ ra thế giới. */
	void DropItemToWorld(AItemBase* Item);

	/** Kiểm tra xem item có vừa vào TopLeftIndex không. */
	bool IsRoomAvaiable(AItemBase* Item, int32 TopLeftIndex) const;

	/** Trả về map item -> tile (dùng cho widget Refresh). */
	TMap<AItemBase*, FIntPoint> GetAllItems() const;

	// Tiện ích toạ độ
	FIntPoint IndexToTile(int32 Index) const;
	int32     TileToIndex(FIntPoint Tile) const;
	bool      IsTileValid(FIntPoint Tile) const;

	void SetInventoryGridWidget(UInventoryGirdWidget* Widget);

	UInventoryGirdWidget* GridWidgetRef = nullptr;
	UInformationWidget* InformationWidgetRef;
private:

	/** Đánh dấu các tile mà item chiếm vào OccupiedTiles. */
	void MarkTiles(AItemBase* Item, int32 TopLeftIndex);

	/** Bỏ đánh dấu các tile của item khỏi OccupiedTiles. */
	void UnmarkTiles(AItemBase* Item, int32 TopLeftIndex);
};