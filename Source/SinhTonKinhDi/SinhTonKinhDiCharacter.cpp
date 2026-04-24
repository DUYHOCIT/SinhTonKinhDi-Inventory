// Copyright Epic Games, Inc. All Rights Reserved.

#include "SinhTonKinhDiCharacter.h"
#include "Engine/LocalPlayer.h"
#include "CoreMinimal.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
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

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

ASinhTonKinhDiCharacter::ASinhTonKinhDiCharacter()
{
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	LineTraceDistance = 500.f;

	InventoryComponent = CreateDefaultSubobject<UInventoryComponent>(TEXT("InventoryComponent"));
}

void ASinhTonKinhDiCharacter::BeginPlay()
{
	Super::BeginPlay();

	GetPlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (GetPlayerController)
	{
		if (InventoryWidgetClass)
		{
			InventoryWidget = CreateWidget(GetWorld(), InventoryWidgetClass);
			InventoryWidget->SetOwningPlayer(GetPlayerController);
			InventoryWidget->AddToViewport();
			InventoryWidget->SetVisibility(ESlateVisibility::Collapsed);
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
	FVector Start;
	FRotator Rotation;
	GetActorEyesViewPoint(Start, Rotation);
	FVector End = Start + FollowCamera->GetForwardVector() * LineTraceDistance;

	FHitResult Hit;

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	if (GetWorld()->LineTraceSingleByChannel(
		Hit,
		Start,
		End,
		ECC_Visibility,
		Params
	)) {
		if (AItemBase* HitItem = Cast<AItemBase>(Hit.GetActor()))
		{
			if (ItemTemp != HitItem)
			{
				if (ItemTemp)
				{
					ItemTemp->Mesh->SetRenderCustomDepth(false);
				}

				ItemTemp = HitItem;
				ItemTemp->Mesh->SetRenderCustomDepth(true);
			}
			DrawDebugLine(
				GetWorld(),
				Start,
				End,
				FColor::Cyan,
				false,
				0.1f,
				0,
				2.0f
			);
		}
		else
		{
			if (ItemTemp)
			{
				ItemTemp->Mesh->SetRenderCustomDepth(false);
				ItemTemp = nullptr;
			}
			DrawDebugLine(
				GetWorld(),
				Start,
				End,
				FColor::Red,
				false,
				0.1f,
				0,
				2.0f
			);
		}

	}
	else
	{
		if (IsValid(ItemTemp))
		{
			ItemTemp->Mesh->SetRenderCustomDepth(false);
			ItemTemp = nullptr;
		}
		DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, 0.1f, 0, 2.0f);
	}
}

void ASinhTonKinhDiCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ASinhTonKinhDiCharacter::Move);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ASinhTonKinhDiCharacter::Look);
		EnhancedInputComponent->BindAction(TuongTacVoiKhoDo, ETriggerEvent::Started, this, &ASinhTonKinhDiCharacter::ToggleInventory);
		EnhancedInputComponent->BindAction(PickUp, ETriggerEvent::Started, this, &ASinhTonKinhDiCharacter::PickUpItem);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input component!"), *GetNameSafe(this));
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

void ASinhTonKinhDiCharacter::ToggleInventory()
{
	if (!InventoryWidget) return;

	if (InventoryWidget->GetVisibility() != ESlateVisibility::Collapsed)
	{
		InventoryWidget->SetVisibility(ESlateVisibility::Collapsed);
		GetPlayerController->SetShowMouseCursor(false);
		GetPlayerController->SetInputMode(FInputModeGameOnly());
		InventoryComponent->GridWidgetRef->SetNullItemSelected();
		if(NotificationWidget)
		{
			NotificationWidget->SetVisibility(ESlateVisibility::Visible);
		}
	}
	else
	{
		InventoryWidget->SetVisibility(ESlateVisibility::Visible);
		GetPlayerController->SetShowMouseCursor(true);
		GetPlayerController->SetInputMode(FInputModeGameAndUI());
		if (NotificationWidget)
		{
			NotificationWidget->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
}

void ASinhTonKinhDiCharacter::PickUpItem()
{
	if (IsValid(ItemTemp))
	{
		if (!ItemTemp || !InventoryComponent) return;

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
			{
				NotificationWidget->AddNotification(FText::FromString(TEXT("Kho đồ đã đầy")));

			}
		}
	}
}
