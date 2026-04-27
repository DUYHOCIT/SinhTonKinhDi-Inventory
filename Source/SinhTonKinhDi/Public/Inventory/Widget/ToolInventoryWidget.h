// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/CanvasPanel.h"
#include "ToolInventoryWidget.generated.h"

class UInventoryComponent;
class UToolSlotWidget;
class ASinhTonKinhDiCharacter;
class UWidget;
class UCanvasPanelSlot;

UCLASS()
class SINHTONKINHDI_API UToolInventoryWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// ────── Blueprint binding ────────────────────────────────────

	UPROPERTY(meta = (BindWidget))
	UCanvasPanel* SlotsCanvas = nullptr;

	// ────── Editor config ────────────────────────────────────────

	UPROPERTY(EditDefaultsOnly, Category = "Tool Belt|Icon")
	float IconScale = 80.f;

	UPROPERTY(EditDefaultsOnly, Category = "Tool Belt|Icon")
	float EmptySlotSize = 64.f;

	/** Class Blueprint của UToolSlotWidget — bắt buộc phải gán. */
	UPROPERTY(EditDefaultsOnly, Category = "Tool Belt")
	TSubclassOf<UToolSlotWidget> ToolSlotClass;

	// ────── Runtime ──────────────────────────────────────────────
	UPROPERTY()
	UInventoryComponent* InventoryComp = nullptr;

	UPROPERTY()
	ASinhTonKinhDiCharacter* CharacterRef = nullptr;

	UPROPERTY()
	TArray<UToolSlotWidget*> Slots;

	// ────── Public API ───────────────────────────────────────────

	void Refresh();

protected:
	virtual void NativeConstruct() override;

	/**
	 * NativeTick dùng để khởi tạo slot SAU khi Slate đã layout xong
	 * (geometry của SlotsCanvas mới có kích thước thực).
	 */
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	/**
	 * FIX: Xử lý Tab để hủy drag + đóng kho khi đang ở tab Tool Belt.
	 * InventoryGirdWidget.NativeOnPreviewKeyDown không chạy khi Grid bị
	 * Collapsed → ToolInventoryWidget phải tự xử lý.
	 */
	virtual FReply NativeOnPreviewKeyDown(const FGeometry& InGeo,
	                                       const FKeyEvent& InKey) override;

private:
	/** Cờ đảm bảo CreateSlotsFromAnchors chỉ chạy 1 lần sau layout. */
	bool bSlotsCreated = false;

	void CreateSlotsFromAnchors();

	/**
	 * Tính tọa độ tâm của AnchorWidget trong không gian local của SlotsCanvas.
	 * Phải gọi sau khi layout đã hoàn tất (NativeTick frame đầu tiên).
	 */
	FVector2D GetAnchorPosition(UWidget* AnchorWidget) const;
};