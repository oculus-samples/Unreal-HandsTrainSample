// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interactable.h"
#include "CollidableInteractable.generated.h"

UENUM(BlueprintType)
enum class EContactTest : uint8
{
	PerpenTest,
	BackwardsPress,
};

/**
 * Basic functionality for (most) collidable interactables.
 */
UCLASS()
class HANDSTRAINSAMPLE_API ACollidableInteractable : public AInteractable
{
	GENERATED_BODY()
public:
	ACollidableInteractable();

	/** For positive side tests, the contact position must be on
	 * the positive side of a plane. Use a scene component to determine
	 * the plane's position. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tools/Collisions")
	USceneComponent* InteractablePlaneCenter;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tools/Collisions")
	TArray<EContactTest> ContactTests;

	/** If the user wants to the press can come from the "positive" side
	 * of an interactable. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tools/Collisions")
	bool MakeSureInteractingToolIsOnPositiveSide;

	/** Depending on the geometry used the interactable might have a special direction. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tools/Collisions")
	FVector LocalPressDirection;

	virtual void UpdateCollisionDepth_Implementation(AInteractableTool* InteractableTool,
		EInteractableCollisionDepth OldCollisionDepth,
		EInteractableCollisionDepth NewCollisionDepth) override;

protected:
	virtual void BeginPlay() override;

	/** Normally this would be private. Make it a UPROPERTY to
	 * prevent dangling references. */
	UPROPERTY()
	TMap<AInteractableTool*, EInteractableState> ToolToState;

private:
	EInteractableState CurrentState;

	EInteractableState GetUpcomingStateNearField(EInteractableState OldState,
		EInteractableCollisionDepth NewCollisionDepth,
		bool ToolIsInActionZone, bool ToolIsInContactZone,
		bool ToolIsInProximity, bool ValidContact, bool OnPositiveSideOfButton);

	/** Can call enter, stay and exit on a specific depth. */
	void FireInteractionEventsEventsOnDepth(EInteractableCollisionDepth CurrentDepth,
		AInteractableTool* CollidingTool, ECollisionInteractionType InteractionType);

	/** If necessary, restrict contacts with button in case a button cannot be pressed
	 * incorrectly. */
	bool IsValidContact(AInteractableTool* CollidingTool, FVector EntryDirection);

	bool PassEntryTest(AInteractableTool* CollidingTool, FVector EntryDirection);
	bool PassPerpTest(AInteractableTool* CollidingTool, FVector EntryDirection);

	const static float EntryDotThreshold;
	const static float PerpDotThreshold;
};
