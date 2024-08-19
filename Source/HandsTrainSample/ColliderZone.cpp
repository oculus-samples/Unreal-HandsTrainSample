/*
Copyright (c) Meta Platforms, Inc. and affiliates.
All rights reserved.

This source code is licensed under the license found in the
LICENSE file in the root directory of this source tree.
*/

#include "ColliderZone.h"
#include "Interactable.h"

/** Should be overridden. */
EInteractableCollisionDepth UColliderZone::GetCollisionDepth_Implementation() const
{
	return EInteractableCollisionDepth::None;
}
