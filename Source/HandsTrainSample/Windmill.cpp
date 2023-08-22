// Fill out your copyright notice in the Description page of Project Settings.

#include "Windmill.h"
#include "CollidableInteractable.h"
#include "SelectionCylinderHelper.h"
#include "InteractableTool.h"
#include <cmath>

AWindmill::AWindmill()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	RootSceneComponent = CreateDefaultSubobject<USceneComponent>(FName(TEXT("Root")));
	RootComponent = RootSceneComponent;

	SelectionCylinderHelper =
		CreateDefaultSubobject<USelectionCylinderHelper>(FName(TEXT("SelectionCylinderHelper")));

	Base = CreateDefaultSubobject<UStaticMeshComponent>(FName(TEXT("Base")));
	Base->SetupAttachment(RootComponent);

	CollidableInteractAnchor = CreateDefaultSubobject<USceneComponent>(
		FName(TEXT("CollidableInteractAnchor")));
	CollidableInteractAnchor->SetupAttachment(RootSceneComponent);
	CollidableInteractAnchor->SetRelativeLocation(FVector(0.0f, 0.0f, 21.0f));

	SelectionMesh = CreateDefaultSubobject<UStaticMeshComponent>(FName(TEXT("SelectionMesh")));
	SelectionMesh->SetupAttachment(Base);

	BladesMesh = CreateDefaultSubobject<UStaticMeshComponent>(FName(TEXT("BladesMesh")));
	BladesMesh->SetupAttachment(Base);

	MaxBladesSpeed = 300.0f;
	CurrentSpeed = 0.0f;
	IsMoving = false;
	OriginalRotation = BladesMesh->GetRelativeRotation().Quaternion();
}

void AWindmill::BeginPlay()
{
	Super::BeginPlay();

	CollidableLogic = GetWorld()->SpawnActor<ACollidableInteractable>(
		CollidableInteractableBP,
		CollidableInteractAnchor->GetComponentLocation(),
		CollidableInteractAnchor->GetComponentRotation());
	CollidableLogic->AttachToActor(this,
		FAttachmentTransformRules::KeepWorldTransform);

	CollidableLogic->OnInteractableStateChanged.AddDynamic(this,
		&AWindmill::InteractableStateChanged);

	SelectionCylinderHelper->Initialize(SelectionMesh);
}

void AWindmill::InteractableStateChanged(const FInteractableStateArgs& StateArgs)
{
	bool bInActionState = StateArgs.NewInteractableState == EInteractableState::ActionState;
	if (bInActionState)
	{
		NewMoveState(!IsMoving);
	}

	ToolInteractingWithMe = StateArgs.NewInteractableState > EInteractableState::Default ? StateArgs.Tool : nullptr;
}

void AWindmill::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	RotationAngle += CurrentSpeed * DeltaTime;
	RotationAngle = fmod(RotationAngle, 360.0f);
	BladesMesh->SetRelativeRotation(OriginalRotation * FRotator(0, RotationAngle, 0).Quaternion());

	if (ToolInteractingWithMe == nullptr)
	{
		SelectionCylinderHelper->SetSelectionState(ESelectionState::Off);
	}
	else
	{
		SelectionCylinderHelper->SetSelectionState(
			(ToolInteractingWithMe->GetCurrInputState() == EToolInputState::PrimaryInputDown || ToolInteractingWithMe->GetCurrInputState() == EToolInputState::PrimaryInputDownStay)
				? ESelectionState::Highlighted
				: ESelectionState::Selected);
	}
}
