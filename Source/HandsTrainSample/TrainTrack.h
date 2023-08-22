#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TrackSegment.h"
#include "TrackSegmentMetaInfo.h"
#include "TrainTrack.generated.h"

class ATrackSegment;

UCLASS()
class HANDSTRAINSAMPLE_API ATrainTrack : public AActor
{
	GENERATED_BODY()

public:
	ATrainTrack();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Dimensions,
		meta = (Tooltip = "Length of track segment's side"))
	float GridSize;

	UPROPERTY(EditDefaultsOnly, Category = "Track Spawning")
	TSubclassOf<ATrackSegment> TrackSegmentBP;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Meshes)
	class USceneComponent* RootSceneComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = TrainVis)
	class USceneComponent* SegmentParent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = TrainVis)
	class USceneComponent* TrainParent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Track)
	TArray<ATrackSegment*> TrackSegments;

	UFUNCTION(CallInEditor, BlueprintCallable, Category = "Track")
	void SetUpTrack();

	UFUNCTION(CallInEditor, BlueprintCallable, Category = "Track")
	void LogTrackInformation();

	UFUNCTION(BlueprintPure, BlueprintCallable, Category = "Track")
	float GetTrackLength() const
	{
		return TrackLength;
	}

	UFUNCTION(BlueprintCallable, Category = "Track")
	ATrackSegment* GetTrackSegment(float DistanceIntoTrack);

	inline static bool SegmentPredicate(const ATrackSegment& Segment1,
		const ATrackSegment& Segment2)
	{
		return Segment1.SegmentIndex < Segment2.SegmentIndex;
	}

	inline static bool SegmentInfoPredicate(
		const UTrackSegmentMetaInfo& SegmentInfo1,
		const UTrackSegmentMetaInfo& SegmentInfo2)
	{
		return SegmentInfo1.SegmentIndex < SegmentInfo2.SegmentIndex;
	}

protected:
	virtual void BeginPlay() override;

private:
	float TrackLength;

	void SetUpTrackSegmentInformationDistances(
		const TArray<UTrackSegmentMetaInfo*>& TrackSegmentInfos,
		TArray<float>& SegmentDistances);
	void CreateTrackSegments(
		const TArray<UTrackSegmentMetaInfo*>& TrackSegmentInfos,
		const TArray<float>& SegmentDistances);

	void InitializeSegmentReferences();
	void InitializeTrain();
	void SetUpTrackSegmentDistances();
	void ScaleTrainByScaleRatio(TArray<class ANormalTrainCar*> NormalTrainCars,
		class ATrainLocomotive* TrainLocomotive);
};
