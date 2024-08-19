/*
Copyright (c) Meta Platforms, Inc. and affiliates.
All rights reserved.

This source code is licensed under the license found in the
LICENSE file in the root directory of this source tree.
*/

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "OculusXRInputFunctionLibrary.h"
#include "HandsVisualizationSwitcher.generated.h"

struct FOculusXRCapsuleCollider;
class UOculusXRHandComponent;

UENUM(BlueprintType)
enum class EOculusXRHandsVisualMode : uint8
{
	Mesh,
	Skeleton,
	Both,
};

USTRUCT(BlueprintType)
struct FLineSegment
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	int32 StartBoneIndex;
	UPROPERTY(BlueprintReadOnly)
	int32 EndBoneIndex;

	FLineSegment()
		: StartBoneIndex(0), EndBoneIndex(0)
	{
	}

	FLineSegment(int32 StartBoneIndex, int32 EndBoneIndex)
		: StartBoneIndex(StartBoneIndex), EndBoneIndex(EndBoneIndex)
	{
	}
};

/**
 * Controls visual state of hands.
 */
UCLASS()
class HANDSTRAINSAMPLE_API AHandsVisualizationSwitcher : public AActor
{
	GENERATED_BODY()

public:
	AHandsVisualizationSwitcher();

	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "Visual")
	void SwitchHandsVisualization();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Meshes")
	USceneComponent* RootSceneComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Visual")
	UInstancedStaticMeshComponent* LeftHandBoneInstancedMeshes;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Visual")
	UInstancedStaticMeshComponent* RightHandBoneInstancedMeshes;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Meshes")
	class UMaterialInterface* NormalBoneMaterial;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Meshes")
	class UMaterialInterface* SystemGestureBoneMaterial;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Hands")
	UOculusXRHandComponent* LeftHand;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Hands")
	UOculusXRHandComponent* RightHand;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Visual")
	EOculusXRHandsVisualMode CurrentVisualMode;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Visual")
	bool bLeftHandBonesVisible;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Visual")
	bool bRightHandBonesVisible;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Visual")
	TArray<FLineSegment> LeftHandSegmentInfos;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Visual")
	TArray<FLineSegment> RightHandSegmentInfos;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Visual")
	TArray<FTransform> LeftHandSegmentTransforms;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Visual")
	TArray<FTransform> RightHandSegmentTransforms;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UMaterialInterface* LeftHandMaterial;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UMaterialInterface* RightHandMaterial;

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Initialization")
	void FindBothHands();

	UFUNCTION(BlueprintCallable, Category = "Initialization")
	void InitializeMeshesFromHands();

	UFUNCTION(BlueprintCallable, Category = "Meshes")
	void ToggleBoneVisuals(bool bAreBoneVisualsEnabled);

private:
	const static FName LeftGestureActionName;
	const static FName RightGestureActionName;
	const static FName HandAlphaParamName;
	const static float BoneScaleFactor;

	const static float HandAlphaWhenBonesVisible;

	void EnforceCurrentVisualMode();

	void InitVisualsPerHand(UOculusXRHandComponent* Hand);
	void UpdateHandBoneVisuals(
		UOculusXRHandComponent* Hand,
		UInstancedStaticMeshComponent* BoneInstancedMeshes,
		TArray<FTransform>& SegmentTransforms,
		TArray<FLineSegment>& HandSegmentInfos,
		bool bMeshVisibility, bool bConfidenceIsHigh,
		float HandScale);

	void LeftHandUseSystemGestureBoneMaterial();
	void LeftHandUseNormalBoneMaterial();

	void RightHandUseSystemGestureBoneMaterial();
	void RightHandUseNormalBoneMaterial();
};
