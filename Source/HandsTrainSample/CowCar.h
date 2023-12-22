// Fill out your copyright notice in the Description page of Project Settings.

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
