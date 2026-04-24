// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "NotificationWidget.generated.h"

/**
 * 
 */
class UWidgetAnimation;
class UTextBlock;
struct FTimerHandle;
UCLASS()
class SINHTONKINHDI_API UNotificationWidget : public UUserWidget
{
	GENERATED_BODY()
	
	UPROPERTY(meta = (BindWidgetAnim), Transient)
    UWidgetAnimation* ThongBaoAnimation;
	UPROPERTY(meta = (BindWidget)) UTextBlock* Text;

protected:
	virtual void NativeConstruct() override;
	UPROPERTY(EditAnywhere) float ThoiGianHienThi = 2.0f;
	UPROPERTY(EditAnywhere) float TocDoAnimation = 1.0f;
	FTimerHandle TimerHandle;
public: 
	UTextBlock* getText();
	void ChayNguoc();
	void DestroyWidget();
};
