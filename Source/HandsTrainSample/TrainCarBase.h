// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TrainTrack.h"
#include "TrainCarBase.generated.h"

/**
 * Base class for all trains. The blueprint is constructed via
 * importing an FBX, so all children of this actor's scene graph
 * are found manually.
 */
UCLASS()
class HANDSTRAINSAMPLE_API ATrainCarBase : public AActor
{
	GENERATED_BODY()

public:
	ATrainCarBase();

	virtual void UpdateState(float DeltaTime);

	// we need to know where we are on the track mathematically, so have
	// a variable that indicates our scale
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Meshes)
	float Scale;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Msc)
	ATrainTrack* TrainTrack;

	UFUNCTION(BlueprintCallable, BlueprintPure)
	float GetDistance() const
	{
		return Distance;
	}

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float Distance;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Meshes)
	class USceneComponent* FrontWheelBase;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Meshes)
	class USceneComponent* RearWheelBase;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Meshes)
	TArray<USceneComponent*> TrainWheels;

	UFUNCTION(BlueprintCallable)
	void UpdateCarPosition();

	UFUNCTION(BlueprintCallable)
	void RotateCarWheels();

private:
	// how far are offset from the track?
	const static FVector UpOffset;
	const static float WheelRadius;

	FTransform FrontPose, RearPose;

	void UpdatePose(float PoseDistance, FTransform& Pose);

	FQuat ConstructLookRotation(const FVector& LookDirection, const FVector& UpVector);
};
