// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InteractableTool.h"
#include "PinchStateModule.h"
#include "RayTool.generated.h"

class AInteractable;
class UOculusXRHandComponent;
class USplineMeshComponent;
class URayToolViewHelper;

/**
 * Ray tool used for far field interactions with
 * objects of type AInteractable.
 */
UCLASS()
class HANDSTRAINSAMPLE_API ARayTool : public AInteractableTool
{
	GENERATED_BODY()
public:
	ARayTool();

	virtual void Tick(float DeltaTime) override;

	virtual void Initialize_Implementation(UOculusXRHandComponent* HandComponent)
		override;

	virtual void SetVisualEnableState_Implementation(bool NewVisualEnableState) override;

	virtual bool GetVisualEnableState_Implementation() override;

	virtual EToolInputState GetCurrInputState_Implementation() override;

	virtual void RefreshCurrentIntersectingObjects_Implementation() override;

	virtual EInteractableToolTags GetToolTags_Implementation() override;

	virtual void FocusOnInteractable_Implementation(
		AInteractable* NewFocusedInteractable, UColliderZone* ColliderZone) override;

	virtual void DeFocus_Implementation() override;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Meshes")
	USceneComponent* RootSceneComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Meshes")
	UStaticMeshComponent* TargetMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Meshes")
	UInstancedStaticMeshComponent* RayMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Meshes")
	URayToolViewHelper* RayToolViewHelperComp;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ranges",
		meta = (ToolTip = "Cast angle from ray's center", UIMin = "0.0",
			UIMax = "45.0"))
	float ConeAngleDegrees;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ranges")
	float FarFieldMaxDistance;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	AInteractable* FocusedInteractable;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	AInteractable* CurrInteractableRaycastedAgainst;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UOculusXRHandComponent* Hand;

	/**
	 * Avoid hand collider during raycasts so we need to cast from some distance
	 * away from where tool is.
	 * @return Proper raycast origin.
	 */
	UFUNCTION(BlueprintPure)
	FVector GetRaycastOrigin();

	UFUNCTION(BlueprintCallable)
	bool HasRayReleasedInteractable(AInteractable* NewFocusedInteractable);

	/**
	 * Find all objects from primary ray or if that fails, all objects in a
	 * cone around main ray direction via "secondary" cast.
	 * @return Target interactable, if any.
	 */
	UFUNCTION(BlueprintCallable)
	AInteractable* FindTargetInteractable();

	/**
	 * Find first hit that supports our tool's method of interaction.
	 */
	UFUNCTION(BlueprintCallable)
	AInteractable* FindPrimaryRaycastHit(FVector RayOrigin, FVector RayDirection);

	/**
	 * If the primary cast fails, try a secondary test to see if can
	 * target an interactable. This target has to be far enough and
	 * support our tool via appropriate usage tags, and must be within
	 * a certain angle from our primary ray direction.
	 * @param RayOrigin - Primary ray origin.
	 * @param RayDirection - Primary ray direction.
	 * @return Interactable found, if any.
	 */
	UFUNCTION(BlueprintCallable)
	AInteractable* FindInteractableViaConeTest(FVector RayOrigin, FVector RayDirection);

private:
	const static float MinimumRaycastDistance;
	const static float ColliderRadius;
	const static uint32 NumMaxPrimaryHits = 10;
	const static uint32 NumMaxSecondaryHits = 25;
	const static uint32 NumCollidersToTest = 20;

	bool bIsInitialized;
	float ConeAngleReleaseDegrees;
	PinchStateModule CurrPinchState;
};
