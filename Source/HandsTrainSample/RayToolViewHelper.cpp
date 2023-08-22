// Fill out your copyright notice in the Description page of Project Settings.

#include "RayToolViewHelper.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SplineMeshComponent.h"
#include "Math/UnrealMathUtility.h"
#include "InteractableTool.h"
#include "Interactable.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

const float URayToolViewHelper::DefaultRayCastDistance = 300.0f;
const FName URayToolViewHelper::ColorPropertyName = "TintColor";
const FName URayToolViewHelper::OpacityPropertName = "Opacity";
// convert to CM (1 scale unit = 100 cm for plane mesh)
const float URayToolViewHelper::RayScaleFactor = 0.01f;

URayToolViewHelper::URayToolViewHelper()
{
	PrimaryComponentTick.bCanEverTick = true;

	NormalColor = FColor(197, 197, 197, 255);
	HighlightColor = FColor(230, 230, 230, 255);
}

void URayToolViewHelper::BeginPlay()
{
	Super::BeginPlay();
	bToolActivateState = false;
}

void URayToolViewHelper::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!IsValid(InteractableTool) || !IsValid(TargetMesh) || !IsValid(RayMesh))
	{
		return;
	}

	auto ToolPosition = InteractableTool->GetActorLocation();
	auto ToolForward = InteractableTool->GetActorForwardVector();

	FVector TargetPosition = IsValid(FocusedInteractable) ? FocusedInteractable->GetActorLocation()
														  : ToolPosition + ToolForward * DefaultRayCastDistance;
	FVector TargetVector = TargetPosition - ToolPosition;
	float TargetDistance = TargetVector.Size();

	UpdateRayMesh(ToolPosition, ToolForward, TargetPosition, TargetDistance);

	TargetMesh->SetWorldLocation(TargetPosition);
}

void URayToolViewHelper::UpdateRayMesh(FVector ToolPosition, FVector ToolForward,
	FVector TargetPosition, float TargetDistance)
{
	auto P0 = ToolPosition;
	// make points in between based on my forward as opposed to targetvector
	// this way the curve "bends" toward to target
	auto P1 = ToolPosition + ToolForward * TargetDistance * 0.3333333f;
	auto P2 = ToolPosition + ToolForward * TargetDistance * 0.6666667f;
	auto P3 = TargetPosition;

	auto InstanceWorldToLocal = RayMesh->GetComponentTransform().Inverse();
	for (uint32 PntIndex = 0; PntIndex < NumRayLinePositions; PntIndex++)
	{
		FVector CurrentRayPoint = GetPointOnBezierCurve(P0, P1, P2, P3, (float)PntIndex / 25.0f);

		RayPositions[PntIndex] = CurrentRayPoint;
		// segment index at second point
		if (PntIndex >= 1)
		{
			uint32 SegmentIndex = PntIndex - 1;
			FVector const& RayPointBefore = RayPositions[SegmentIndex];
			FTransform& SegmentTransform = RayMeshSegmentTransforms[SegmentIndex];

			FVector SegmentVector = CurrentRayPoint - RayPointBefore;
			float SegmentSize = SegmentVector.Size();

			FVector SegmentLocalScale(SegmentSize * RayScaleFactor,
				0.2f * RayScaleFactor, 0.2f * RayScaleFactor);
			FVector AveragePosition = (RayPointBefore + CurrentRayPoint) * 0.5f;
			FVector SegmentLocalPosition(InstanceWorldToLocal.TransformPosition(AveragePosition));

			SegmentVector /= SegmentSize;
			// Segment uses default up
			FVector SegmentWorldUp(0.0f, 0.0f, 1.0f);
			SegmentWorldUp.Normalize();
			// If up is nearly parallel/anti-parallel with forward, use worl forward instead
			if (fabs(FVector::DotProduct(SegmentVector, SegmentWorldUp) - 1.0f)
				< 0.00001f)
			{
				SegmentWorldUp = FVector(1.0f, 0.0f, 0.0f);
			}

			FQuat SegmentWorldRotation = UKismetMathLibrary::MakeRotFromXZ(
				SegmentVector, SegmentWorldUp)
											 .Quaternion();
			FQuat SegmentLocalRotation = InstanceWorldToLocal.TransformRotation(
				SegmentWorldRotation);

			SegmentTransform.SetComponents(SegmentLocalRotation, SegmentLocalPosition,
				SegmentLocalScale);
		}
	}
	RayMesh->BatchUpdateInstancesTransforms(0, RayMeshSegmentTransforms, false, true);
}

FVector URayToolViewHelper::GetPointOnBezierCurve(FVector P0, FVector P1, FVector P2,
	FVector P3, float T)
{
	T = FMath::Clamp(T, 0.0f, 1.0f);
	float OneMinusT = 1.0f - T;
	float OneMinusTSqr = OneMinusT * OneMinusT;
	float TSqr = T * T;
	return OneMinusT * OneMinusTSqr * P0 + 3.0f * OneMinusTSqr * T * P1 + 3.0f * OneMinusT * TSqr * P2 + T * TSqr * P3;
}

void URayToolViewHelper::Initialize(AInteractableTool* Tool,
	UStaticMeshComponent* NewTargetMesh,
	UInstancedStaticMeshComponent* NewRayMesh)
{
	InteractableTool = Tool;
	TargetMesh = NewTargetMesh;
	RayMesh = NewRayMesh;

	if (IsValid(RayMesh))
	{
		RayMesh->SetVectorParameterValueOnMaterials(ColorPropertyName,
			FVector((float)NormalColor.R / 255.0f, (float)NormalColor.G / 255.0f, (float)NormalColor.B / 255.0f));
		RayMesh->SetScalarParameterValueOnMaterials(OpacityPropertName,
			(float)NormalColor.A / 255.0f);
		FTransform StandardTransform(FQuat::Identity, FVector::ZeroVector,
			FVector(0.01f, RayScaleFactor, 0.01f));
		for (uint32 SegmentIndex = 0; SegmentIndex < NumRayLinePositions; SegmentIndex++)
		{
			RayPositions.Add(FVector(0.0f, 0.0f, 0.0f));

			// if there are N positions, there are N-1 segments. start at index 1
			if (SegmentIndex >= 1)
			{
				RayMesh->AddInstance(StandardTransform);
				RayMeshSegmentTransforms.Add(StandardTransform);
			}
		}
	}
}

bool URayToolViewHelper::GetEnableState()
{
	if (!IsValid(RayMesh))
	{
		return false;
	}
	return RayMesh->IsVisible();
}

void URayToolViewHelper::SetEnableState(bool bIsEnabled)
{
	if (!IsValid(TargetMesh) || !IsValid(RayMesh))
	{
		return;
	}
	TargetMesh->SetVisibility(bIsEnabled);
	RayMesh->SetVisibility(bIsEnabled);
}

bool URayToolViewHelper::GetToolActiveState()
{
	return bToolActivateState;
}

void URayToolViewHelper::SetToolActiveState(bool bNewActiveState)
{
	bToolActivateState = bNewActiveState;
	RayMesh->SetVectorParameterValueOnMaterials(ColorPropertyName,
		bToolActivateState ? FVector(HighlightColor.R / 255.0f, HighlightColor.G / 255.0f, HighlightColor.B / 255.0f)
						   : FVector(NormalColor.R / 255.0f, NormalColor.G / 255.0f, NormalColor.B / 255.0f));
	RayMesh->SetScalarParameterValueOnMaterials(OpacityPropertName,
		bToolActivateState ? HighlightColor.A / 255.0f : NormalColor.A / 255.0f);
}

void URayToolViewHelper::SetFocusedInteractable(AInteractable* NewFocusedInteractable)
{
	if (!IsValid(NewFocusedInteractable))
	{
		FocusedInteractable = nullptr;
	}
	else
	{
		FocusedInteractable = NewFocusedInteractable;
	}
}
