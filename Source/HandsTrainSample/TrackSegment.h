// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TrackSegment.generated.h"

UENUM(BlueprintType)
enum class ESegmentType : uint8
{
	Straight,
	LeftTurn,
	RightTurn,
};

UCLASS()
class HANDSTRAINSAMPLE_API ATrackSegment : public AActor
{
	GENERATED_BODY()

public:
	ATrackSegment();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enum")
	ESegmentType TrackSegmentType;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Meshes")
	class USceneComponent* RootSceneComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Meshes")
	UStaticMeshComponent* StraightSegment;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Meshes")
	UStaticMeshComponent* LeftSegment;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Meshes")
	UStaticMeshComponent* RightSegment;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Meshes")
	TArray<class UStaticMeshComponent*> SegmentMeshes;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dimensions",
		meta = (Tooltip = "Length of track segment's side"))
	float GridSize;

	UPROPERTY(VisibleAnywhere, Category = "Positioning",
		meta = (Tooltip = "Start distance of segment"))
	float StartDistance;

	// Use index to assign an order to the segments of the track
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Positioning")
	int SegmentIndex;

	UFUNCTION(BlueprintCallable, Category = "Positioning")
	FTransform GetEndPose() const;

	UFUNCTION(BlueprintCallable, Category = "Positioning")
	float GetSegmentLength() const;

	UFUNCTION(BlueprintCallable, Category = "Positioning")
	float SetGridSizeAndReturnScaleRatio(float NewSize)
	{
		GridSize = NewSize;
		return GridSize / OriginalMeshGridSize;
	}

	UFUNCTION(BlueprintCallable, Category = "Positioning")
	class UStaticMeshComponent* GetMeshComp()
	{
		return SegmentMeshes[(unsigned int)TrackSegmentType];
	}

	UFUNCTION(BlueprintCallable, Category = "Positioning")
	void EnableMeshAndRegenerateTrack();

	UFUNCTION(BlueprintCallable, Category = "Positioning")
	void MoveMeshBottomTowardPivot();

	UFUNCTION(BlueprintCallable, Category = "Positioning")
	void UpdatePoseInSegment(float DistanceIntoSegment, FTransform& Pose) const;

protected:
	const static float OriginalMeshGridSize;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Positioning")
	float ComputeRadius() const
	{
		return 0.5f * GridSize;
	}

	UFUNCTION(BlueprintCallable, Category = "Positioning")
	void ToggleStaticMesh(UStaticMeshComponent* MeshComp,
		bool ToggleValue);
};
