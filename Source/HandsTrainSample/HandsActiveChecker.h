/*
Copyright (c) Meta Platforms, Inc. and affiliates.
All rights reserved.

This source code is licensed under the license found in the
LICENSE file in the root directory of this source tree.
*/

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HandsActiveChecker.generated.h"

class UMotionControllerComponent;

UCLASS()
class HANDSTRAINSAMPLE_API AHandsActiveChecker : public AActor
{
	GENERATED_BODY()

public:
	AHandsActiveChecker();

	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Meshes")
	USceneComponent* RootSceneComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Meshes")
	UStaticMeshComponent* NotificationMesh;

	UPROPERTY()
	APawn* PlayerPawn;

	UPROPERTY()
	UStaticMeshComponent* LeftMesh;

	UPROPERTY()
	UStaticMeshComponent* RightMesh;

private:
	void FindPawnAndControllers();
};
