/*
Copyright (c) Meta Platforms, Inc. and affiliates.
All rights reserved.

This source code is licensed under the license found in the
LICENSE file in the root directory of this source tree.
*/

#include "TrackSegment.h"
#include "Components/StaticMeshComponent.h"

const float ATrackSegment::OriginalMeshGridSize = 31.5f;

// Sets default values
ATrackSegment::ATrackSegment()
{
	PrimaryActorTick.bCanEverTick = false;
	TrackSegmentType = ESegmentType::Straight;

	RootSceneComponent = CreateDefaultSubobject<USceneComponent>(FName(TEXT("Root")));
	RootComponent = RootSceneComponent;

	StraightSegment = CreateDefaultSubobject<UStaticMeshComponent>(
		FName(TEXT("Straight")));
	StraightSegment->SetupAttachment(RootComponent);
	LeftSegment = CreateDefaultSubobject<UStaticMeshComponent>(
		FName(TEXT("Left")));
	LeftSegment->SetupAttachment(RootComponent);
	RightSegment = CreateDefaultSubobject<UStaticMeshComponent>(
		FName(TEXT("Right")));
	RightSegment->SetupAttachment(RootComponent);

	SegmentMeshes.Add(StraightSegment);
	SegmentMeshes.Add(LeftSegment);
	SegmentMeshes.Add(RightSegment);

	// don't enable mesh until we are initialized
	ToggleStaticMesh(StraightSegment, false);
	ToggleStaticMesh(LeftSegment, false);
	ToggleStaticMesh(RightSegment, false);
}

float ATrackSegment::GetSegmentLength() const
{
	switch (TrackSegmentType)
	{
		case ESegmentType::Straight:
			return GridSize;
		default:
			// return quarter of circumference
			// for turns
			return 0.5f * PI * ComputeRadius();
	}
}

FTransform ATrackSegment::GetEndPose() const
{
	FTransform newPose;
	UpdatePoseInSegment(GetSegmentLength(), newPose);
	return newPose;
}

// note that start of segment's mesh starts at 0 for forward (X) in local space
//
void ATrackSegment::UpdatePoseInSegment(float DistanceIntoSegment, FTransform& Pose) const
{
	float CurrentRadius = ComputeRadius();

	if (TrackSegmentType == ESegmentType::Straight)
	{
		Pose.SetLocation(GetActorLocation() + DistanceIntoSegment * GetActorForwardVector());
		Pose.SetRotation(GetActorRotation().Quaternion());
	}
	else if (TrackSegmentType == ESegmentType::LeftTurn)
	{
		float NormalizedDistanceIntoSegment = DistanceIntoSegment / GetSegmentLength();
		// the turn is 90 degrees, so find out how far we are into it
		// by multiply angle for quarter circle by normalized distance into it
		float Angle = 0.5f * PI * NormalizedDistanceIntoSegment;
		const FTransform& actorTransform = GetTransform();

		// sin represents x component in local space, which starts from 0 and goes to radius
		// cos, or the right component, starts from 0 and goes to -radius
		FVector LocalPosition(CurrentRadius * sin(Angle),
			CurrentRadius * cos(Angle) - CurrentRadius, 0.0f);
		// args are pitch, yaw, then roll -- rotate around yaw or up axis since this is a turn
		// note negation (left turn means train pose needs to go in opposite direction -- opposite
		// of left-hand rule)
		FRotator LocalRotation(0, -Angle * 180.0f / PI, 0);
		Pose.SetLocation(actorTransform.TransformPosition(LocalPosition));
		Pose.SetRotation(actorTransform.TransformRotation(LocalRotation.Quaternion()));
	}
	else
	{
		float NormalizedDistanceIntoSegment = DistanceIntoSegment / GetSegmentLength();
		float Angle = 0.5f * PI * NormalizedDistanceIntoSegment;
		const FTransform& ActorTransform = GetTransform();
		// forward goes from 0 to radius units
		// right component goes from 0 to radius units
		FVector LocalPosition(CurrentRadius * sin(Angle),
			CurrentRadius - CurrentRadius * cos(Angle), 0.0f);
		FRotator LocalRotation(0, Angle * 180.0f / PI, 0);
		Pose.SetLocation(ActorTransform.TransformPosition(LocalPosition));
		Pose.SetRotation(ActorTransform.TransformRotation(LocalRotation.Quaternion()));
	}
}

void ATrackSegment::EnableMeshAndRegenerateTrack()
{
	for (unsigned int SegmentTypeIndex = (unsigned int)ESegmentType::Straight;
		 SegmentTypeIndex <= (unsigned int)ESegmentType::RightTurn; SegmentTypeIndex++)
	{
		ToggleStaticMesh(SegmentMeshes[SegmentTypeIndex],
			SegmentTypeIndex == (unsigned int)TrackSegmentType);
	}

	UStaticMeshComponent* MeshComp = SegmentMeshes[(unsigned int)TrackSegmentType];
	if (IsValid(MeshComp))
	{
		MeshComp->SetRelativeScale3D(FVector(
			GridSize / OriginalMeshGridSize,
			GridSize / OriginalMeshGridSize,
			GridSize / OriginalMeshGridSize));
	}
}

// zero distance means bottom of mesh, and bottom of
// mesh should be at the origin in local space. so move
// mesh forward halfway
void ATrackSegment::MoveMeshBottomTowardPivot()
{
	UStaticMeshComponent* MeshComp = SegmentMeshes[(unsigned int)TrackSegmentType];

	if (IsValid(MeshComp))
	{
		MeshComp->SetRelativeLocation(FVector::ForwardVector * GridSize * 0.5f);
	}
}

void ATrackSegment::ToggleStaticMesh(UStaticMeshComponent* MeshComp, bool ToggleValue)
{
	if (IsValid(MeshComp))
	{
		MeshComp->SetVisibility(ToggleValue, true);
	}
}
