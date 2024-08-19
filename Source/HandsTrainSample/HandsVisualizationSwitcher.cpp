/*
Copyright (c) Meta Platforms, Inc. and affiliates.
All rights reserved.

This source code is licensed under the license found in the
LICENSE file in the root directory of this source tree.
*/

#include "HandsVisualizationSwitcher.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/PoseableMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "OculusXRHandComponent.h"
#include "OculusXRInputFunctionLibrary.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Kismet/KismetMathLibrary.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Logging/StructuredLog.h"

const FName AHandsVisualizationSwitcher::HandAlphaParamName = FName(TEXT("HandAlpha"));
const FName AHandsVisualizationSwitcher::LeftGestureActionName = FName(TEXT("OculusHand_Left_SystemGesture"));
const FName AHandsVisualizationSwitcher::RightGestureActionName = FName(TEXT("OculusHand_Right_SystemGesture"));

const float AHandsVisualizationSwitcher::BoneScaleFactor = 0.01f;
const float AHandsVisualizationSwitcher::HandAlphaWhenBonesVisible = 0.4f;

AHandsVisualizationSwitcher::AHandsVisualizationSwitcher()
{
	PrimaryActorTick.bCanEverTick = true;

	RootSceneComponent = CreateDefaultSubobject<USceneComponent>(FName(TEXT("Root")));
	RootComponent = RootSceneComponent;

	LeftHandBoneInstancedMeshes = CreateDefaultSubobject<UInstancedStaticMeshComponent>(
		FName(TEXT("LeftHandBoneInstancedMeshes")));
	LeftHandBoneInstancedMeshes->SetupAttachment(RootComponent);
	LeftHandBoneInstancedMeshes->SetMobility(EComponentMobility::Movable);

	RightHandBoneInstancedMeshes = CreateDefaultSubobject<UInstancedStaticMeshComponent>(
		FName(TEXT("RightHandBoneInstancedMeshes")));
	RightHandBoneInstancedMeshes->SetupAttachment(RootComponent);
	RightHandBoneInstancedMeshes->SetMobility(EComponentMobility::Movable);
}

void AHandsVisualizationSwitcher::BeginPlay()
{
	Super::BeginPlay();
	EnableInput(GetWorld()->GetFirstPlayerController());
	InputComponent->BindAction(LeftGestureActionName,
		EInputEvent::IE_Pressed, this,
		&AHandsVisualizationSwitcher::LeftHandUseSystemGestureBoneMaterial);
	InputComponent->BindAction(LeftGestureActionName,
		EInputEvent::IE_Released, this,
		&AHandsVisualizationSwitcher::LeftHandUseNormalBoneMaterial);

	InputComponent->BindAction(RightGestureActionName,
		EInputEvent::IE_Pressed, this,
		&AHandsVisualizationSwitcher::RightHandUseSystemGestureBoneMaterial);
	InputComponent->BindAction(RightGestureActionName,
		EInputEvent::IE_Released, this,
		&AHandsVisualizationSwitcher::RightHandUseNormalBoneMaterial);

	FindBothHands();

	bLeftHandBonesVisible = false;
	bRightHandBonesVisible = false;
}

void AHandsVisualizationSwitcher::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!IsValid(LeftHand) || !IsValid(RightHand))
	{
		return;
	}

	bool bLeftHandConfidenceHigh = UOculusXRInputFunctionLibrary::IsHandTrackingEnabled() && UOculusXRInputFunctionLibrary::GetTrackingConfidence(LeftHand->SkeletonType, 0) == EOculusXRTrackingConfidence::High;
	UpdateHandBoneVisuals(LeftHand, LeftHandBoneInstancedMeshes,
		LeftHandSegmentTransforms,
		LeftHandSegmentInfos,
		bLeftHandBonesVisible, bLeftHandConfidenceHigh,
		UOculusXRInputFunctionLibrary::GetHandScale(LeftHand->SkeletonType));

	bool bRightHandConfidenceHigh = UOculusXRInputFunctionLibrary::IsHandTrackingEnabled() && UOculusXRInputFunctionLibrary::GetTrackingConfidence(RightHand->SkeletonType, 0) == EOculusXRTrackingConfidence::High;
	UpdateHandBoneVisuals(RightHand, RightHandBoneInstancedMeshes,
		RightHandSegmentTransforms,
		RightHandSegmentInfos,
		bRightHandBonesVisible, bRightHandConfidenceHigh,
		UOculusXRInputFunctionLibrary::GetHandScale(RightHand->SkeletonType));

	// if the hand component toggled the alpha of our hand (it can happen if the hand
	// detects the system gesture), make sure that is corrected
	float ProperAlpha = (CurrentVisualMode == EOculusXRHandsVisualMode::Both) ? HandAlphaWhenBonesVisible : 1.0f;
	float LeftAlpha;
	float RightAlpha;
	if (CurrentVisualMode == EOculusXRHandsVisualMode::Both && LeftHandMaterial->GetScalarParameterValue(HandAlphaParamName, LeftAlpha) && RightHandMaterial->GetScalarParameterValue(HandAlphaParamName, RightAlpha) && (LeftAlpha > ProperAlpha || RightAlpha > ProperAlpha))
	{
		LeftHand->SetScalarParameterValueOnMaterials(HandAlphaParamName, ProperAlpha);
		RightHand->SetScalarParameterValueOnMaterials(HandAlphaParamName, ProperAlpha);
	}
}

void AHandsVisualizationSwitcher::InitializeMeshesFromHands()
{
	if (!IsValid(LeftHand) || !IsValid(RightHand))
	{
		UE_LOG(LogTemp, Warning, TEXT("Hands Visualization Switcher could not find left or right hand!"));
		return;
	}

	LeftHandMaterial = LeftHand->GetMaterial(0);
	RightHandMaterial = RightHand->GetMaterial(0);

	InitVisualsPerHand(LeftHand);
	InitVisualsPerHand(RightHand);
	EnforceCurrentVisualMode();
}

void AHandsVisualizationSwitcher::InitVisualsPerHand(UOculusXRHandComponent* Hand)
{
	FTransform StandardBoneTransform(FQuat::Identity, FVector::ZeroVector,
		FVector(1.0f, 0.01f, 0.01f));
	bool IsLeftHand = Hand->SkeletonType == EOculusXRHandType::HandLeft;

	int32 NumHandBones = Hand->GetNumBones();
	FName NoParent(NAME_None);

	for (int32 BoneIndex = 0; BoneIndex < NumHandBones; BoneIndex++)
	{
		// create segment if bone has a parent (so we can create segment
		// from parent to this bone)
		auto BoneName = Hand->GetBoneName(BoneIndex);
		auto ParentBoneName = Hand->GetParentBone(BoneName);
		if (ParentBoneName.Compare(NoParent))
		{
			auto ParentIndex = Hand->GetBoneIndex(ParentBoneName);
			FLineSegment NewLineSegment(ParentIndex, BoneIndex);

			if (IsLeftHand)
			{
				LeftHandBoneInstancedMeshes->AddInstance(StandardBoneTransform);
				LeftHandSegmentInfos.Add(NewLineSegment);
				LeftHandSegmentTransforms.Add(StandardBoneTransform);
			}
			else
			{
				RightHandBoneInstancedMeshes->AddInstance(StandardBoneTransform);
				RightHandSegmentInfos.Add(NewLineSegment);
				RightHandSegmentTransforms.Add(StandardBoneTransform);
			}
		}
	}

	if (IsLeftHand)
	{
		LeftHandUseNormalBoneMaterial();
	}
	else
	{
		RightHandUseNormalBoneMaterial();
	}
}

void AHandsVisualizationSwitcher::UpdateHandBoneVisuals(
	UOculusXRHandComponent* Hand,
	UInstancedStaticMeshComponent* BoneInstancedMeshes,
	TArray<FTransform>& SegmentTransforms,
	TArray<FLineSegment>& HandSegmentInfos,
	bool bMeshVisibility, bool bConfidenceIsHigh,
	float HandScale)
{
	auto NumRenderInstances = BoneInstancedMeshes->GetNumRenderInstances();
	if (NumRenderInstances == 0)
	{
		return;
	}

	if (!bConfidenceIsHigh || !bMeshVisibility)
	{
		if (BoneInstancedMeshes->IsVisible())
		{
			BoneInstancedMeshes->SetVisibility(false);
		}
		return;
	}
	else if (!BoneInstancedMeshes->IsVisible())
	{
		BoneInstancedMeshes->SetVisibility(true);
	}

	// a scale of 1 is 1 meter for instanced mesh (because it uses a plane);
	// convert to CM
	float CurrentBoneScale = HandScale * BoneScaleFactor;
	auto InstanceWorldToLocal = BoneInstancedMeshes->GetComponentTransform().Inverse();

	// Get HMD Position; bone segments need to "face" the HMD
	FRotator DeviceRotation;
	FVector DevicePosition;
	UHeadMountedDisplayFunctionLibrary::GetOrientationAndPosition(DeviceRotation, DevicePosition);
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (!IsValid(PlayerPawn))
	{
		return;
	}
	FTransform PlayerTransform = PlayerPawn->GetTransform();
	FVector HDMPosition = PlayerTransform.TransformPosition(DevicePosition);

	for (int32 InstancedIndex = 0; InstancedIndex < NumRenderInstances; InstancedIndex++)
	{
		auto& SegmentTransform = SegmentTransforms[InstancedIndex];
		auto const& HandSegmentInfo = HandSegmentInfos[InstancedIndex];

		FTransform StartTransform = Hand->GetBoneTransform(HandSegmentInfo.StartBoneIndex);
		FTransform EndTransform = Hand->GetBoneTransform(HandSegmentInfo.EndBoneIndex);

		FVector StartPosition = StartTransform.GetLocation();
		FVector EndPosition = EndTransform.GetLocation();
		if (StartPosition.Equals(EndPosition))
		{
			// No need to update this instance, since there's a single bone
			continue;
		}

		FVector AveragePosition = (StartPosition + EndPosition) * 0.5f;
		FVector SegmentLocalPosition(InstanceWorldToLocal.TransformPosition(AveragePosition));

		FVector SegmentForwardVector = EndPosition - StartPosition;
		float SegmentSize = SegmentForwardVector.Size();
		FVector SegmentLocalScale(SegmentSize * BoneScaleFactor,
			CurrentBoneScale, CurrentBoneScale);

		FQuat SegmentStartWorldRotation = StartTransform.GetRotation();
		FQuat SegmentEndWorldRotation = EndTransform.GetRotation();
		FVector StartUp = SegmentStartWorldRotation.GetUpVector();
		FVector EndUp = SegmentEndWorldRotation.GetUpVector();

		SegmentForwardVector /= SegmentSize;
		// make segment's up face HMD
		FVector SegmentWorldUp = (HDMPosition - AveragePosition);
		SegmentWorldUp.Normalize();
		// If up is nearly parallel/anti-parallel with forward, use bone's up instead
		if (fabs(FVector::DotProduct(SegmentForwardVector, SegmentWorldUp) - 1.0f)
			< 0.00001f)
		{
			SegmentWorldUp = (StartUp + EndUp) * 0.5f;
			SegmentWorldUp.Normalize();
		}

		FQuat SegmentWorldRotation = UKismetMathLibrary::MakeRotFromXZ(
			SegmentForwardVector, SegmentWorldUp)
										 .Quaternion();
		FQuat SegmentLocalRotation = InstanceWorldToLocal.TransformRotation(
			SegmentWorldRotation);

		SegmentTransform.SetComponents(SegmentLocalRotation, SegmentLocalPosition,
			SegmentLocalScale);
	}

	BoneInstancedMeshes->BatchUpdateInstancesTransforms(0,
		SegmentTransforms, false, true);
}

void AHandsVisualizationSwitcher::SwitchHandsVisualization()
{
	if (!IsValid(LeftHand) || !IsValid(RightHand) || LeftHandBoneInstancedMeshes->GetInstanceCount() == 0 || RightHandBoneInstancedMeshes->GetInstanceCount() == 0)
	{
		return;
	}

	CurrentVisualMode = (EOculusXRHandsVisualMode)((((int)CurrentVisualMode + 1) % ((int)EOculusXRHandsVisualMode::Both + 1)));
	EnforceCurrentVisualMode();
}

void AHandsVisualizationSwitcher::EnforceCurrentVisualMode()
{
	float ProperAlpha = (CurrentVisualMode == EOculusXRHandsVisualMode::Both) ? HandAlphaWhenBonesVisible : 1.0f;
	LeftHand->SetScalarParameterValueOnMaterials(HandAlphaParamName, ProperAlpha);
	RightHand->SetScalarParameterValueOnMaterials(HandAlphaParamName, ProperAlpha);

	switch (CurrentVisualMode)
	{
		case EOculusXRHandsVisualMode::Mesh:
			LeftHand->SetVisibility(true);
			RightHand->SetVisibility(true);
			ToggleBoneVisuals(false);
			break;
		case EOculusXRHandsVisualMode::Skeleton:
			LeftHand->SetVisibility(false);
			RightHand->SetVisibility(false);
			ToggleBoneVisuals(true);
			break;
		default:
			LeftHand->SetVisibility(true);
			RightHand->SetVisibility(true);
			ToggleBoneVisuals(true);
			break;
	}
}

void AHandsVisualizationSwitcher::ToggleBoneVisuals(bool bAreBoneVisualsEnabled)
{
	bLeftHandBonesVisible = bAreBoneVisualsEnabled;
	bRightHandBonesVisible = bAreBoneVisualsEnabled;
}

void AHandsVisualizationSwitcher::LeftHandUseSystemGestureBoneMaterial()
{
	LeftHandBoneInstancedMeshes->SetMaterial(0, SystemGestureBoneMaterial);
}

void AHandsVisualizationSwitcher::LeftHandUseNormalBoneMaterial()
{
	LeftHandBoneInstancedMeshes->SetMaterial(0, NormalBoneMaterial);
}

void AHandsVisualizationSwitcher::RightHandUseSystemGestureBoneMaterial()
{
	RightHandBoneInstancedMeshes->SetMaterial(0, SystemGestureBoneMaterial);
}

void AHandsVisualizationSwitcher::RightHandUseNormalBoneMaterial()
{
	RightHandBoneInstancedMeshes->SetMaterial(0, NormalBoneMaterial);
}
