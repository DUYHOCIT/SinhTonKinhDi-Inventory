// Copyright Epic Games, Inc. All Rights Reserved.

#include "SinhTonKinhDiCharacter.h"
#include "Engine/LocalPlayer.h"
#include "CoreMinimal.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Inventory/Component/InventoryComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/UserWidget.h"
#include "Inventory/Widget/InventoryGirdWidget.h"
#include "Inventory/Widget/InventoryWidget.h"
#include "DrawDebugHelpers.h"
#include "Inventory/Item/ItemBase.h"
#include "Inventory/Widget/Notification/NotificationPanel.h"
#include "TimerManager.h"
#include "Inventory/Widget/InformationWidget.h"
#include "Components/ChildActorComponent.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

ASinhTonKinhDiCharacter::ASinhTonKinhDiCharacter()
{
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(GetMesh(), TEXT("head"));
	FollowCamera->bUsePawnControlRotation = true;

	LineTraceDistance = 500.f;
	InventoryComponent = CreateDefaultSubobject<UInventoryComponent>(TEXT("InventoryComponent"));

	WeaponChild = CreateDefaultSubobject<UChildActorComponent>(TEXT("WeaponChild"));
	WeaponChild->SetupAttachment(GetMesh(), TEXT("socket_hand_r"));
}

void ASinhTonKinhDiCharacter::BeginPlay()
{
	Super::BeginPlay();
	InventoryComponent->WeaponChild = WeaponChild;
	GetPlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (GetPlayerController)
	{
		if (InventoryWidgetClass)
		{
			InventoryWidget = CreateWidget(GetWorld(), InventoryWidgetClass);
			InventoryWidget->SetOwningPlayer(GetPlayerController);
			InventoryWidget->AddToViewport();
			InventoryWidget->SetVisibility(ESlateVisibility::Collapsed);
			InventoryComponent->InformationWidgetRef = Cast<UInventoryWidget>(InventoryWidget)->InformationWidgetRef;
			InventoryComponent->InformationWidgetRef->SetVisibility(ESlateVisibility::Collapsed);
		}
		if (NotificationWidgetClass)
		{
			NotificationWidget = Cast<UNotificationPanel>(
				CreateWidget(GetWorld(), NotificationWidgetClass)
			);
			if (NotificationWidget)
			{
				NotificationWidget->SetOwningPlayer(GetPlayerController);
				NotificationWidget->AddToViewport();
				NotificationWidget->SetVisibility(ESlateVisibility::Visible);
			}
		}
	}

	GetWorldTimerManager().SetTimer(
		TraceHandle,
		this,
		&ASinhTonKinhDiCharacter::LineTraceForItems,
		0.1f,
		true
	);
}

void ASinhTonKinhDiCharacter::LineTraceForItems()
{
	FVector Start = FollowCamera->GetComponentLocation();
	FVector End = Start + FollowCamera->GetForwardVector() * LineTraceDistance;

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
	{
		if (AItemBase* HitItem = Cast<AItemBase>(Hit.GetActor()))
		{
			if (ItemTemp != HitItem)
			{
				if (ItemTemp) ItemTemp->Mesh->SetRenderCustomDepth(false);
				ItemTemp = HitItem;
				ItemTemp->Mesh->SetRenderCustomDepth(true);
			}
		}
		else
		{
			if (ItemTemp)
			{
				ItemTemp->Mesh->SetRenderCustomDepth(false);
				ItemTemp = nullptr;
			}
		}
	}
	else
	{
		if (IsValid(ItemTemp))
		{
			ItemTemp->Mesh->SetRenderCustomDepth(false);
			ItemTemp = nullptr;
		}
	}
}

void ASinhTonKinhDiCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(
				PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}

	if (UEnhancedInputComponent* EnhancedInputComponent =
		Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ASinhTonKinhDiCharacter::Move);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ASinhTonKinhDiCharacter::Look);
		EnhancedInputComponent->BindAction(TuongTacVoiKhoDo, ETriggerEvent::Started, this, &ASinhTonKinhDiCharacter::ToggleInventory);
		EnhancedInputComponent->BindAction(PickUp, ETriggerEvent::Started, this, &ASinhTonKinhDiCharacter::PickUpItem);

		if (ScrollToolSlotAction)
		{
			EnhancedInputComponent->BindAction(ScrollToolSlotAction, ETriggerEvent::Triggered,
				this, &ASinhTonKinhDiCharacter::OnScrollToolSlot);
		}
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error,
			TEXT("'%s' Failed to find an Enhanced Input component!"), *GetNameSafe(this));
	}
}

void ASinhTonKinhDiCharacter::Move(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();
	if (Controller != nullptr)
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void ASinhTonKinhDiCharacter::Look(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();
	if (Controller != nullptr)
	{
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void ASinhTonKinhDiCharacter::OnScrollToolSlot(const FInputActionValue& Value)
{
	if (!InventoryComponent) return;
	if (InventoryWidget &&
		InventoryWidget->GetVisibility() != ESlateVisibility::Collapsed)
		return;

	const float AxisVal = Value.Get<float>();
	if (FMath::IsNearlyZero(AxisVal)) return;

	const int32 Direction = (AxisVal > 0.f) ? 1 : -1;
	InventoryComponent->ScrollToolSlot(Direction);
}

void ASinhTonKinhDiCharacter::UpdateFocus(float DeltaTime)
{
	if (!FollowCamera) return;

	FVector Start = FollowCamera->GetComponentLocation();
	FVector End = Start + FollowCamera->GetForwardVector() * 10000.0f;
	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	float TargetFocusDistance = GetWorld()->LineTraceSingleByChannel(
		Hit, Start, End, ECC_Visibility, Params)
		? (Hit.Location - Start).Size()
		: 10000.0f;

	FPostProcessSettings& PPS = FollowCamera->PostProcessSettings;

	PPS.bOverride_DepthOfFieldFocalDistance = true;
	PPS.bOverride_DepthOfFieldFstop = true;
	PPS.bOverride_DepthOfFieldMinFstop = true;
	PPS.bOverride_DepthOfFieldFocalRegion = true;
	PPS.bOverride_DepthOfFieldNearTransitionRegion = true;
	PPS.bOverride_DepthOfFieldFarTransitionRegion = true;
	PPS.bOverride_DepthOfFieldDepthBlurRadius = true;
	PPS.bOverride_DepthOfFieldDepthBlurAmount = true;
	PPS.bOverride_DepthOfFieldSensorWidth = true;

	float SmoothFocus = FMath::FInterpTo(
		PPS.DepthOfFieldFocalDistance, TargetFocusDistance, DeltaTime, 6.0f);
	PPS.DepthOfFieldFocalDistance = SmoothFocus;

	const float NearDist = 20.0f;
	const float FarDist = 600.0f;
	float T = FMath::Clamp((SmoothFocus - NearDist) / (FarDist - NearDist), 0.0f, 1.0f);

	float TargetFstop = FMath::Lerp(1.2f, 11.0f, T);
	float FstopSpeed = (TargetFstop > PPS.DepthOfFieldFstop) ? 2.0f : 5.0f;
	PPS.DepthOfFieldFstop = FMath::FInterpTo(PPS.DepthOfFieldFstop, TargetFstop, DeltaTime, FstopSpeed);
	PPS.DepthOfFieldMinFstop = 0.7f;
	PPS.DepthOfFieldSensorWidth = FMath::Lerp(45.0f, 24.0f, T);
	PPS.DepthOfFieldFocalRegion = FMath::Lerp(2.0f, 500.0f, T);
	PPS.DepthOfFieldNearTransitionRegion = FMath::Lerp(5.0f, 150.0f, T);
	PPS.DepthOfFieldFarTransitionRegion = FMath::Lerp(10.0f, 2000.0f, T * T);
	PPS.DepthOfFieldDepthBlurRadius = FMath::Lerp(4.0f, 0.0f, T);
	PPS.DepthOfFieldDepthBlurAmount = FMath::Lerp(1.0f, 0.0f, T);

	FollowCamera->PostProcessBlendWeight = 1.0f;

}

void ASinhTonKinhDiCharacter::ToggleInventory()
{
	if (!InventoryWidget) return;

	if (InventoryWidget->GetVisibility() != ESlateVisibility::Collapsed)
	{
		InventoryWidget->SetVisibility(ESlateVisibility::Collapsed);
		GetPlayerController->SetShowMouseCursor(false);
		GetPlayerController->SetInputMode(FInputModeGameOnly());

		// Xoá selection trên tất cả 3 grid (+ ẩn InformationWidget)
		if (InventoryComponent)
			InventoryComponent->ClearAllGridSelections();

		if (NotificationWidget)
			NotificationWidget->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		InventoryWidget->SetVisibility(ESlateVisibility::Visible);
		GetPlayerController->SetShowMouseCursor(true);
		GetPlayerController->SetInputMode(FInputModeGameAndUI());
		if (NotificationWidget)
			NotificationWidget->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void ASinhTonKinhDiCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	UpdateFocus(DeltaSeconds);
}

void ASinhTonKinhDiCharacter::PickUpItem()
{
	if (!IsValid(ItemTemp) || !InventoryComponent) return;

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AItemBase* InventoryItem = GetWorld()->SpawnActor<AItemBase>(
		ItemTemp->GetClass(),
		FVector::ZeroVector,
		FRotator::ZeroRotator,
		Params
	);
	if (!InventoryItem) return;

	InventoryItem->SetProperties(
		ItemTemp->GetDimensions(),
		ItemTemp->GetIcon(),
		ItemTemp->GetRotatedIcon(),
		ItemTemp->GetName(),
		ItemTemp->GetDescription()
	);
	InventoryItem->SetInInventory(true);

	if (InventoryComponent->TryAddItem(InventoryItem))
	{
		ItemTemp->Destroy();
		ItemTemp = nullptr;
		if (NotificationWidget && NotificationWidget->IsVisible())
		{
			NotificationWidget->AddNotification(
				FText::Format(
					FText::FromString(TEXT("Đã nhặt được {0}")),
					InventoryItem->GetName()
				)
			);
		}
	}
	else
	{
		InventoryItem->Destroy();
		if (NotificationWidget && NotificationWidget->IsVisible())
			NotificationWidget->AddNotification(FText::FromString(TEXT("Kho đồ đã đầy")));
	}
}