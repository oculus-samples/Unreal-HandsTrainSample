// Fill out your copyright notice in the Description page of Project Settings.

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
