/*
Copyright (c) Meta Platforms, Inc. and affiliates.
All rights reserved.

This source code is licensed under the license found in the
LICENSE file in the root directory of this source tree.
*/

#pragma once

#include "CoreMinimal.h"

class AInteractable;
class UOculusXRHandComponent;

/**
 * Manages pinch state, including if an object is being focused
 * via something like a ray (or not).
 */
class HANDSTRAINSAMPLE_API PinchStateModule
{
public:
	PinchStateModule();
	~PinchStateModule();

	/**
	 * We want a pinch up and down gesture to be done **while** an object
	 * is focused. Focus means that a ray strikes an item and "focuses" it.
	 * We don't want someone to pinch, unfocus an object, then
	 * refocus before doing pinch up. We also want to avoid focusing a different
	 * interactable during this process. While the latter is difficult to
	 * do since a person might focus nothing before focus nothing before
	 * focusing on another interactable, it's theoretically possible.
	 */
	bool GetPinchUpAndDownOnFocusedObject();

	bool PinchSteadyOnFocusedObject();

	bool PinchDownOnFocusedObject();

	bool UpdateState(float PinchStrength,
		AInteractable* CurrentFocusedInteractable, bool IsRightHanded);

private:
	enum EPinchState
	{
		None = 0,
		PinchDown,
		PinchStay,
		PinchUp
	};

	const static float PinchStrengthThreshold;

	EPinchState CurrentPinchState;
	AInteractable* FirstFocusedInteractable;
};
