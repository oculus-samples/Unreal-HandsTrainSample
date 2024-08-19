/*
Copyright (c) Meta Platforms, Inc. and affiliates.
All rights reserved.

This source code is licensed under the license found in the
LICENSE file in the root directory of this source tree.
*/

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
