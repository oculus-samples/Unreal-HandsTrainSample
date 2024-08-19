/*
Copyright (c) Meta Platforms, Inc. and affiliates.
All rights reserved.

This source code is licensed under the license found in the
LICENSE file in the root directory of this source tree.
*/

#include "TrainParent.h"

ATrainParent::ATrainParent()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ATrainParent::SpawnTrainCars(ATrainTrack* ParentTrack)
{
	TrainLocomotiveAnchorComp = (USceneComponent*)GetDefaultSubobjectByName(
		FName(TEXT("TrainLocomotiveAnchor")));
	TrainCoalCarAnchorComp = (USceneComponent*)GetDefaultSubobjectByName(
		FName(TEXT("TrainCoalCarAnchor")));
	TrainBoxCarAnchorComp = (USceneComponent*)GetDefaultSubobjectByName(
		FName(TEXT("TrainBoxCarAnchor")));
	TrainOilCarAnchorComp = (USceneComponent*)GetDefaultSubobjectByName(
		FName(TEXT("TrainOilCarAnchor")));
	TrainLumberCarAnchorComp = (USceneComponent*)GetDefaultSubobjectByName(
		FName(TEXT("TrainLumberCarAnchor")));
	TrainCowCarAnchorComp = (USceneComponent*)GetDefaultSubobjectByName(
		FName(TEXT("TrainCowCarAnchor")));

	TrainLocomotive = GetWorld()->SpawnActor<ATrainLocomotive>(
		TrainLocomotiveBP,
		TrainLocomotiveAnchorComp->GetComponentLocation(),
		TrainLocomotiveAnchorComp->GetComponentRotation());
	TrainLocomotive->TrainTrack = ParentTrack;
	TrainLocomotive->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);
	TrainLocomotive->SetActorRelativeScale3D(TrainLocomotiveAnchorComp->GetRelativeScale3D());

	SpawnTrainCar(ParentTrack, TrainCoalCarAnchorComp, TrainCoalCarBP);
	SpawnTrainCar(ParentTrack, TrainBoxCarAnchorComp, TrainBoxCarBP);
	SpawnTrainCar(ParentTrack, TrainOilCarAnchorComp, TrainOilCarBP);
	SpawnTrainCar(ParentTrack, TrainLumberCarAnchorComp, TrainLumberCarBP);
	SpawnTrainCar(ParentTrack, TrainCowCarAnchorComp, TrainCowCarBP);

	TrainLocomotive->Initialize(TrainChildCars);
	TrainLocomotive->StartStopTrain(true);
}

void ATrainParent::SpawnTrainCar(ATrainTrack* ParentTrack, USceneComponent* AnchorComp,
	TSubclassOf<ANormalTrainCar> ReferenceBlueprint)
{
	ANormalTrainCar* NewTrainCar = GetWorld()->SpawnActor<ANormalTrainCar>(
		ReferenceBlueprint,
		AnchorComp->GetComponentLocation(),
		AnchorComp->GetComponentRotation());
	TrainChildCars.Add(NewTrainCar);
	NewTrainCar->TrainTrack = ParentTrack;
	NewTrainCar->DistanceBehindParent = fabs(AnchorComp->GetRelativeLocation().X);
	NewTrainCar->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);
	NewTrainCar->SetActorRelativeScale3D(AnchorComp->GetRelativeScale3D());
}
