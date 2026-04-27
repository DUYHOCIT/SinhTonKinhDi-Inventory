// Fill out your copyright notice in the Description page of Project Settings.
#include "Inventory/Widget/InventoryWidget.h"
#include "Blueprint/DragDropOperation.h"
#include "SinhTonKinhDi/SinhTonKinhDiCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Inventory/Item/ItemBase.h"
#include "Inventory/Widget/InventoryGirdWidget.h"
#include "Inventory/Widget/ItemWidget.h"
#include "Inventory/Widget/ToolInventoryWidget.h"
#include "Inventory/Component/InventoryComponent.h"
#include "Inventory/Widget/InformationWidget.h"
#include "Components/Button.h"

void UInventoryWidget::NativeConstruct()
{
	Super::NativeConstruct();

	CharacterReference = Cast<ASinhTonKinhDiCharacter>(
		UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));

	if (Tab_Equipment) Tab_Equipment->OnClicked.AddDynamic(this, &UInventoryWidget::OnTabEquipment);
	if (Tab_Tool)      Tab_Tool->OnClicked.AddDynamic(this, &UInventoryWidget::OnTabTool);
	if (Tab_Food)      Tab_Food->OnClicked.AddDynamic(this, &UInventoryWidget::OnTabFood);
	if (Tab_Other)     Tab_Other->OnClicked.AddDynamic(this, &UInventoryWidget::OnTabOther);

	// Mặc định mở tab Trang bị
	OnTabEquipment();
}

// ─────────────────────────────────────────────────────────────────
//  Show / Hide logic
// ─────────────────────────────────────────────────────────────────

void UInventoryWidget::ShowGridForCategory(EItemCategory Cat)
{
	// Ẩn hết
	if (Grid_Equipment) Grid_Equipment->SetVisibility(ESlateVisibility::Collapsed);
	if (Grid_Food)      Grid_Food->SetVisibility(ESlateVisibility::Collapsed);
	if (Grid_Other)     Grid_Other->SetVisibility(ESlateVisibility::Collapsed);
	if (ToolBelt)       ToolBelt->SetVisibility(ESlateVisibility::Collapsed);

	// Hiện đúng grid
	UInventoryGirdWidget* Target = nullptr;
	switch (Cat)
	{
	case EItemCategory::Food:  Target = Grid_Food;      break;
	case EItemCategory::Other: Target = Grid_Other;     break;
	default:                   Target = Grid_Equipment; break; // Equipment
	}

	if (Target)
	{
		Target->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		Target->Refresh(); // cập nhật kho của category này
	}
}

void UInventoryWidget::ShowToolBelt()
{
	if (Grid_Equipment) Grid_Equipment->SetVisibility(ESlateVisibility::Collapsed);
	if (Grid_Food)      Grid_Food->SetVisibility(ESlateVisibility::Collapsed);
	if (Grid_Other)     Grid_Other->SetVisibility(ESlateVisibility::Collapsed);
	if (ToolBelt)       ToolBelt->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
}

// ─────────────────────────────────────────────────────────────────
//  Tab handlers
// ─────────────────────────────────────────────────────────────────

void UInventoryWidget::OnTabEquipment()
{
	InformationWidgetRef->SetVisibility(ESlateVisibility::Collapsed);
	CurrentTab = EItemCategory::Equipment;
	ShowGridForCategory(EItemCategory::Equipment);
	UpdateTabHighlight(Tab_Equipment);
}

void UInventoryWidget::OnTabTool()
{
	InformationWidgetRef->SetVisibility(ESlateVisibility::Collapsed);
	CurrentTab = EItemCategory::Tool;
	ShowToolBelt();
	UpdateTabHighlight(Tab_Tool);
}

void UInventoryWidget::OnTabFood()
{
	InformationWidgetRef->SetVisibility(ESlateVisibility::Collapsed);
	CurrentTab = EItemCategory::Food;
	ShowGridForCategory(EItemCategory::Food);
	UpdateTabHighlight(Tab_Food);
}

void UInventoryWidget::OnTabOther()
{
	InformationWidgetRef->SetVisibility(ESlateVisibility::Collapsed);
	CurrentTab = EItemCategory::Other;
	ShowGridForCategory(EItemCategory::Other);
	UpdateTabHighlight(Tab_Other);
}

void UInventoryWidget::UpdateTabHighlight(UButton* ActiveBtn)
{
	TArray<UButton*> AllTabs = { Tab_Equipment, Tab_Tool, Tab_Food, Tab_Other };
	for (UButton* Btn : AllTabs)
	{
		if (!Btn) continue;
		FButtonStyle Style = Btn->GetStyle();
		FSlateColor  Color = (Btn == ActiveBtn) ? ActiveTabColor : InactiveTabColor;
		Style.Normal.TintColor = Color;
		Style.Hovered.TintColor = Color;
		Style.Pressed.TintColor = Color;
		Btn->SetStyle(Style);
	}
}

// ─────────────────────────────────────────────────────────────────
//  Drop ra ngoài → spawn trước mặt nhân vật
// ─────────────────────────────────────────────────────────────────

bool UInventoryWidget::NativeOnDrop(const FGeometry& InGeometry,
	const FDragDropEvent& InDragDropEvent,
	UDragDropOperation* InOperation)
{
	if (!InOperation || !InOperation->Payload || !CharacterReference)
		return false;

	AItemBase* Item = Cast<AItemBase>(InOperation->Payload);
	if (!Item) return false;

	// Xoá selection trên grid tương ứng
	UInventoryComponent* Comp = CharacterReference->InventoryComponent;
	if (Comp)
	{
		auto ClearSelectionOnGrid = [&](UInventoryGirdWidget* Grid)
			{
				if (!Grid) return;
				UItemWidget* ItemWidget = Cast<UItemWidget>(InOperation->DefaultDragVisual);
				if (ItemWidget && ItemWidget == Grid->ItemSelected)
					Grid->SetItemSelected(nullptr);
			};
		ClearSelectionOnGrid(Comp->GridWidgetRef_Equipment);
		ClearSelectionOnGrid(Comp->GridWidgetRef_Food);
		ClearSelectionOnGrid(Comp->GridWidgetRef_Other);
	}

	const FVector  SpawnLoc = CharacterReference->GetActorLocation()
		+ CharacterReference->GetActorForwardVector() * 200.f;
	const FRotator SpawnRot = CharacterReference->GetActorRotation();

	Item->SetInInventory(false);
	Item->SetActorLocationAndRotation(SpawnLoc, SpawnRot);

	if (Comp)
	{
		const bool bFromToolSlot = (Comp->OriginalToolSlotIndex != -1);

		Comp->OriginalDragStartIndex = -1;
		Comp->OriginalToolSlotIndex = -1;
		Comp->DraggedItem_Internal = nullptr;

		if (bFromToolSlot && Comp->ToolWidgetRef)
			Comp->ToolWidgetRef->Refresh();

		// Refresh grid đúng category của item vừa thả
		switch (Item->ItemCategory)
		{
		case EItemCategory::Food:
			if (Comp->GridWidgetRef_Food)  Comp->GridWidgetRef_Food->Refresh();
			break;
		case EItemCategory::Other:
			if (Comp->GridWidgetRef_Other) Comp->GridWidgetRef_Other->Refresh();
			break;
		default:
			if (Comp->GridWidgetRef_Equipment) Comp->GridWidgetRef_Equipment->Refresh();
			break;
		}
	}

	return true;
}