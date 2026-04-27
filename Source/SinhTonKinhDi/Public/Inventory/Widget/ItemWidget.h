// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ItemWidget.generated.h"

class UCanvasPanel;
class USizeBox;
class UBorder;
class UImage;
class AItemBase;
class UInventoryComponent;
class UInventoryGirdWidget;

UCLASS()
class SINHTONKINHDI_API UItemWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Cập nhật hình ảnh + kích thước widget theo item. */
	void Refresh(AItemBase* InItem, UInventoryComponent* InInventory);

	/** Trả về item đang được đại diện (dùng bởi InformationWidget). */
	FORCEINLINE AItemBase* GetItem() const { return Item; }

	UInventoryGirdWidget* GirdWidget = nullptr;

protected:
	UPROPERTY(meta = (BindWidget)) UCanvasPanel* Canvas;
	UPROPERTY(meta = (BindWidget)) USizeBox*     BackgroundSize;
	UPROPERTY(meta = (BindWidget)) UBorder*      BackgroundBorder;
	UPROPERTY(meta = (BindWidget)) UImage*       ItemImage;

	UPROPERTY() AItemBase*          Item      = nullptr;
	UPROPERTY() UInventoryComponent* Inventory = nullptr;

	virtual void   NativeOnMouseEnter(const FGeometry&, const FPointerEvent&) override;
	virtual void   NativeOnMouseLeave(const FPointerEvent&) override;
	virtual void   NativeOnDragDetected(const FGeometry&, const FPointerEvent&, UDragDropOperation*&) override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry&, const FPointerEvent&) override;
	virtual FReply NativeOnMouseButtonUp(const FGeometry&, const FPointerEvent&) override;
};