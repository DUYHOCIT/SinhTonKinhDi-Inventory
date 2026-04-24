// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/CanvasPanel.h"

#include "Components/Border.h"
#include "Components/BackgroundBlur.h"

#include "InventoryWidget.generated.h"

class UCanvasPanle;
class UBorder;
class UBackgroundBlur;
class ASinhTonKinhDiCharacter;
class AItemBase;
class UInventoryGirdWidget;

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
	UPROPERTY(VisibleAnywhere, meta = (BindWidget), Category = "UI")
	UInventoryGirdWidget* Grid;
protected:
	ASinhTonKinhDiCharacter* CharacterReference;
	virtual void NativeConstruct() override;
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
};
