// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Containers/Map.h"
#include "InteractableEnums.h"
#include "InteractableTool.generated.h"

class UColliderZone;
class AInteractable;
class UOculusXRHandComponent;

UENUM(BlueprintType)
enum class EToolInputState : uint8
{
	Inactive,
	PrimaryInputDown,
	PrimaryInputDownStay,
	PrimaryInputUp,
};

/**
 * Describes tool-to-collision information.
 */
USTRUCT(BlueprintType)
struct FInteractableCollisionInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	UColliderZone* InteractableCollider;
	UPROPERTY(BlueprintReadOnly)
	EInteractableCollisionDepth CollisionDepth;
	UPROPERTY(BlueprintReadOnly)
	AInteractableTool* CollidingTool;

	FInteractableCollisionInfo(UColliderZone* InteractableCollider,
		EInteractableCollisionDepth CollisionDepth,
		AInteractableTool* CollidingTool)
		: InteractableCollider(InteractableCollider), CollisionDepth(CollisionDepth), CollidingTool(CollidingTool)
	{
	}

	FInteractableCollisionInfo()
		: InteractableCollider(nullptr), CollisionDepth(EInteractableCollisionDepth::None), CollidingTool(nullptr)
	{
	}
};

USTRUCT(BlueprintType)
struct FCollisionInfoKeyValuePair
{
	GENERATED_BODY()

	UPROPERTY()
	AInteractable* Interactable;
	UPROPERTY()
	FInteractableCollisionInfo CollisionInfo;

	FCollisionInfoKeyValuePair(AInteractable* Interactable,
		FInteractableCollisionInfo CollisionInfo)
		: Interactable(Interactable), CollisionInfo(CollisionInfo)
	{
	}

	FCollisionInfoKeyValuePair()
		: Interactable(nullptr)
	{
	}
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FInteractableToolDeathEventSignature,
	AInteractableTool*, InteractableTool);

/**
 * Base class for all interactable tools.
 * Not meant to be used directly. It is must be inherited from.
 */
UCLASS()
class HANDSTRAINSAMPLE_API AInteractableTool : public AActor
{
	GENERATED_BODY()

public:
	AInteractableTool();

	virtual ~AInteractableTool()
	{
	}

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tool Properties")
	bool IsFarFieldTool;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tool Properties")
	bool IsRightHandedTool;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tool Properties")
	TArray<FInteractableCollisionInfo> CurrentIntersectingObjects;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FInteractableToolDeathEventSignature OnInteractableToolDeathEvent;

	UFUNCTION(BlueprintPure, Category = "Movement")
	FVector GetInteractionPosition() const
	{
		return InteractionPosition;
	}

	UFUNCTION(BlueprintPure, Category = "Movement")
	FVector GetToolVelocity() const
	{
		return CalculatedToolVelocity;
	}

	UFUNCTION(BlueprintPure, Category = "Movement")
	FTransform GetToolTransform() const
	{
		return GetActorTransform();
	}

	/**
	 * The tools input router is meant to initialize each tool
	 * manually.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Initialization")
	void Initialize(UOculusXRHandComponent* HandComponent);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	EToolInputState GetCurrInputState();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	void SetVisualEnableState(bool NewEnableState);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	bool GetVisualEnableState();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	void RefreshCurrentIntersectingObjects();

	/**
	 * Used to tell the tool to "focus" on a specific object, if
	 * focusing is indeed given the tool in question. This can
	 * allow far-field tools to focus on an interactable.
	 * @param FocusedInteractable - Interactable to focus.
	 * @param ColliderZone - Collider zone of interactable.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	void FocusOnInteractable(AInteractable* FocusedInteractable,
		UColliderZone* ColliderZone);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	void DeFocus();

	/** Properties of tool -- ray, poke, etc. */
	UFUNCTION(BlueprintNativeEvent, BlueprintPure, Meta = (Bitmask),
		Category = "Interaction")
	EInteractableToolTags GetToolTags();

	/**
	 * We need to find out which collision is closest to a certain position.
	 * Useful if a far field tool like a ray is case and we want to find
	 * the first hit.
	 */
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	FCollisionInfoKeyValuePair GetFirstCurrentCollisionInfoClosestToPosition(
		FVector WorldPosition);

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void ClearAllCurrentCollisionInfos()
	{
		CurrInteractableToCollisionInfos.Empty();
	}

	/**
	 * For each intersecting interactable, update meta data to indicate
	 * deepest collision available.
	 */
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void UpdateCurrentCollisionsMapBasedOnDepth();

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void UpdateCurrentCollisionsMap(AInteractable* CurrInteractable,
		FInteractableCollisionInfo CollisionInfo);

	/**
	 * If our collision information changed per frame, make note of it.
	 * Removed, added and remaining objects must get their proper events.
	 * A new object encountered is considered "added," for instance.
	 */
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void SyncLatestCollisionDataWithInteractables();

	virtual void BeginDestroy() override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tool Properties")
	TMap<AInteractable*, FInteractableCollisionInfo> CurrInteractableToCollisionInfos;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tool Properties")
	TMap<AInteractable*, FInteractableCollisionInfo> PrevInteractableToCollisionInfos;

	FVector InteractionPosition;
	FVector CalculatedToolVelocity;

	/** 
	 * These arrays are created now so they don't need to be
	 * constructed per use. Inherited classes tend to use
	 * these arrays so putting them in the base class
	 * cleans up duplicate code.
	 */
	TArray<AInteractable*> AddedInteractables;
	TArray<AInteractable*> RemovedInteractables;
	TArray<AInteractable*> RemainingInteractables;
};
