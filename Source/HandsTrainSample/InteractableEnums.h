// Fill out your copyright notice in the Description page of Project Settings.

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
