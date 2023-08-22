// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "TrackSegment.h"
#include "TrackSegmentMetaInfo.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class HANDSTRAINSAMPLE_API UTrackSegmentMetaInfo : public USceneComponent
{
	GENERATED_BODY()

public:
	UTrackSegmentMetaInfo();

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Enum")
	ESegmentType TrackSegmentType;

	// Use index to assign an order to the segments of the track
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Positioning")
	int SegmentIndex;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Positioning",
		meta = (Tooltip = "Start distance of segment"))
	float StartDistance;

	UFUNCTION(BlueprintCallable, Category = "Positioning")
	float GetSegmentLength(float GridSize) const;

protected:
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Positioning")
	float ComputeRadius(float GridSize) const
	{
		return 0.5f * GridSize;
	}
};
