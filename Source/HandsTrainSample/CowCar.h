/*
Copyright (c) Meta Platforms, Inc. and affiliates.
All rights reserved.

This source code is licensed under the license found in the
LICENSE file in the root directory of this source tree.
*/

#pragma once

#include "CoreMinimal.h"
#include "NormalTrainCar.h"
#include "CowCar.generated.h"

/**
 * The cow car train car has special cow-related
 * behaviors so it is a slightly different train car.
 */
UCLASS()
class HANDSTRAINSAMPLE_API ACowCar : public ANormalTrainCar
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "CowAnimations")
	void GoMooCowGo();
};
