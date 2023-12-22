// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NormalTrainCar.h"
#include "TrainLocomotive.h"
#include "TrainParent.generated.h"

class ATrainTrack;

UCLASS()
class HANDSTRAINSAMPLE_API ATrainParent : public AActor
{
	GENERATED_BODY()

public:
	ATrainParent();

	UFUNCTION(BlueprintCallable, Category = "Behaviors")
	void SpawnTrainCars(ATrainTrack* ParentTrack);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	ATrainLocomotive* TrainLocomotive;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<ANormalTrainCar*> TrainChildCars;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Initialization")
	TSubclassOf<ATrainLocomotive> TrainLocomotiveBP;

	UPROPERTY(EditDefaultsOnly, Category = "Initialization")
	TSubclassOf<ANormalTrainCar> TrainCoalCarBP;

	UPROPERTY(EditDefaultsOnly, Category = "Initialization")
	TSubclassOf<ANormalTrainCar> TrainBoxCarBP;

	UPROPERTY(EditDefaultsOnly, Category = "Initialization")
	TSubclassOf<ANormalTrainCar> TrainOilCarBP;

	UPROPERTY(EditDefaultsOnly, Category = "Initialization")
	TSubclassOf<ANormalTrainCar> TrainLumberCarBP;

	UPROPERTY(EditDefaultsOnly, Category = "Initialization")
	TSubclassOf<ANormalTrainCar> TrainCowCarBP;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Initialization")
	USceneComponent* TrainLocomotiveAnchorComp;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Initialization")
	USceneComponent* TrainCoalCarAnchorComp;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Initialization")
	USceneComponent* TrainBoxCarAnchorComp;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Initialization")
	USceneComponent* TrainOilCarAnchorComp;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Initialization")
	USceneComponent* TrainLumberCarAnchorComp;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Initialization")
	USceneComponent* TrainCowCarAnchorComp;

private:
	void SpawnTrainCar(ATrainTrack* ParentTrack, USceneComponent* AnchorComp,
		TSubclassOf<ANormalTrainCar> ReferenceBlueprint);
};
