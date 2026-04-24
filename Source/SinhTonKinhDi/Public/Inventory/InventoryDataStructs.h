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
	// ✅ Fix C26495: khởi tạo giá trị mặc định
	FMousePositionInTile() : Right(false), Down(false) {}
	bool Right;
	bool Down;
};

class SINHTONKINHDI_API InventoryDataStructs
{
public:
	InventoryDataStructs();
	~InventoryDataStructs();
};
