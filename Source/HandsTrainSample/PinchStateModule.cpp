/*
Copyright (c) Meta Platforms, Inc. and affiliates.
All rights reserved.

This source code is licensed under the license found in the
LICENSE file in the root directory of this source tree.
*/

#include "PinchStateModule.h"
#include "Interactable.h"
#include "OculusXRHandComponent.h"
#include "OculusXRInputFunctionLibrary.h"
#include <cmath>

const float PinchStateModule::PinchStrengthThreshold = 1.0f;

PinchStateModule::PinchStateModule()
{
	CurrentPinchState = EPinchState::None;
	FirstFocusedInteractable = nullptr;
}

PinchStateModule::~PinchStateModule()
{
}

bool PinchStateModule::GetPinchUpAndDownOnFocusedObject()
{
	return CurrentPinchState == EPinchState::PinchUp && IsValid(FirstFocusedInteractable);
}

bool PinchStateModule::PinchSteadyOnFocusedObject()
{
	return CurrentPinchState == EPinchState::PinchStay && IsValid(FirstFocusedInteractable);
}

bool PinchStateModule::PinchDownOnFocusedObject()
{
	return CurrentPinchState == EPinchState::PinchDown && IsValid(FirstFocusedInteractable);
}

bool PinchStateModule::UpdateState(float PinchStrength,
	AInteractable* CurrentFocusedInteractable, bool IsRightHanded)
{
	bool IsPinching = fabs(PinchStrengthThreshold - PinchStrength)
		< 0.0001f;
	auto OldPinchState = CurrentPinchState;

	switch (OldPinchState)
	{
		case EPinchState::PinchUp:
			/**
			 * Can only be in pinch for a single frame.
			 *  So consider the next frame carefully. */
			if (IsPinching)
			{
				CurrentPinchState = EPinchState::PinchDown;
				if (CurrentFocusedInteractable != FirstFocusedInteractable)
				{
					FirstFocusedInteractable = nullptr;
				}
			}
			else
			{
				CurrentPinchState = EPinchState::None;
				FirstFocusedInteractable = nullptr;
			}
			break;
		case EPinchState::PinchStay:
			if (!IsPinching)
			{
				CurrentPinchState = EPinchState::PinchUp;
			}
			// If object is not focused anymore, then forget it.
			if (CurrentFocusedInteractable != FirstFocusedInteractable)
			{
				FirstFocusedInteractable = nullptr;
			}
			break;
		// Pinch down lasts for a max of 1 frame; either go to pinch stay or up
		case EPinchState::PinchDown:
			CurrentPinchState = IsPinching ? EPinchState::PinchStay : EPinchState::PinchUp;
			// If the focused interactable changes, then original is now invalid.
			if (FirstFocusedInteractable != CurrentFocusedInteractable)
			{
				FirstFocusedInteractable = nullptr;
			}
			break;
		default:
			if (IsPinching)
			{
				CurrentPinchState = EPinchState::PinchDown;
				/**
				 * This is the interactable that must be focused
				 * throughout the pinch up and down gesture. */
				FirstFocusedInteractable = CurrentFocusedInteractable;
			}
			break;
	}

	return true;
}
