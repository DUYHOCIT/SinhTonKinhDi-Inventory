// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InformationWidget.generated.h"

class UCanvasPanel;
class UBorder;
class UTextBlock;

UCLASS()
class SINHTONKINHDI_API UInformationWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(VisibleAnywhere, meta = (BindWidget), Category = "UI")
	UBorder* BackgroundBorder;

	UPROPERTY(VisibleAnywhere, meta = (BindWidget), Category = "UI")
	UTextBlock* Name;

	UPROPERTY(VisibleAnywhere, meta = (BindWidget), Category = "UI")
	UTextBlock* Description;

	void NativeConstruct() override;

public:
	/** Cập nhật tên + mô tả item hiển thị trên widget. */
	void setInformation(const FText& NameText, const FText& DescriptionText);
};