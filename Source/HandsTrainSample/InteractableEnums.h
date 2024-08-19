/*
Copyright (c) Meta Platforms, Inc. and affiliates.
All rights reserved.

This source code is licensed under the license found in the
LICENSE file in the root directory of this source tree.
*/

#pragma once

#include "InteractableEnums.generated.h"

UENUM(BlueprintType)
enum class EInteractableCollisionDepth : uint8
{
	None,
	Proximity,
	Contact,
	Action,
};

UENUM(BlueprintType)
enum class ECollisionInteractionType : uint8
{
	Enter,
	Stay,
	Exit,
};

UENUM(BlueprintType)
enum class EInteractableState : uint8
{
	Default,
	ProximityState,
	ContactState,
	ActionState,
};

UENUM(BlueprintType)
enum class EInteractableToolTags : uint8
{
	None = 0,
	Ray = 1 << 0,
	Poke = 1 << 2,
	All = 255,
};
