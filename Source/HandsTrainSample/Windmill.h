/*
Copyright (c) Meta Platforms, Inc. and affiliates.
All rights reserved.

This source code is licensed under the license found in the
LICENSE file in the root directory of this source tree.
*/

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LatentActions.h"
#include "Interactable.h"
#include "Windmill.generated.h"

class ACollidableInteractable;
class AInteractableTool;

UCLASS()
class HANDSTRAINSAMPLE_API AWindmill : public AActor
{
	GENERATED_BODY()

public:
	AWindmill();

	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Movement")
	void NewMoveState(bool IsMovingNow);

	UFUNCTION()
	void InteractableStateChanged(const FInteractableStateArgs& StateArgs);

protected:
	virtual void BeginPlay() override;

	// lerp speed from start time to end time
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Movement")
	void ChangeSpeed(float SpeedChangeDuration, float TimeLeftForSpeedChange, float StartSpeed, float TargetSpeed);

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Movement")
	float CurrentSpeed;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Movement")
	bool IsMoving;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class USceneComponent* RootSceneComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class UStaticMeshComponent* Base;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class USelectionCylinderHelper* SelectionCylinderHelper;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class UStaticMeshComponent* SelectionMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class UStaticMeshComponent* BladesMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	USceneComponent* CollidableInteractAnchor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxBladesSpeed;

	UPROPERTY(BlueprintReadOnly)
	ACollidableInteractable* CollidableLogic;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<ACollidableInteractable> CollidableInteractableBP;

	UPROPERTY(BlueprintReadOnly)
	AInteractableTool* ToolInteractingWithMe;

private:
	float RotationAngle;
	FQuat OriginalRotation;
};
