// Fill out your copyright notice in the Description page of Project Settings.

#include "NormalTrainCar.h"

ANormalTrainCar::ANormalTrainCar()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ANormalTrainCar::UpdateState(float DeltaTime)
{
	if (!IsValid(ParentLocomotive))
	{
		return;
	}

	// if everything is scaled, take that into account
	Distance = ParentLocomotive->GetDistance() - Scale * DistanceBehindParent;
	UpdateCarPosition();
	RotateCarWheels();
}
