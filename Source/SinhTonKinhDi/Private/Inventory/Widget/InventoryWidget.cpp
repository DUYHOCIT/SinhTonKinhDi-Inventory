// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/Widget/InventoryWidget.h"
#include "Blueprint/DragDropOperation.h"
#include "SinhTonKinhDi/SinhTonKinhDiCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Inventory/Item/ItemBase.h"
#include "Inventory/Widget/InventoryGirdWidget.h"
#include "Inventory/Widget/ItemWidget.h"
#include "Inventory/Component/InventoryComponent.h"

void UInventoryWidget::NativeConstruct()
{
	Super::NativeConstruct();
	CharacterReference = Cast<ASinhTonKinhDiCharacter>(
		UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
}

bool UInventoryWidget::NativeOnDrop(const FGeometry& InGeometry,
	const FDragDropEvent& InDragDropEvent,
	UDragDropOperation* InOperation)
{
	// Đến đây nghĩa là GridWidget đã từ chối (trả false)
	// → người chơi thả item ra ngoài inventory → spawn ra world
	if (!InOperation || !InOperation->Payload || !CharacterReference)
		return false;

	AItemBase* Item = Cast<AItemBase>(InOperation->Payload);
	if (!Item) return false;
	UItemWidget* ItemWidget = Cast<UItemWidget>(InOperation->DefaultDragVisual);
	if (ItemWidget == Grid->ItemSelected)
	{
		Grid->SetItemSelected(nullptr);
	}
	// Tính vị trí spawn trước mặt nhân vật
	const FVector    SpawnLoc = CharacterReference->GetActorLocation()
		+ CharacterReference->GetActorForwardVector() * 200.f;
	const FRotator   SpawnRot = CharacterReference->GetActorRotation();

	// Bật lại actor (hiện mesh, bật collision) rồi đặt vị trí
	Item->SetInInventory(false);
	Item->SetActorLocationAndRotation(SpawnLoc, SpawnRot);

	// Xoá trạng thái drag tồn đọng
	if (CharacterReference->InventoryComponent)
	{
		CharacterReference->InventoryComponent->OriginalDragStartIndex = -1;
		CharacterReference->InventoryComponent->DraggedItem_Internal = nullptr;
	}

	return true;
}

