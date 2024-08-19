/*
Copyright (c) Meta Platforms, Inc. and affiliates.
All rights reserved.

This source code is licensed under the license found in the
LICENSE file in the root directory of this source tree.
*/

#include "CollidableInteractable.h"
#include "ColliderZone.h"
#include "ButtonTriggerZone.h"
#include "InteractableTool.h"
#include "ButtonTriggerZone.h"
#include "Kismet/KismetSystemLibrary.h"

const float ACollidableInteractable::EntryDotThreshold = 0.8f;
const float ACollidableInteractable::PerpDotThreshold = 0.5f;

ACollidableInteractable::ACollidableInteractable()
{
	ProximityZone = CreateDefaultSubobject<UButtonTriggerZone>(
		FName(TEXT("ProximityZone")));
	ProximityZone->SetupAttachment(RootComponent);
	ContactZone = CreateDefaultSubobject<UButtonTriggerZone>(
		FName(TEXT("ContactZone")));
	ContactZone->SetupAttachment(RootComponent);
	ActionZone = CreateDefaultSubobject<UButtonTriggerZone>(
		FName(TEXT("ActionZone")));
	ActionZone->SetupAttachment(RootComponent);

	InteractablePlaneCenter = CreateDefaultSubobject<USceneComponent>(
		FName(TEXT("InteractablePlaneCenter")));
	InteractablePlaneCenter->SetupAttachment(RootComponent);
}

void ACollidableInteractable::BeginPlay()
{
	Super::BeginPlay();

	ProximityZone->ParentInteractable = this;
	ContactZone->ParentInteractable = this;
	ActionZone->ParentInteractable = this;
	CurrentState = EInteractableState::Default;
}

void ACollidableInteractable::UpdateCollisionDepth_Implementation(
	AInteractableTool* InteractableTool, EInteractableCollisionDepth OldCollisionDepth,
	EInteractableCollisionDepth NewCollisionDepth)
{
	bool IsFarFieldTool = InteractableTool->IsFarFieldTool;

	// If this is a near field tool and another tool already controls it, bail
	if (!IsFarFieldTool && ToolToState.Num() > 0 && !ToolToState.Contains(InteractableTool))
	{
		return;
	}

	auto OldState = CurrentState;

	// Ignore contact test if you are using the far field tool
	auto MyTransform = GetActorTransform();
	auto CurrPressDirection = MyTransform.TransformVector(LocalPressDirection);

	bool ValidContact = IsValidContact(InteractableTool, CurrPressDirection) || InteractableTool->IsFarFieldTool;
	// In case tool enters contact zone first, we are in proximity as well
	bool ToolIsInProximity = NewCollisionDepth >= EInteractableCollisionDepth::Proximity;
	bool ToolIsInContactZone = NewCollisionDepth == EInteractableCollisionDepth::Contact;
	bool ToolIsInActionZone = NewCollisionDepth == EInteractableCollisionDepth::Action;

	bool SwitchingStates = OldCollisionDepth != NewCollisionDepth;
	if (SwitchingStates)
	{
		FireInteractionEventsEventsOnDepth(OldCollisionDepth,
			InteractableTool, ECollisionInteractionType::Exit);
		FireInteractionEventsEventsOnDepth(NewCollisionDepth,
			InteractableTool, ECollisionInteractionType::Enter);
	}
	else
	{
		FireInteractionEventsEventsOnDepth(NewCollisionDepth,
			InteractableTool, ECollisionInteractionType::Stay);
	}

	auto UpcomingState = OldState;
	if (IsFarFieldTool)
	{
		UpcomingState = ToolIsInContactZone ? EInteractableState::ContactState : ToolIsInActionZone ? EInteractableState::ActionState
																									: EInteractableState::Default;
	}
	else
	{
		// Use plane describing positive side of interactable to filter collisions
		FPlane InteractableZonePlane(InteractablePlaneCenter->GetComponentLocation(), -CurrPressDirection);
		// Skip plane test if boolean flag tells us to ignore it
		float DotProdPlane = InteractableZonePlane.PlaneDot(InteractableTool->GetInteractionPosition());
		bool OnPositiveSideOfInteractable = !MakeSureInteractingToolIsOnPositiveSide || DotProdPlane > 0.0f;
		UpcomingState = GetUpcomingStateNearField(OldState, NewCollisionDepth,
			ToolIsInActionZone, ToolIsInContactZone, ToolIsInProximity,
			ValidContact, OnPositiveSideOfInteractable);
	}

	if (UpcomingState != EInteractableState::Default)
	{
		if (ToolToState.Contains(InteractableTool))
		{
			ToolToState[InteractableTool] = UpcomingState;
		}
		else
		{
			ToolToState.Add(InteractableTool, UpcomingState);
		}
	}
	else
	{
		ToolToState.Remove(InteractableTool);
	}

	// Far field set max state set so far. This can happen
	// when multiple far field tools interact with object at same time;
	// you don't want another one to lower the interactable's state.
	if (IsFarFieldTool)
	{
		for (auto ToolStatePair : ToolToState)
		{
			auto ToolState = ToolStatePair.Value;
			if (UpcomingState < ToolState)
			{
				UpcomingState = ToolState;
			}
		}
	}

	if (OldState != UpcomingState)
	{
		CurrentState = UpcomingState;
		auto InteractionType = !SwitchingStates ? ECollisionInteractionType::Stay : NewCollisionDepth == EInteractableCollisionDepth::None ? ECollisionInteractionType::Exit
																																		   : ECollisionInteractionType::Enter;
		auto CurrentCollider = CurrentState == EInteractableState::ProximityState ? ProximityZone : CurrentState == EInteractableState::ContactState ? ContactZone
			: CurrentState == EInteractableState::ActionState																						 ? ActionZone
																																					 : nullptr;
		OnInteractableStateChanged.Broadcast(
			FInteractableStateArgs(this, InteractableTool, OldState, CurrentState,
				FColliderZoneArgs(CurrentCollider, UKismetSystemLibrary::GetFrameCount(),
					InteractableTool, InteractionType)));
	}
}

EInteractableState ACollidableInteractable::GetUpcomingStateNearField(EInteractableState OldState,
	EInteractableCollisionDepth NewCollisionDepth,
	bool ToolIsInActionZone, bool ToolIsInContactZone,
	bool ToolIsInProximity, bool ValidContact, bool OnPositiveSideOfButton)
{
	EInteractableState UpcomingState = OldState;

	switch (OldState)
	{
		case EInteractableState::ActionState:
			if (!ToolIsInActionZone)
			{
				// If retreating from action, can go back into action
				// state even if contact is not legal (i.e. tool/finger retracts)
				if (ToolIsInContactZone)
				{
					UpcomingState = EInteractableState::ContactState;
				}
				else if (ToolIsInProximity)
				{
					UpcomingState = EInteractableState::ProximityState;
				}
				else
				{
					UpcomingState = EInteractableState::Default;
				}
			}
			break;
		case EInteractableState::ContactState:
			if (NewCollisionDepth < EInteractableCollisionDepth::Contact)
			{
				UpcomingState = ToolIsInProximity ? EInteractableState::ProximityState
												  : EInteractableState::Default;
			}
			/**
			 * Can only go to action state if contact is legal.
			 * If tool goes into contact state due to proper movement,
			 * but does not maintain that movement throughout (i.e. a tool/finger
			 * presses downwards initially but moves in random directions
			 * afterwards), then don't go into action state
			 */
			else if (ToolIsInActionZone && ValidContact && OnPositiveSideOfButton)
			{
				UpcomingState = EInteractableState::ActionState;
			}
			break;
		case EInteractableState::ProximityState:
			if (NewCollisionDepth < EInteractableCollisionDepth::Proximity)
			{
				UpcomingState = EInteractableState::Default;
			}
			else if (ValidContact && OnPositiveSideOfButton && NewCollisionDepth > EInteractableCollisionDepth::Proximity)
			{
				UpcomingState = NewCollisionDepth == EInteractableCollisionDepth::Action
					? EInteractableState::ActionState
					: EInteractableState::ContactState;
			}
			break;
		case EInteractableState::Default:
			// Test contact, action first then proximity (more important
			// states take precedence)
			if (ValidContact && OnPositiveSideOfButton && NewCollisionDepth > EInteractableCollisionDepth::Proximity)
			{
				UpcomingState = NewCollisionDepth == EInteractableCollisionDepth::Action
					? EInteractableState::ActionState
					: EInteractableState::ContactState;
			}
			else if (ToolIsInProximity)
			{
				UpcomingState = EInteractableState::ProximityState;
			}
			break;
	}

	return UpcomingState;
}

void ACollidableInteractable::FireInteractionEventsEventsOnDepth(EInteractableCollisionDepth CurrentDepth,
	AInteractableTool* CollidingTool, ECollisionInteractionType InteractionType)
{
	switch (CurrentDepth)
	{
		case EInteractableCollisionDepth::Proximity:
			OnProximityZoneEvent.Broadcast(FColliderZoneArgs(ProximityZone,
				UKismetSystemLibrary::GetFrameCount(), CollidingTool, InteractionType));
			break;
		case EInteractableCollisionDepth::Contact:
			OnContactZoneEvent.Broadcast(FColliderZoneArgs(ContactZone,
				UKismetSystemLibrary::GetFrameCount(), CollidingTool, InteractionType));
			break;
		case EInteractableCollisionDepth::Action:
			OnActionZoneEvent.Broadcast(FColliderZoneArgs(ActionZone,
				UKismetSystemLibrary::GetFrameCount(), CollidingTool, InteractionType));
			break;
	}
}

bool ACollidableInteractable::IsValidContact(AInteractableTool* CollidingTool, FVector EntryDirection)
{
	if (ContactTests.Num() == 0 || CollidingTool->IsFarFieldTool)
	{
		return true;
	}

	for (auto contactTest : ContactTests)
	{
		switch (contactTest)
		{
			case EContactTest::BackwardsPress:
				if (!PassEntryTest(CollidingTool, EntryDirection))
				{
					return false;
				}

				break;
			default:
				if (!PassPerpTest(CollidingTool, EntryDirection))
				{
					return false;
				}

				break;
		}
	}

	return true;
}

bool ACollidableInteractable::PassEntryTest(AInteractableTool* CollidingTool, FVector EntryDirection)
{
	FVector ToolVelocityVector = CollidingTool->GetToolVelocity().GetSafeNormal();
	float dotProduct = FVector::DotProduct(ToolVelocityVector, EntryDirection);
	return dotProduct > ACollidableInteractable::EntryDotThreshold;
}

bool ACollidableInteractable::PassPerpTest(AInteractableTool* CollidingTool, FVector EntryDirection)
{
	FVector ToolDirection = CollidingTool->GetActorForwardVector();
	float dotProduct = FVector::DotProduct(ToolDirection, EntryDirection);
	return dotProduct > ACollidableInteractable::PerpDotThreshold;
}
