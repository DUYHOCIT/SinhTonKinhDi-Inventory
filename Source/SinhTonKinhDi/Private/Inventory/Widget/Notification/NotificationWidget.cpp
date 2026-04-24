// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/Widget/Notification/NotificationWidget.h"
#include "TimerManager.h"

void UNotificationWidget::NativeConstruct()
{
	Super::NativeConstruct();
	if (ThongBaoAnimation)
	{
		PlayAnimation(ThongBaoAnimation, 0, 1, EUMGSequencePlayMode::Forward, TocDoAnimation, false);
	}

	GetWorld()->GetTimerManager().SetTimer(
		TimerHandle,
		this,
		&UNotificationWidget::ChayNguoc,
		ThoiGianHienThi,
		false
	);
}


UTextBlock* UNotificationWidget::getText()
{
	return Text;
}

void UNotificationWidget::ChayNguoc()
{
	if (ThongBaoAnimation)
	{
		PlayAnimation(ThongBaoAnimation, 0, 1, EUMGSequencePlayMode::Reverse, TocDoAnimation, false);
	}
	GetWorld()->GetTimerManager().SetTimer(
		TimerHandle,
		this,
		&UNotificationWidget::DestroyWidget,
		(TocDoAnimation != 0) ? 0.5f / TocDoAnimation : 0.5f,
		false
	);
}

void UNotificationWidget::DestroyWidget()
{
	RemoveFromParent();
}
