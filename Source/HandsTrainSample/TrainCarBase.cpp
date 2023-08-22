// Fill out your copyright notice in the Description page of Project Settings.

#include "TrainCarBase.h"
#include "TrackSegment.h"
#include "Kismet/KismetMathLibrary.h"

const FVector ATrainCarBase::UpOffset(0.0f, 0.0f, 1.95f);
const float ATrainCarBase::WheelRadius = 2.7f;

ATrainCarBase::ATrainCarBase()
{
	PrimaryActorTick.bCanEverTick = false;

	Distance = 0.0f;
}

void ATrainCarBase::BeginPlay()
{
	Super::BeginPlay();

	TArray<USceneComponent*> childComponents;
	this->RootComponent->GetChildrenComponents(true, childComponents);
	for (USceneComponent* childComponent : childComponents)
	{
		FString childName = *childComponent->GetName();
		if (childName.Contains(TEXT("Wheel_A")))
		{
			TrainWheels.Add(childComponent);
		}
		else if (childName.Equals("WheelBaseFront"))
		{
			FrontWheelBase = childComponent;
		}
		else if (childName.Equals("WheelBaseBack"))
		{
			RearWheelBase = childComponent;
		}
	}
}

void ATrainCarBase::UpdateCarPosition()
{
	if (!IsValid(FrontWheelBase) || !IsValid(RearWheelBase))
	{
		return;
	}

	FVector FrontWheelsRelativeLoc = FrontWheelBase->GetRelativeLocation();
	FVector RearWheelsRelativeLoc = RearWheelBase->GetRelativeLocation();
	// because the model is rotated in the blueprint, forward in relative direction is z TODO remove
	UpdatePose(Distance + FrontWheelsRelativeLoc[0] * Scale, FrontPose);
	UpdatePose(Distance + RearWheelsRelativeLoc[0] * Scale, RearPose);

	const FVector& FrontPosePosition = FrontPose.GetLocation();
	const FVector& RearPosePosition = RearPose.GetLocation();
	FVector LookDirection = FrontPosePosition - RearPosePosition;
	FQuat LookRotation = ConstructLookRotation(LookDirection, this->GetActorUpVector());

	// while the train looks toward the front pose position, its
	// position is based on the center of the front and rear axles
	FVector MidPoint = 0.5f * (FrontPosePosition + RearPosePosition);
	this->SetActorLocation(MidPoint + UpOffset);
	this->SetActorRotation(LookRotation);
	FrontWheelBase->SetWorldRotation(FrontPose.GetRotation());
	RearWheelBase->SetWorldRotation(RearPose.GetRotation());
}

void ATrainCarBase::RotateCarWheels()
{
	// dividing distance by radius gives us how
	// many radians we have traveled
	float AngleOfRot = fmod(Distance / ATrainCarBase::WheelRadius, TWO_PI);
	for (auto Wheel : TrainWheels)
	{
		if (IsValid(Wheel))
		{
			Wheel->SetRelativeRotation(FRotator(-180.0f / PI * AngleOfRot, 0, 0));
		}
	}
}

void ATrainCarBase::UpdatePose(float PoseDistance, FTransform& Pose)
{
	if (!IsValid(TrainTrack))
	{
		return;
	}
	float TrackLength = TrainTrack->GetTrackLength();
	// distance can be negative; add track length to it
	// in case that happens
	PoseDistance = fmod(TrackLength + PoseDistance, TrackLength);
	if (PoseDistance < 0.0f)
	{
		PoseDistance += TrackLength;
	}

	ATrackSegment* trackSegment = TrainTrack->GetTrackSegment(PoseDistance);
	if (IsValid(trackSegment))
	{
		float DistanceIntoSegment = PoseDistance - trackSegment->StartDistance;
		trackSegment->UpdatePoseInSegment(DistanceIntoSegment, Pose);
	}
}

FQuat ATrainCarBase::ConstructLookRotation(const FVector& LookDirection,
	const FVector& UpVector)
{
	return UKismetMathLibrary::MakeRotFromXZ(
		LookDirection.GetSafeNormal(), UpVector.GetSafeNormal())
		.Quaternion();
}

void ATrainCarBase::UpdateState(float DeltaTime)
{
	// inheriting classes will override this function
	// this normally should be pure virtual but UE4 forbids
	// such functions in actor classes
}
