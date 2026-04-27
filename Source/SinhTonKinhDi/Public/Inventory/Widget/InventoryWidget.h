// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/CanvasPanel.h"
#include "Components/Border.h"
#include "Components/BackgroundBlur.h"
#include "Components/Button.h"
#include "Inventory/InventoryDataStructs.h"
#include "InventoryWidget.generated.h"

class UBorder;
class UBackgroundBlur;
class ASinhTonKinhDiCharacter;
class AItemBase;
class UInventoryGirdWidget;
class UToolInventoryWidget;
class UInformationWidget;

UCLASS()
class SINHTONKINHDI_API UInventoryWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, meta = (BindWidget), Category = "UI")
	UCanvasPanel* Canvas;

	UPROPERTY(VisibleAnywhere, meta = (BindWidget), Category = "UI")
	UBorder* BackgroundBorder;

	UPROPERTY(VisibleAnywhere, meta = (BindWidget), Category = "UI")
	UBackgroundBlur* Blur;

	// ────── 3 grid riêng biệt theo category ─────────────────────

	/** Kho Trang bị (Equipment). */
	UPROPERTY(VisibleAnywhere, meta = (BindWidget), Category = "UI")
	UInventoryGirdWidget* Grid_Equipment;

	/** Kho Đồ ăn (Food). */
	UPROPERTY(VisibleAnywhere, meta = (BindWidget), Category = "UI")
	UInventoryGirdWidget* Grid_Food;

	/** Kho Đồ khác (Other). */
	UPROPERTY(VisibleAnywhere, meta = (BindWidget), Category = "UI")
	UInventoryGirdWidget* Grid_Other;

	/** Tool belt — không thay đổi so với trước. */
	UPROPERTY(VisibleAnywhere, meta = (BindWidget), Category = "UI")
	UToolInventoryWidget* ToolBelt;

	UPROPERTY(VisibleAnywhere, meta = (BindWidget), Category = "UI")
	UInformationWidget* InformationWidgetRef;

	// ────── Tab buttons ──────────────────────────────────────────

	/** Tab Trang bị (thay thế "Tất cả"). */
	UPROPERTY(meta = (BindWidget)) UButton* Tab_Equipment;
	UPROPERTY(meta = (BindWidget)) UButton* Tab_Tool;
	UPROPERTY(meta = (BindWidget)) UButton* Tab_Food;
	UPROPERTY(meta = (BindWidget)) UButton* Tab_Other;



	UPROPERTY(EditDefaultsOnly, Category = "UI|Tabs")
	FLinearColor ActiveTabColor = FLinearColor(0.8f, 0.6f, 0.1f, 1.f);

	UPROPERTY(EditDefaultsOnly, Category = "UI|Tabs")
	FLinearColor InactiveTabColor = FLinearColor(0.15f, 0.15f, 0.15f, 0.8f);

protected:
	ASinhTonKinhDiCharacter* CharacterReference = nullptr;

	virtual void NativeConstruct() override;
	virtual bool NativeOnDrop(const FGeometry&, const FDragDropEvent&,
	                           UDragDropOperation*) override;

private:
	UFUNCTION() void OnTabEquipment();
	UFUNCTION() void OnTabTool();
	UFUNCTION() void OnTabFood();
	UFUNCTION() void OnTabOther();

	/** Ẩn tất cả grid + tool belt, sau đó hiện đúng grid được yêu cầu. */
	void ShowGridForCategory(EItemCategory Cat);

	/** Ẩn tất cả grid, hiện tool belt. */
	void ShowToolBelt();

	void UpdateTabHighlight(UButton* ActiveBtn);

	EItemCategory CurrentTab = EItemCategory::Equipment;
};