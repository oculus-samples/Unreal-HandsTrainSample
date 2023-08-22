// Fill out your copyright notice in the Description page of Project Settings.

#include "HandsActiveChecker.h"
#include "Components/StaticMeshComponent.h"
#include "OculusXRInputFunctionLibrary.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "MotionControllerComponent.h"

AHandsActiveChecker::AHandsActiveChecker()
{
	PrimaryActorTick.bCanEverTick = true;

	RootSceneComponent = CreateDefaultSubobject<USceneComponent>(FName(TEXT("Root")));
	RootComponent = RootSceneComponent;

	NotificationMesh = CreateDefaultSubobject<UStaticMeshComponent>(
		FName(TEXT("NotificationMesh")));
	NotificationMesh->SetupAttachment(RootComponent);
	NotificationMesh->SetMobility(EComponentMobility::Movable);
	NotificationMesh->SetVisibility(false);
}

void AHandsActiveChecker::BeginPlay()
{
	Super::BeginPlay();
	FindPawnAndControllers();
}

void AHandsActiveChecker::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!IsValid(PlayerPawn) || !IsValid(LeftMesh) || !IsValid(RightMesh))
	{
		return;
	}

	if (UOculusXRInputFunctionLibrary::IsHandTrackingEnabled())
	{
		if (NotificationMesh->IsVisible())
		{
			NotificationMesh->SetVisibility(false);
		}
		if (IsValid(LeftMesh) && LeftMesh->IsVisible())
		{
			LeftMesh->SetVisibility(false);
		}
		if (IsValid(RightMesh) && RightMesh->IsVisible())
		{
			RightMesh->SetVisibility(false);
		}
	}
	else
	{
		if (!NotificationMesh->IsVisible())
		{
			NotificationMesh->SetVisibility(true);
		}

		if (IsValid(LeftMesh) && !LeftMesh->IsVisible())
		{
			LeftMesh->SetVisibility(true);
		}
		if (IsValid(RightMesh) && !RightMesh->IsVisible())
		{
			RightMesh->SetVisibility(true);
		}

		FRotator DeviceRotation;
		FVector DevicePosition;
		UHeadMountedDisplayFunctionLibrary::GetOrientationAndPosition(DeviceRotation, DevicePosition);
		if (IsValid(PlayerPawn))
		{
			FTransform PlayerTransform = PlayerPawn->GetTransform();
			FVector HMDPosition = PlayerTransform.TransformPosition(DevicePosition);
			FVector NotifPosition = PlayerTransform.TransformPosition(DevicePosition
				+ FVector(50.0f, 0.0f, 0.0f));
			FQuat TransformedQuat = PlayerTransform.TransformRotation(DeviceRotation.Quaternion());
			FVector LookDirection = PlayerPawn->GetActorLocation() - HMDPosition;

			SetActorLocation(NotifPosition);
			SetActorRotation(UKismetMathLibrary::MakeRotFromXZ(
				-TransformedQuat.GetForwardVector(),
				TransformedQuat.GetUpVector())
								 .Quaternion());
		}
	}
}

void AHandsActiveChecker::FindPawnAndControllers()
{
	PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);

	if (IsValid(PlayerPawn) && (LeftMesh == nullptr || RightMesh == nullptr))
	{
		LeftMesh =
			(UStaticMeshComponent*)PlayerPawn->GetDefaultSubobjectByName(
				FName(TEXT("LeftControllerMesh")));
		RightMesh =
			(UStaticMeshComponent*)PlayerPawn->GetDefaultSubobjectByName(
				FName(TEXT("RightControllerMesh")));
	}
}
