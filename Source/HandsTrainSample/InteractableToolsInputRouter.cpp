/*
Copyright (c) Meta Platforms, Inc. and affiliates.
All rights reserved.

This source code is licensed under the license found in the
LICENSE file in the root directory of this source tree.
*/

#include "InteractableToolsInputRouter.h"
#include "InteractableTool.h"
#include "Interactable.h"
#include "OculusXRHandComponent.h"
#include "MotionControllerComponent.h"
#include "OculusXRInputFunctionLibrary.h"

InteractableToolsInputRouter::InteractableToolsInputRouter()
{
}

InteractableToolsInputRouter::~InteractableToolsInputRouter()
{
}

void InteractableToolsInputRouter::UpdateTools(UOculusXRHandComponent* LeftHand,
	UOculusXRHandComponent* RightHand, const TSet<AInteractableTool*>& LeftHandNearTools,
	const TSet<AInteractableTool*>& LeftHandFarTools,
	const TSet<AInteractableTool*>& RightHandNearTools,
	const TSet<AInteractableTool*>& RightHandFarTools)
{
	UpdateToolsForHand(LeftHand, LeftHandNearTools, LeftHandFarTools);
	UpdateToolsForHand(RightHand, RightHandNearTools, RightHandFarTools);
}

void InteractableToolsInputRouter::UpdateToolsForHand(
	UOculusXRHandComponent* Hand,
	const TSet<AInteractableTool*>& HandNearTools,
	const TSet<AInteractableTool*>& HandFarTools)
{
	if (!IsValid(Hand))
	{
		return;
	}

	bool HandIsReliable = UOculusXRInputFunctionLibrary::GetTrackingConfidence(
							  Hand->SkeletonType, 0)
			== EOculusXRTrackingConfidence::High
		&& UOculusXRInputFunctionLibrary::IsHandTrackingEnabled();

	bool EncounteredNearObjectsHand = UpdateToolsAndGetEncounteredObjects(
		HandNearTools, HandIsReliable);

	bool PointerPoseIsValid = UOculusXRInputFunctionLibrary::IsPointerPoseValid(
		Hand->SkeletonType);
	/**
	 * Enable far field if near objects were not encountered, hand
	 * tracking is reliable and pointer pose is valid.
	 */

	UpdateToolsAndGetEncounteredObjects(HandFarTools,
		!EncounteredNearObjectsHand && HandIsReliable && PointerPoseIsValid);
}

bool InteractableToolsInputRouter::UpdateToolsAndGetEncounteredObjects(
	const TSet<AInteractableTool*>& Tools,
	bool ToolsAreVisuallyEnabledThisFrame)
{
	bool ResetCollisionData = !ToolsAreVisuallyEnabledThisFrame;
	bool EncounteredObjects = UpdateTools(Tools, ResetCollisionData);
	ToggleToolsVisualEnableState(Tools, ToolsAreVisuallyEnabledThisFrame);
	return EncounteredObjects;
}

bool InteractableToolsInputRouter::UpdateTools(const TSet<AInteractableTool*>& Tools,
	bool ResetCollisionData)
{
	bool AnyToolEncounteredObjects = false;

	for (auto CurrentInteractableTool : Tools)
	{
		CurrentInteractableTool->RefreshCurrentIntersectingObjects();
		const TArray<FInteractableCollisionInfo>& CurrIntersectingObjects =
			CurrentInteractableTool->CurrentIntersectingObjects;

		auto CurrToolHasIntersectingObjects = CurrIntersectingObjects.Num() > 0;
		// Step one: update interactables focus states, etc.
		if (CurrToolHasIntersectingObjects && !ResetCollisionData)
		{
			if (!AnyToolEncounteredObjects)
			{
				AnyToolEncounteredObjects = CurrToolHasIntersectingObjects;
			}
			CurrentInteractableTool->UpdateCurrentCollisionsMapBasedOnDepth();

			if (CurrentInteractableTool->IsFarFieldTool)
			{
				auto FirstInteractable = CurrentInteractableTool->GetFirstCurrentCollisionInfoClosestToPosition(
					CurrentInteractableTool->GetActorLocation());

				/**
				 * If our tool is activated, make sure depth is "action".
				 * This means that far field tools will make an interactable
				 * go directly into the action state.
				 */
				if (CurrentInteractableTool->GetCurrInputState() == EToolInputState::PrimaryInputUp)
				{
					FirstInteractable.CollisionInfo.InteractableCollider = FirstInteractable.Interactable->ActionZone;
					FirstInteractable.CollisionInfo.CollisionDepth =
						EInteractableCollisionDepth::Action;
				}
				else
				{
					FirstInteractable.CollisionInfo.InteractableCollider = FirstInteractable.Interactable->ContactZone;
					FirstInteractable.CollisionInfo.CollisionDepth =
						EInteractableCollisionDepth::Contact;
				}
				// update map
				CurrentInteractableTool->UpdateCurrentCollisionsMap(FirstInteractable.Interactable,
					FirstInteractable.CollisionInfo);

				// far field tools can only focus elements; pick first (for now)
				CurrentInteractableTool->FocusOnInteractable(
					FirstInteractable.Interactable,
					FirstInteractable.CollisionInfo.InteractableCollider);
			}
		}
		else
		{
			// If something was focused before, defocus it now.
			CurrentInteractableTool->DeFocus();
			CurrentInteractableTool->ClearAllCurrentCollisionInfos();
		}

		// Step two: sync tool with latest interactable states.
		CurrentInteractableTool->SyncLatestCollisionDataWithInteractables();
	}

	return AnyToolEncounteredObjects;
}

void InteractableToolsInputRouter::ToggleToolsVisualEnableState(
	const TSet<AInteractableTool*>& Tools,
	bool VisualEnableState)
{
	for (auto Tool : Tools)
	{
		if (Tool->GetVisualEnableState() != VisualEnableState)
		{
			Tool->SetVisualEnableState(VisualEnableState);
		}
	}
}
