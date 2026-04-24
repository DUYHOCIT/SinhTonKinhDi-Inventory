// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/Widget/InformationWidget.h"
#include "Components/TextBlock.h"
#include <SinhTonKinhDi/SinhTonKinhDiCharacter.h>
#include "Inventory/Component/InventoryComponent.h"

void UInformationWidget::NativeConstruct()
{
	Super::NativeConstruct();
	ASinhTonKinhDiCharacter* CharacterRef = Cast<ASinhTonKinhDiCharacter>(GetOwningPlayerPawn());
	if (!CharacterRef) return;

	if (!CharacterRef->InventoryComponent) return;

	CharacterRef->InventoryComponent->InformationWidgetRef = this;
}

void UInformationWidget::setInformation(const FText& NameText, const FText& DescriptionText)
{
	if (Name) Name->SetText(NameText);
	if (Description) Description->SetText(DescriptionText);
}
