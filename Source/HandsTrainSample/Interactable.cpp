// Fill out your copyright notice in the Description page of Project Settings.

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
