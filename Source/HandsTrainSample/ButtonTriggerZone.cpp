// Fill out your copyright notice in the Description page of Project Settings.

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
