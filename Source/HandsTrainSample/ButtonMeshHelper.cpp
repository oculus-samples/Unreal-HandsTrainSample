// Fill out your copyright notice in the Description page of Project Settings.

#include "ButtonMeshHelper.h"
#include "ColliderZone.h"
#include "InteractableTool.h"
#include "InteractableButton.h"
#include "Math/UnrealMathUtility.h"
#include <Components/StaticMeshComponent.h>

const float UButtonMeshHelper::LerpToOldPositionDuration = 1.0f;
const FName UButtonMeshHelper::TintColorParameterName = FName(TEXT("TintColor"));
const float UButtonMeshHelper::ButtonDepth = 5.0f;

UButtonMeshHelper::UButtonMeshHelper()
{
	PrimaryComponentTick.bCanEverTick = false;
	ButtonContactColor = FColor(214, 169, 53, 255);
	ButtonActionColor = FColor(253, 239, 162, 255);

	ContactMaxDisplacementDistance = 1.365f;
}

void UButtonMeshHelper::Initialize(AInteractableButton* ParentButton, UColliderZone* ButtonContact, UStaticMeshComponent* StaticMeshComponent)
{
	OldRelativePosition = StaticMeshComponent->GetRelativeLocation();

	this->Button = ParentButton;
	this->ButtonContactComponent = ButtonContact;
	this->MeshComponent = StaticMeshComponent;

	ButtonInContactOrActionStates = false;

	ParentButton->OnInteractableStateChanged.AddDynamic(this,
		&UButtonMeshHelper::InteractableStateChanged);
	ParentButton->OnContactZoneEvent.AddDynamic(this,
		&UButtonMeshHelper::ActionOrInContactZoneStayEvent);
	ParentButton->OnActionZoneEvent.AddDynamic(this,
		&UButtonMeshHelper::ActionOrInContactZoneStayEvent);
}

void UButtonMeshHelper::InteractableStateChanged(const FInteractableStateArgs& StateArgs)
{
	ButtonInContactOrActionStates = false;
	Button->ToggleButtonGlow(StateArgs.NewInteractableState > EInteractableState::Default);

	switch (StateArgs.NewInteractableState)
	{
		case EInteractableState::ContactState:
			Button->StopResetLerp();
			MeshComponent->SetVectorParameterValueOnMaterials(
				UButtonMeshHelper::TintColorParameterName,
				FVector(ButtonContactColor));
			ButtonInContactOrActionStates = true;
			break;
		case EInteractableState::ProximityState:
			Button->ResetPositionLerp(LerpToOldPositionDuration, LerpToOldPositionDuration);
			MeshComponent->SetVectorParameterValueOnMaterials(
				UButtonMeshHelper::TintColorParameterName,
				FVector(ButtonDefaultColor));
			break;
		case EInteractableState::ActionState:
			Button->StopResetLerp();
			MeshComponent->SetVectorParameterValueOnMaterials(
				UButtonMeshHelper::TintColorParameterName,
				FVector(ButtonActionColor));
			Button->PlayClickSound();
			ButtonInContactOrActionStates = true;
			break;
		default:
			Button->ResetPositionLerp(LerpToOldPositionDuration, LerpToOldPositionDuration);
			MeshComponent->SetVectorParameterValueOnMaterials(
				UButtonMeshHelper::TintColorParameterName,
				FVector(ButtonDefaultColor));
			break;
	}
}

void UButtonMeshHelper::ActionOrInContactZoneStayEvent(
	const FColliderZoneArgs& ZoneArgs)
{
	// Button movements are governed by near-field interaction, not far-field.
	// (since near field involves touching button)
	if (!ButtonInContactOrActionStates || ZoneArgs.CollidingTool->IsFarFieldTool)
	{
		return;
	}

	/** Calculate how much the button should be pushed inwards based on the contact
	 * zone. The collider's dimension is required for this. Another way to test
	 * distances is to measure distance to the plane that represents where the button
	 * translation must stop. */
	FVector InteractionPosition = ZoneArgs.CollidingTool->GetInteractionPosition();
	const FTransform& ContactZoneTransform = Button->ContactZone->GetComponentTransform();
	FVector PositionInContactZoneLocalSpace = ContactZoneTransform.InverseTransformPosition(
		InteractionPosition);
	/** Calculate how far button press must be. If we are on the button surface,
	 * or depth units away, then no travel is necessary, for instance. The travel
	 * length is calculate based on max button depth. */
	float DepthPositionClamped = FMath::Clamp(PositionInContactZoneLocalSpace.X,
		0.0f, ButtonDepth);
	float ButtonTravel = ButtonDepth - DepthPositionClamped;
	/** Restrict button movement; it can't go too far or go backwards. */
	if (ButtonTravel < UButtonMeshHelper::ContactMaxDisplacementDistance
		&& ButtonTravel >= 0.0f)
	{
		MeshComponent->SetRelativeLocation(
			FVector(OldRelativePosition.X - ButtonTravel,
				OldRelativePosition.Y,
				OldRelativePosition.Z));
	}
}
