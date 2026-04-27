// Fill out your copyright notice in the Description page of Project Settings.
#include "Inventory/Item/ItemBase.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"

AItemBase::AItemBase()
{
	PrimaryActorTick.bCanEverTick = false;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));

	RootComponent = Mesh;

	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AItemBase::BeginPlay()
{
	Super::BeginPlay();
}

// ─────────────────────────────────────────────────────────────────

FIntPoint AItemBase::GetDimensions() const
{
	return bIsRotated ? FIntPoint(Dimensions.Y, Dimensions.X) : Dimensions;
}

void AItemBase::RotateItem()
{
	bIsRotated = !bIsRotated;
}

void AItemBase::SetProperties(FIntPoint NewDimensions,
	UMaterialInterface* NewIcon,
	UMaterialInterface* NewRotatedIcon,
	FText NewName,
	FText NewDescription)
{
	Dimensions = NewDimensions;
	Icon = NewIcon;
	RotatedIcon = NewRotatedIcon;
	Name = NewName;
	Description = NewDescription;
}

void AItemBase::SetInInventory(bool bHide)
{
	SetActorHiddenInGame(bHide);
	SetActorEnableCollision(!bHide);
}

FText AItemBase::GetName()
{
	return Name;
}

void AItemBase::OnEquipped()
{
	SetActorEnableCollision(false);
	Mesh->SetSimulatePhysics(false);
}
