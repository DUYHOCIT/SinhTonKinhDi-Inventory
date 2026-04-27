// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Border.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Inventory/InventoryDataStructs.h"
#include "InventoryGirdWidget.generated.h"

class ASinhTonKinhDiCharacter;
class UInventoryComponent;
class AItemBase;
class UPanelSlot;
class UItemWidget;
class UInformationWidget;

UCLASS()
class SINHTONKINHDI_API UInventoryGirdWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// ────── Category của grid này ────────────────────────────────
	/**
	 * Đặt trong Blueprint Details Panel.
	 * Equipment grid → Equipment (mặc định)
	 * Food grid      → Food
	 * Other grid     → Other
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	EItemCategory GridCategory = EItemCategory::Equipment;

	// ────── Public API ───────────────────────────────────────────

	/** Xóa và vẽ lại toàn bộ item widget thuộc GridCategory. */
	void Refresh();

	// ────── Drag state ───────────────────────────────────────────
	UPROPERTY() AItemBase* DraggedItem = nullptr;
	UPROPERTY() AItemBase* DroppedItem = nullptr;

	// ────── Item selection ───────────────────────────────────────
	UItemWidget* ItemSelected = nullptr;
	void SetItemSelected(UItemWidget* NewSelected);
	void SetNullItemSelected();

protected:
	UPROPERTY(meta = (BindWidget)) UCanvasPanel* Canvas;
	UPROPERTY(meta = (BindWidget)) UBorder*      GirdBorder;
	UPROPERTY(meta = (BindWidget)) UCanvasPanel* GirdCanvasPanel;

	UPROPERTY() ASinhTonKinhDiCharacter* CharacterRef  = nullptr;
	UPROPERTY() UInventoryComponent*     InventoryComp = nullptr;
	UPROPERTY() UDragDropOperation*      StoredDragOp  = nullptr;
	UPROPERTY() UPanelSlot*              PanelSlot     = nullptr;

	int32 Columns  = 0;
	int32 Rows     = 0;
	float TileSize = 64.f;

	// ────── Grid lines (cached) ──────────────────────────────────
	TArray<float> StartX, StartY, EndX, EndY;
	FIntPoint     DraggedItemTopLeftTile;
	FMousePositionInTile MouseInTile;

	// ────── Overrides ────────────────────────────────────────────
	virtual void    NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual void    NativeConstruct() override;

	virtual int32   NativePaint(const FPaintArgs&, const FGeometry&,
	                            const FSlateRect&, FSlateWindowElementList&,
	                            int32, const FWidgetStyle&, bool) const override;

	virtual bool    NativeOnDrop(const FGeometry&, const FDragDropEvent&, UDragDropOperation*) override;
	virtual bool    NativeOnDragOver(const FGeometry&, const FDragDropEvent&, UDragDropOperation*) override;
	virtual void    NativeOnDragEnter(const FGeometry&, const FDragDropEvent&, UDragDropOperation*) override;
	virtual FReply  NativeOnMouseButtonDown(const FGeometry&, const FPointerEvent&) override;
	virtual FReply  NativeOnPreviewKeyDown(const FGeometry&, const FKeyEvent&) override;

private:
	void BuildLineSegments();
	FMousePositionInTile GetMousePositionInTile(FVector2D LocalPos) const;
	bool IsRoomAvailableAt(AItemBase* Item, FIntPoint TopLeft) const;
};