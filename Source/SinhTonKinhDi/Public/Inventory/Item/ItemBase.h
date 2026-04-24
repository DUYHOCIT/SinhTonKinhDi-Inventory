// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ItemBase.generated.h"

class UStaticMeshComponent;
class USphereComponent;
class UMaterialInterface;

UCLASS()
class SINHTONKINHDI_API AItemBase : public AActor
{
	GENERATED_BODY()

public:
	AItemBase();

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UStaticMeshComponent* Mesh;

	// --- Dữ liệu item ---
	FIntPoint           GetDimensions() const; // trả về đã swap nếu xoay
	UMaterialInterface* GetIcon()        const { return Icon; }
	UMaterialInterface* GetRotatedIcon() const { return RotatedIcon; }
	bool                GetIsRotated()   const { return bIsRotated; }
	void                RotateItem();

	/** Gọi sau khi SpawnActor để copy dữ liệu từ item ngoài thế giới */
	void SetProperties(FIntPoint NewDimensions,
		UMaterialInterface* NewIcon,
		UMaterialInterface* NewRotatedIcon,
		FText Name,
		FText Description);

	/**
	 * Ẩn/hiện actor trong thế giới.
	 * Gọi khi đưa vào inventory (bHide=true) hoặc thả ra (bHide=false).
	 */
	void SetInInventory(bool bHide);
	FText GetName();

	FText GetDescription() const { return Description; }
protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, Category = "Item|Dimensions")
	FIntPoint Dimensions = FIntPoint(1, 1);

	UPROPERTY(EditAnywhere, Category = "Item|Icon")
	UMaterialInterface* Icon = nullptr;

	UPROPERTY(EditAnywhere, Category = "Item|Icon")
	UMaterialInterface* RotatedIcon = nullptr;

	UPROPERTY(EditAnywhere, Category = "Infomation")
	FText Name;

	UPROPERTY(EditAnywhere, Category = "Infomation")
	FText Description;


private:
	bool bIsRotated = false;
};