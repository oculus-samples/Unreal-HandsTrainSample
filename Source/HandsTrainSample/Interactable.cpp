/*
Copyright (c) Meta Platforms, Inc. and affiliates.
All rights reserved.

This source code is licensed under the license found in the
LICENSE file in the root directory of this source tree.
*/

#include "Interactable.h"
#include "ColliderZone.h"
#include "InteractableTool.h"

AInteractable::AInteractable()
{
	PrimaryActorTick.bCanEverTick = false;

	RootSceneComponent = CreateDefaultSubobject<USceneComponent>(FName(TEXT("Root")));
	RootComponent = RootSceneComponent;
}

void AInteractable::BeginPlay()
{
	Super::BeginPlay();
	AllValidToolTagsMask = 0;
	for (auto validToolTag : AllValidToolTags)
	{
		AllValidToolTagsMask |= (int)validToolTag;
	}
}

void AInteractable::UpdateCollisionDepth_Implementation(
	AInteractableTool* InteractableTool, EInteractableCollisionDepth OldCollisionDepth,
	EInteractableCollisionDepth CollisionDepth)
{
	// meant to be overridden
}
