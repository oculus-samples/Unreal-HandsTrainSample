/*
Copyright (c) Meta Platforms, Inc. and affiliates.
All rights reserved.

This source code is licensed under the license found in the
LICENSE file in the root directory of this source tree.
*/

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "RayToolViewHelper.generated.h"

class AInteractable;
class AInteractableTool;

/**
 * Modifies visual state of ray tool. Not quite the same as the Unity's
 * line renderer but it works.
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class HANDSTRAINSAMPLE_API URayToolViewHelper : public UActorComponent
{
	GENERATED_BODY()

public:
	URayToolViewHelper();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category = "Initialization")
	void Initialize(AInteractableTool* Tool, UStaticMeshComponent* NewTargetMesh,
		UInstancedStaticMeshComponent* NewRayMesh);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "ViewState")
	bool GetEnableState();

	UFUNCTION(BlueprintCallable, Category = "ViewState")
	void SetEnableState(bool bIsEnabled);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "ViewState")
	bool GetToolActiveState();

	UFUNCTION(BlueprintCallable, Category = "ViewState")
	void SetToolActiveState(bool bNewActiveState);

	UFUNCTION(BlueprintCallable, Category = "ViewState")
	void SetFocusedInteractable(AInteractable* NewFocusedInteractable);

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ViewState")
	bool bToolActivateState;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Visual")
	FColor NormalColor;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Visual")
	FColor HighlightColor;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Visual")
	AInteractable* FocusedInteractable;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Visual")
	TArray<FTransform> RayMeshSegmentTransforms;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Visual")
	TArray<FVector> RayPositions;

private:
	const static float DefaultRayCastDistance;
	const static FName ColorPropertyName;
	const static FName OpacityPropertName;
	const static uint32 NumRayLinePositions = 25;
	const static float RayScaleFactor;

	AInteractableTool* InteractableTool;
	UStaticMeshComponent* TargetMesh;
	UInstancedStaticMeshComponent* RayMesh;

	void UpdateRayMesh(FVector ToolPosition, FVector ToolForward,
		FVector TargetPosition, float TargetDistance);

	/**
	 * Returns point on four-point Bezier curve.
	 * @param P0 - Beginning point.
	 * @param P1 - t=1/3 point.
	 * @param P2 - t=2/3 point.
	 * @param P3 - End point.
	 * @param T - Interpolation parameter.
	 * @return Point along Bezier curve.
	 */
	FVector GetPointOnBezierCurve(FVector P0, FVector P1, FVector P2,
		FVector P3, float T);
};
