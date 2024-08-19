/*
Copyright (c) Meta Platforms, Inc. and affiliates.
All rights reserved.

This source code is licensed under the license found in the
LICENSE file in the root directory of this source tree.
*/

#include "TrainTrack.h"
#include "Containers/Array.h"
#include "NormalTrainCar.h"
#include "TrainLocomotive.h"
#include "TrainParent.h"

ATrainTrack::ATrainTrack()
{
	PrimaryActorTick.bCanEverTick = false;

	GridSize = 45.0f;

	RootSceneComponent = CreateDefaultSubobject<USceneComponent>(FName(TEXT("Root")));
	RootComponent = RootSceneComponent;

	SegmentParent = CreateDefaultSubobject<USceneComponent>(FName(TEXT("SegmentParent")));
	SegmentParent->SetupAttachment(RootComponent);
	TrainParent = CreateDefaultSubobject<USceneComponent>(FName(TEXT("TrainParent")));
	TrainParent->SetupAttachment(RootComponent);
}

void ATrainTrack::BeginPlay()
{
	Super::BeginPlay();
	InitializeSegmentReferences();
	SetUpTrackSegmentDistances();

	InitializeTrain();
}

// Regenerate in the editor. Should rebake lighting after.
void ATrainTrack::SetUpTrack()
{
	TArray<UTrackSegmentMetaInfo*> TrackSegmentInfos;
	GetComponents<UTrackSegmentMetaInfo>(TrackSegmentInfos);
	TrackSegmentInfos.Sort(ATrainTrack::SegmentInfoPredicate);

	TArray<float> SegmentDistances;
	SetUpTrackSegmentInformationDistances(TrackSegmentInfos, SegmentDistances);
	CreateTrackSegments(TrackSegmentInfos, SegmentDistances);
}

void ATrainTrack::LogTrackInformation()
{
	TArray<AActor*> AttachedActors;
	GetAttachedActors(AttachedActors, true);
	// print out information regarding segments locations and rotations
	for (AActor* AttachedActor : AttachedActors)
	{
		ATrackSegment* CastedTrackSegment = Cast<ATrackSegment>(AttachedActor);
		if (!IsValid(CastedTrackSegment))
		{
			continue;
		}
		FTransform ActorTransform = CastedTrackSegment->GetTransform();
		auto RelativeTransform = ActorTransform.GetRelativeTransform(this->GetTransform());
		auto RelativeLocation = RelativeTransform.GetLocation();
		auto RelativeRotation = RelativeTransform.GetRotation().Euler();
		auto RelativeScale = RelativeTransform.GetScale3D();
		UE_LOG(
			LogTemp,
			Log,
			TEXT(
				"Relative segment (%d) location: (%f, %f, %f), rotation: (%f, %f, %f), scale: (%f, %f, %f)"),
			CastedTrackSegment->SegmentIndex,
			RelativeLocation[0],
			RelativeLocation[1],
			RelativeLocation[2],
			RelativeRotation[0],
			RelativeRotation[1],
			RelativeRotation[2],
			RelativeScale[0],
			RelativeScale[1],
			RelativeScale[2]);

		UStaticMeshComponent* MeshComp = CastedTrackSegment->GetMeshComp();
		RelativeTransform = MeshComp->GetRelativeTransform();
		FVector RelativeScaleMesh = RelativeTransform.GetScale3D();
		UE_LOG(LogTemp, Log, TEXT("Mesh scale: %f"), RelativeScaleMesh[0]);
	}
}

void ATrainTrack::SetUpTrackSegmentInformationDistances(
	const TArray<UTrackSegmentMetaInfo*>& TrackSegmentInfos,
	TArray<float>& SegmentDistances)
{
	float CurrTrackLength = 0;
	for (UTrackSegmentMetaInfo* TrackSegmentInfo : TrackSegmentInfos)
	{
		SegmentDistances.Add(CurrTrackLength);
		CurrTrackLength += TrackSegmentInfo->GetSegmentLength(GridSize);
	}
}

void ATrainTrack::CreateTrackSegments(
	const TArray<UTrackSegmentMetaInfo*>& TrackSegmentInfos,
	const TArray<float>& SegmentDistances)
{
	// remove old segments, if any
	TArray<AActor*> AttachedActors;
	GetAttachedActors(AttachedActors, true);
	for (AActor* Actor : AttachedActors)
	{
		ATrackSegment* CastedTrackSegment = Cast<ATrackSegment>(Actor);
		if (IsValid(CastedTrackSegment))
		{
			CastedTrackSegment->Destroy();
		}
	}

	int ChildCount = TrackSegmentInfos.Num();
	ATrackSegment* LastTrackSegment = nullptr;
	for (int i = 0; i < ChildCount; i++)
	{
		UTrackSegmentMetaInfo* TrackSegmentInfo =
			TrackSegmentInfos[i];
		float Distance = SegmentDistances[i];

		ATrackSegment* TrackSegment = GetWorld()->SpawnActor<ATrackSegment>(
			TrackSegmentBP,
			GetActorLocation(),
			GetActorRotation());

		if (!IsValid(TrackSegment))
		{
			UE_LOG(LogTemp, Error, TEXT("Could not spawn track segment %d!"),
				i);
			continue;
		}
		TrackSegment->AttachToActor(this,
			FAttachmentTransformRules::KeepWorldTransform,
			FName("SegmentParent"));
		TrackSegment->TrackSegmentType = TrackSegmentInfo->TrackSegmentType;
		TrackSegment->SegmentIndex = TrackSegmentInfo->SegmentIndex;
		TrackSegment->StartDistance = Distance;
		TrackSegment->SetGridSizeAndReturnScaleRatio(GridSize);

		// the starting position of this track segment should be
		// based on the end pose of the last segment
		if (LastTrackSegment != nullptr)
		{
			auto endPose = LastTrackSegment->GetEndPose();
			// zero out relative values first, then set position (in case blueprint has values)
			TrackSegment->SetActorRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
			TrackSegment->SetActorRelativeRotation(FQuat::Identity);
			TrackSegment->SetActorLocation(endPose.GetLocation());
			TrackSegment->SetActorRotation(endPose.GetRotation());
		}

		TrackSegment->EnableMeshAndRegenerateTrack();
		TrackSegment->MoveMeshBottomTowardPivot();
		LastTrackSegment = TrackSegment;
	}
}

void ATrainTrack::InitializeSegmentReferences()
{
	// reset all references before setting them
	TrackSegments.Empty();

	TArray<AActor*> AttachedActors;
	GetAttachedActors(AttachedActors, true);
	for (AActor* Actor : AttachedActors)
	{
		ATrackSegment* CastedTrackSegment = Cast<ATrackSegment>(Actor);
		if (IsValid(CastedTrackSegment))
		{
			TrackSegments.Add(CastedTrackSegment);
		}
	}

	TrackSegments.Sort(ATrainTrack::SegmentPredicate);
}

void ATrainTrack::InitializeTrain()
{
	TArray<class ANormalTrainCar*> NormalTrainCars;
	ATrainLocomotive* TrainLocomotive = nullptr;

	TArray<AActor*> ChildActors;
	GetAllChildActors(ChildActors, true);
	ATrainParent* TrainParentActor = nullptr;

	for (AActor* Actor : ChildActors)
	{
		ATrainParent* CastedTrainParent = Cast<ATrainParent>(Actor);
		if (IsValid(CastedTrainParent))
		{
			TrainParentActor = CastedTrainParent;
			break;
		}
	}

	if (IsValid(TrainParentActor))
	{
		TrainParentActor->SpawnTrainCars(this);
		TrainLocomotive = TrainParentActor->TrainLocomotive;
		NormalTrainCars = TrainParentActor->TrainChildCars;
	}

	ScaleTrainByScaleRatio(NormalTrainCars, TrainLocomotive);
}

void ATrainTrack::SetUpTrackSegmentDistances()
{
	TrackLength = 0;

	for (ATrackSegment* TrackSegment : TrackSegments)
	{
		if (!IsValid(TrackSegment))
		{
			continue;
		}
		TrackSegment->SetGridSizeAndReturnScaleRatio(GridSize);
		TrackSegment->StartDistance = TrackLength;
		TrackLength += TrackSegment->GetSegmentLength();
	}
}

void ATrainTrack::ScaleTrainByScaleRatio(TArray<ANormalTrainCar*> NormalTrainCars,
	ATrainLocomotive* TrainLocomotive)
{
	if (TrackSegments.Num() == 0 || !IsValid(TrackSegments[0]))
	{
		return;
	}

	float SegmentScaleRatio = TrackSegments[0]->SetGridSizeAndReturnScaleRatio(GridSize);
	// scale everything up. the train is sized according to the default segment size
	// so if the segment scales up or down, the train scales accordingly
	if (TrainParent != nullptr && IsValid(TrainParent))
	{
		TrainParent->SetRelativeScale3D(
			FVector(SegmentScaleRatio, SegmentScaleRatio, SegmentScaleRatio));
	}
	if (IsValid(TrainLocomotive))
	{
		TrainLocomotive->Scale = SegmentScaleRatio;
	}
	for (auto Car : NormalTrainCars)
	{
		if (IsValid(Car))
		{
			Car->Scale = SegmentScaleRatio;
		}
	}
}

ATrackSegment* ATrainTrack::GetTrackSegment(float DistanceIntoTrack)
{
	unsigned int NumSegments = TrackSegments.Num();
	unsigned int LastSegmentIndex = NumSegments - 1;
	for (unsigned int SegmentIndex = 0; SegmentIndex < NumSegments; SegmentIndex++)
	{
		auto CurrSegment = TrackSegments[SegmentIndex];
		auto NextSegment = TrackSegments[(SegmentIndex + 1) % NumSegments];
		if (DistanceIntoTrack >= CurrSegment->StartDistance && (DistanceIntoTrack < NextSegment->StartDistance || SegmentIndex == LastSegmentIndex))
		{
			return CurrSegment;
		}
	}

	return nullptr;
}
