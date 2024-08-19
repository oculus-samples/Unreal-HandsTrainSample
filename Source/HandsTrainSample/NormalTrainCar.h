/*
Copyright (c) Meta Platforms, Inc. and affiliates.
All rights reserved.

This source code is licensed under the license found in the
LICENSE file in the root directory of this source tree.
*/

#pragma once

#include "CoreMinimal.h"
#include "TrainCarBase.h"
#include "NormalTrainCar.generated.h"

/**
 * A train car that is driven by the locomotive.
 */
UCLASS()
class HANDSTRAINSAMPLE_API ANormalTrainCar : public ATrainCarBase
{
	GENERATED_BODY()
public:
	ANormalTrainCar();

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Meshes)
	class ATrainCarBase* ParentLocomotive;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Meshes)
	float DistanceBehindParent;

	virtual void UpdateState(float DeltaTime) override;
};
