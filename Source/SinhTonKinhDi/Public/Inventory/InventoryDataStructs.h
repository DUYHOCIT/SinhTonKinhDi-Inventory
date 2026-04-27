// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
#include "InventoryDataStructs.generated.h"

USTRUCT()
struct Flines
{
	GENERATED_USTRUCT_BODY();
	Flines() {}
	TArray<FVector2D> XLines;
	TArray<FVector2D> YLines;
};

USTRUCT()
struct FMousePositionInTile
{
	GENERATED_USTRUCT_BODY();
	FMousePositionInTile() : Right(false), Down(false) {}
	bool Right;
	bool Down;
};

UENUM(BlueprintType)
enum class EItemCategory : uint8
{
	// "Tất cả" tab bị xoá → thay bằng "Trang bị"
	Equipment UMETA(DisplayName = "Trang bị"),
	Tool      UMETA(DisplayName = "Công cụ"),
	Food      UMETA(DisplayName = "Đồ ăn"),
	Other     UMETA(DisplayName = "Đồ khác")
};

class SINHTONKINHDI_API InventoryDataStructs
{
public:
	InventoryDataStructs();
	~InventoryDataStructs();
};