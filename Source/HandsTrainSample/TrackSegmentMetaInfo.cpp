/*
Copyright (c) Meta Platforms, Inc. and affiliates.
All rights reserved.

This source code is licensed under the license found in the
LICENSE file in the root directory of this source tree.
*/

#include "TrackSegmentMetaInfo.h"

UTrackSegmentMetaInfo::UTrackSegmentMetaInfo()
{
	PrimaryComponentTick.bCanEverTick = false;
}

float UTrackSegmentMetaInfo::GetSegmentLength(float GridSize) const
{
	switch (TrackSegmentType)
	{
		case ESegmentType::Straight:
			return GridSize;
		default:
			// return quarter of circumference
			// for turns
			return 0.5f * PI * ComputeRadius(GridSize);
	}
}
