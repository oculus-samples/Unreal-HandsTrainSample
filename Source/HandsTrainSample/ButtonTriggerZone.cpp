/*
Copyright (c) Meta Platforms, Inc. and affiliates.
All rights reserved.

This source code is licensed under the license found in the
LICENSE file in the root directory of this source tree.
*/

#include "ButtonTriggerZone.h"
#include "Interactable.h"

EInteractableCollisionDepth UButtonTriggerZone::GetCollisionDepth_Implementation() const
{
	auto depth =
		ParentInteractable->ProximityZone == this ? EInteractableCollisionDepth::Proximity : ParentInteractable->ContactZone == this ? EInteractableCollisionDepth::Contact
		: ParentInteractable->ActionZone == this																					 ? EInteractableCollisionDepth::Action
																																	 : EInteractableCollisionDepth::None;
	return depth;
}
