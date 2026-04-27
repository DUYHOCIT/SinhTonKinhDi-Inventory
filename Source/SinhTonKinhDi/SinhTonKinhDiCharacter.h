// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "SinhTonKinhDiCharacter.generated.h"

class UCameraComponent;
class UInputMappingContext;
class UInputAction;
class UInventoryComponent;
class APlayerController;
class UUserWidget;
class AActor;
class AItemBase;
class UInventoryGirdWidget;
class UNotificationPanel;
class UChildActorComponent;
struct FTimerHandle;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

UCLASS(config=Game)
class ASinhTonKinhDiCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Follow camera */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;
	
	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* TuongTacVoiKhoDo;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* PickUp;

	/**
	 * Input Action cho lăn chuột đổi slot tool.
	 * Tạo IA_ScrollTool trong editor: loại Axis1D (float),
	 * mapping: Mouse Wheel Axis (hoặc MouseScrollUp +1 / MouseScrollDown -1).
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* ScrollToolSlotAction;

public:
	ASinhTonKinhDiCharacter();

	UPROPERTY(EditAnywhere)
	UInventoryComponent* InventoryComponent;
	void ToggleInventory();


protected:

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);
			
	void UpdateFocus(float DeltaTime);

	UFUNCTION()
	void PickUpItem();

	/** Lăn chuột → đổi slot tool đang active */
	void OnScrollToolSlot(const FInputActionValue& Value);

	APlayerController* GetPlayerController;

protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	// To add mapping context
	virtual void BeginPlay();

	void Tick(float DeltaSeconds) override;

	UPROPERTY()
	AItemBase* ItemTemp;

	UPROPERTY(EditAnywhere, Category = "LineTrace")
	float LineTraceDistance;

	void LineTraceForItems();
	FTimerHandle TraceHandle;

public:
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<UUserWidget> InventoryWidgetClass;
	UUserWidget* InventoryWidget;

	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<UUserWidget> NotificationWidgetClass;
	UNotificationPanel* NotificationWidget;

	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<UUserWidget> ItemWidgetClass;
	UUserWidget* ItemWidget;

	UPROPERTY(EditAnywhere)
	UChildActorComponent* WeaponChild;
};