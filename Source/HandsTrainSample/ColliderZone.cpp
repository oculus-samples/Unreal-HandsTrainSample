// Fill out your copyright notice in the Description page of Project Settings.

#include "ColliderZone.h"
#include "Interactable.h"

/** Should be overridden. */
EInteractableCollisionDepth UColliderZone::GetCollisionDepth_Implementation() const
{
	return EInteractableCollisionDepth::None;
}
