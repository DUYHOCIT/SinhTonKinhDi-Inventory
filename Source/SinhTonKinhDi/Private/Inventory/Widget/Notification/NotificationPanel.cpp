// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/Widget/Notification/NotificationPanel.h"
#include "Inventory/Widget/Notification/NotificationWidget.h"
#include <Kismet/GameplayStatics.h>
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"


void UNotificationPanel::AddNotification(const FText& InText)
{
	APlayerController* GetPlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	UUserWidget* Widget = CreateWidget(GetWorld(), NotificationWidgetClass);
	Widget->SetOwningPlayer(GetPlayerController);
	NotificationWidget = Cast<UNotificationWidget>(Widget);
	if (NotificationWidget)
	{
		NotificationWidget->getText()->SetText(InText);
		if (NotificationBox->GetChildrenCount() >= MaxNotifications)
		{
			NotificationBox->RemoveChildAt(0);
		}
		NotificationBox->AddChild(NotificationWidget);
	}
}

