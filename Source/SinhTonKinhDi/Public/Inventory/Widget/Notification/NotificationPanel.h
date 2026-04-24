// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "NotificationPanel.generated.h"

/**
 * 
 */
class UVerticalBox;
class UNotificationWidget;
UCLASS()
class SINHTONKINHDI_API UNotificationPanel : public UUserWidget
{
	GENERATED_BODY()
protected:

	UPROPERTY(meta = (BindWidget)) UVerticalBox* NotificationBox;
	UPROPERTY(EditAnywhere) TSubclassOf<UUserWidget> NotificationWidgetClass;
	UPROPERTY(EditAnywhere) uint16 MaxNotifications = 6;
public: 
	UFUNCTION(BlueprintCallable)
	void AddNotification(const FText& InText);
	UNotificationWidget* NotificationWidget;

};
