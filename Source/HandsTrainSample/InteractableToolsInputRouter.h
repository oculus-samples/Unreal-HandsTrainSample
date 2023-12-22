// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

class AInteractableTool;
class UOculusXRHandComponent;
class UMotionControllerComponent;

/**
 * Routes all collisions from interactable tools to the interactables.
 * We want to do collisions in a top-down fashion since in certain
 * scenarios near-field interactiosn would take precedence over
 * far-field interactions. If the tools managed collisions on their
 * this design would be more difficult to implement.
 */
class HANDSTRAINSAMPLE_API InteractableToolsInputRouter
{
public:
	InteractableToolsInputRouter();
	~InteractableToolsInputRouter();

	void UpdateTools(UOculusXRHandComponent* LeftHand,
		UOculusXRHandComponent* RightHand,
		const TSet<AInteractableTool*>& LeftHandNearTools,
		const TSet<AInteractableTool*>& LeftHandFarTools,
		const TSet<AInteractableTool*>& RightHandNearTools,
		const TSet<AInteractableTool*>& RightHandFarTools);

private:
	void UpdateToolsForHand(UOculusXRHandComponent* Hand,
		const TSet<AInteractableTool*>& HandNearTools,
		const TSet<AInteractableTool*>& HandFarTools);

	bool UpdateToolsAndGetEncounteredObjects(const TSet<AInteractableTool*>& Tools,
		bool ToolsAreVisuallyEnabledThisFrame);

	/**
	 * Update tools specified based on new collisions.
	 * @param Tools - tools to update.
	 * @param ResetCollisionData - whether a tool's collision state
	 * should be reset or not.
	 */
	bool UpdateTools(const TSet<AInteractableTool*>& Tools,
		bool ResetCollisionData);
	void ToggleToolsVisualEnableState(const TSet<AInteractableTool*>& Tools,
		bool VisualEnableState);
};
