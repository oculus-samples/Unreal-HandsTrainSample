// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "InteractableEnums.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Interactable.generated.h"

class UColliderZone;
class AInteractableTool;

/** Useful for collision tests against proximity, contact and action zones. */
USTRUCT(BlueprintType)
struct FColliderZoneArgs
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	UColliderZone* Zone;
	UPROPERTY(BlueprintReadOnly)
	int64 CollisionFrame;
	UPROPERTY(BlueprintReadOnly)
	AInteractableTool* CollidingTool;
	UPROPERTY(BlueprintReadOnly)
	ECollisionInteractionType InteractionT;

	FColliderZoneArgs(UColliderZone* Zone, int64 CollisionFrame,
		AInteractableTool* CollidingTool, ECollisionInteractionType InteractionT)
		: Zone(Zone), CollisionFrame(CollisionFrame), CollidingTool(CollidingTool), InteractionT(InteractionT)
	{
	}

	FColliderZoneArgs()
		: Zone(nullptr), CollisionFrame(UKismetSystemLibrary::GetFrameCount()), CollidingTool(nullptr), InteractionT(ECollisionInteractionType::Enter)
	{
	}
};

/** 
 * Used when interactable change state, which means a tool is involved
 * in its state change. */
USTRUCT(BlueprintType)
struct FInteractableStateArgs
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadonly)
	AInteractable* Interactable;
	UPROPERTY(BlueprintReadonly)
	class AInteractableTool* Tool;
	UPROPERTY(BlueprintReadonly)
	EInteractableState OldInteractableState;
	UPROPERTY(BlueprintReadonly)
	EInteractableState NewInteractableState;
	UPROPERTY(BlueprintReadonly)
	FColliderZoneArgs ColliderArgs;

	FInteractableStateArgs(AInteractable* Interactable, AInteractableTool* Tool,
		EInteractableState OldInteractableState, EInteractableState NewInteractableState,
		FColliderZoneArgs ColliderArgs)
		: Interactable(Interactable), Tool(Tool), OldInteractableState(OldInteractableState), NewInteractableState(NewInteractableState), ColliderArgs(ColliderArgs)
	{
	}

	FInteractableStateArgs()
		: Interactable(nullptr), Tool(nullptr), OldInteractableState(EInteractableState::Default), NewInteractableState(EInteractableState::Default)
	{
	}
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FZoneEventSignature,
	const struct FColliderZoneArgs&, ColliderArgs);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FInteractableStateEventSignature,
	const struct FInteractableStateArgs&, InteractableStateArgs);

/**
 * Base class for all interactables.
 * Not meant to be used directly! It is must be inherited from.
 */
UCLASS()
class HANDSTRAINSAMPLE_API AInteractable : public AActor
{
	GENERATED_BODY()

public:
	AInteractable();

	virtual ~AInteractable()
	{
	}

	// Create zones on inheriting class so that the subclass type can be picked.
	/** Collider that indicates if we are close to interactable or not. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tools / Collisions")
	UColliderZone* ProximityZone;

	/** Collider that indicates if we are making contact -- i.e. touching. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tools / Collisions")
	UColliderZone* ContactZone;

	/** Use action collider when something, like a button, activates. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tools / Collisions")
	UColliderZone* ActionZone;

	/** 
	 * While these events will tell you if a tool has entered an interactable's
	 * specific zone, the interactable might not go into the associated state.
	 * If a finger is pressing a button incorrectly even though it's in
	 * the contact zone, we should not be in the contact state. */

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FZoneEventSignature OnProximityZoneEvent;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FZoneEventSignature OnContactZoneEvent;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FZoneEventSignature OnActionZoneEvent;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FInteractableStateEventSignature OnInteractableStateChanged;

	/** Used to call overridable function */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Tools / Collisions")
	void UpdateCollisionDepth(AInteractableTool* InteractableTool,
		EInteractableCollisionDepth OldCollisionDepth, EInteractableCollisionDepth NewCollisionDepth);

	UFUNCTION(BlueprintCallable, Category = "Tools / Collisions")
	int GetValidToolTagsMask() const
	{
		return AllValidToolTagsMask;
	}

protected:
	/** Tools that interactable can interact with are filtered via tags. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (Bitmask), Category = "Tools / Collisions")
	TArray<EInteractableToolTags> AllValidToolTags;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Meshes")
	USceneComponent* RootSceneComponent;

	virtual void BeginPlay() override;

	uint32 AllValidToolTagsMask;
};
