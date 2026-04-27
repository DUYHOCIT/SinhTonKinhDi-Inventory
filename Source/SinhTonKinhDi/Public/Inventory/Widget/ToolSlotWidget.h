// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ToolSlotWidget.generated.h"

class AItemBase;
class UInventoryComponent;
class UToolInventoryWidget;
class UImage;
class UBorder;
class USizeBox;

/**
 * Widget hiển thị một công cụ được treo tại điểm neo (anchor canvas).
 *
 * Blueprint cần có:
 *   - ItemContainer  (USizeBox,  BindWidget)  — bọc icon, kích thước thay đổi theo item
 *   - ItemIcon       (UImage,    BindWidget)  — hình ảnh icon
 *   - SlotHighlight  (UBorder,   BindWidgetOptional) — viền active/hover (tuỳ chọn)
 *
 * Kích thước icon = Item->GetDimensions() * IconScale pixel
 * Ví dụ: Kiếm 1×4 tile + IconScale 80 → icon 80×320 px, treo ở giữa điểm neo.
 *
 * Thao tác:
 *   - Giữ chuột trái → kéo công cụ ra khỏi slot
 *   - Thả lên slot khác → đổi chỗ 2 công cụ
 *   - Thả ra ngoài (lên InventoryWidget) → vứt ra thế giới
 *   - Chuột phải → tháo về grid/world
 *   - Click trái không kéo → chọn slot active
 */
UCLASS()
class SINHTONKINHDI_API UToolSlotWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// ────── Blueprint bindings ───────────────────────────────────

	/** Bọc icon — kích thước được đặt theo item dimensions * IconScale. */
	UPROPERTY(meta = (BindWidget))
	USizeBox* ItemContainer = nullptr;

	/** Hình icon của item. */
	UPROPERTY(meta = (BindWidget))
	UImage* ItemIcon = nullptr;

	// THÊM MỚI: Hình ảnh hiển thị khi slot trống (vòng tròn/khung mặc định)
	UPROPERTY(meta = (BindWidget))
	UImage* EmptySlotIcon = nullptr;

	/** Viền highlight (tuỳ chọn — xóa khỏi Blueprint cũng được). */
	UPROPERTY(meta = (BindWidgetOptional))
	UBorder* SlotHighlight = nullptr;

	// ────── Runtime ──────────────────────────────────────────────
	UPROPERTY()
	UInventoryComponent* InventoryComp = nullptr;

	UPROPERTY()
	UToolInventoryWidget* ToolWidget = nullptr;

	int32      SlotIndex   = 0;
	AItemBase* SlottedItem = nullptr;

	/**
	 * Pixel per tile unit — set từ ToolInventoryWidget sau khi khởi tạo.
	 * Ví dụ: IconScale=80, item 1×3 → icon 80×240 px.
	 */
	float IconScale = 80.f;

	/**
	 * Kích thước tối thiểu của ItemContainer khi slot rỗng.
	 * Đủ lớn để nhận drop.
	 */
	float EmptySlotSize = 64.f;

	// ────── Style colors ─────────────────────────────────────────
	UPROPERTY(EditDefaultsOnly, Category = "Tool Belt|Style")
	FLinearColor NormalColor  = FLinearColor(0.f, 0.f, 0.f, 0.f);     // trong suốt

	UPROPERTY(EditDefaultsOnly, Category = "Tool Belt|Style")
	FLinearColor ActiveColor  = FLinearColor(0.8f, 0.6f, 0.1f, 0.6f); // vàng nhạt

	UPROPERTY(EditDefaultsOnly, Category = "Tool Belt|Style")
	FLinearColor HoverColor   = FLinearColor(1.f, 1.f, 1.f, 0.15f);   // trắng mờ

	// ────── Public API ───────────────────────────────────────────

	/** Khởi tạo slot — gọi ngay sau CreateWidget. */
	void Init(int32 InSlotIndex,
	          UInventoryComponent* InComp,
	          UToolInventoryWidget* InToolWidget,
	          float InIconScale,
	          float InEmptySlotSize);

	/**
	 * Cập nhật icon và kích thước container theo item.
	 * nullptr = slot rỗng → kích thước EmptySlotSize × EmptySlotSize.
	 */
	void SetItem(AItemBase* Item);

	/** Highlight viền khi slot được chọn. */
	void SetActive(bool bActive);

protected:
	virtual void NativeConstruct() override;

	/** Giữ chuột trái → bắt đầu kéo. */
	virtual void NativeOnDragDetected(const FGeometry&,
	                                   const FPointerEvent&,
	                                   UDragDropOperation*&) override;

	/** Nhấn chuột — bật DetectDrag hoặc tháo (chuột phải). */
	virtual FReply NativeOnMouseButtonDown(const FGeometry&,
	                                        const FPointerEvent&) override;

	/** Nhả chuột không kéo → chọn slot active. */
	virtual FReply NativeOnMouseButtonUp(const FGeometry&,
	                                      const FPointerEvent&) override;

	/** Nhận drop từ slot khác hoặc từ grid. */
	virtual bool NativeOnDrop(const FGeometry&,
	                           const FDragDropEvent&,
	                           UDragDropOperation*) override;

	virtual void NativeOnMouseEnter(const FGeometry&, const FPointerEvent&) override;
	virtual void NativeOnMouseLeave(const FPointerEvent&) override;

private:
	bool bIsActive    = false;
	bool bDragStarted = false;
};