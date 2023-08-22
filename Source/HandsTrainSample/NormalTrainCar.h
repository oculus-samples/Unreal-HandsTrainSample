// Fill out your copyright notice in the Description page of Project Settings.

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
