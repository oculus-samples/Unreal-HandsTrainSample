/*
Copyright (c) Meta Platforms, Inc. and affiliates.
All rights reserved.

This source code is licensed under the license found in the
LICENSE file in the root directory of this source tree.
*/

#include "FingerTipPokeTool.h"
#include "OculusXRHandComponent.h"
#include "BoneCapsuleTriggerLogic.h"
#include "ColliderZone.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include <cstring>
#include <vector>

AFingerTipPokeTool::AFingerTipPokeTool()
{
	PrimaryActorTick.bCanEverTick = true;

	RootSceneComponent = CreateDefaultSubobject<USceneComponent>(FName(TEXT("Root")));
	RootComponent = RootSceneComponent;

	TargetMesh = CreateDefaultSubobject<UStaticMeshComponent>(
		FName(TEXT("TargetMesh")));
	TargetMesh->SetupAttachment(RootComponent);
	TargetMesh->SetMobility(EComponentMobility::Movable);

	SphereRadius = TargetMesh->GetRelativeScale3D()[0] * 0.5f;
	LastScale = 1.0f;
	CurrentVelocityFrame = 0;
	SampledMaxFramesAlready = false;
	bIsInitialized = false;
}

void AFingerTipPokeTool::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bIsInitialized || !IsValid(CapsuleToTrack.Capsule))
	{
		return;
	}

	auto CapsuleObject = CapsuleToTrack.Capsule;
	auto HandType = IsRightHandedTool ? EOculusXRHandType::HandRight : EOculusXRHandType::HandLeft;
	float CurrentHandScale = UOculusXRInputFunctionLibrary::GetHandScale(HandType);

	const FTransform& CapsuleTransform = CapsuleObject->GetComponentTransform();
	FQuat CapsuleRotation = CapsuleTransform.GetRotation();
	FVector CapsuleDirection = CapsuleRotation.GetUpVector();
	// first find position of capsule's tip
	FVector CapsuleTipPosition = CapsuleObject->GetComponentLocation() + CapsuleObject->GetScaledCapsuleHalfHeight() * CapsuleDirection;
	// then push tool's center away from tip. the center should be
	// radius units away from tip
	FVector ToolSphereRadiusOffsetFromTip = -CurrentHandScale * SphereRadius
		* CapsuleDirection;

	FVector ToolPosition = CapsuleTipPosition + ToolSphereRadiusOffsetFromTip;
	SetActorLocation(ToolPosition);
	SetActorRotation(UKismetMathLibrary::MakeRotFromXZ(
		CapsuleDirection, CapsuleRotation.GetRightVector())
						 .Quaternion());

	InteractionPosition = CapsuleTipPosition;

	UpdateAverageVelocity(DeltaTime);
	CheckAndUpdateScale();
}

void AFingerTipPokeTool::Initialize_Implementation(UOculusXRHandComponent* HandComponent)
{
	memset(VelocityFrames, 0, NumVelocityFrames * sizeof(FVector));

	BoneToTestCollisions = EOculusXRBone::Pinky_3;
	switch (FingerToFollow)
	{
		case EHandFinger::Thumb:
			BoneToTestCollisions = EOculusXRBone::Thumb_3;
			break;
		case EHandFinger::Index:
			BoneToTestCollisions = EOculusXRBone::Index_3;
			break;
		case EHandFinger::Middle:
			BoneToTestCollisions = EOculusXRBone::Middle_3;
			break;
		case EHandFinger::Ring:
			BoneToTestCollisions = EOculusXRBone::Ring_3;
			break;
		default:
			BoneToTestCollisions = EOculusXRBone::Pinky_3;
			break;
	}

	this->HandToTrack = HandComponent;
	TArray<FOculusXRCapsuleCollider>& HandCapsules =
		HandComponent->CollisionCapsules;
	// look for proper capsules and also apply proper collision filtering
	for (auto CapsuleCollider : HandCapsules)
	{
		auto CapsuleComp = CapsuleCollider.Capsule;
		CapsuleComp->CanCharacterStepUpOn = ECanBeCharacterBase::ECB_No;
		CapsuleComp->SetCanEverAffectNavigation(false);
		// See DefaultEngine.ini for a mapping between this enum and the custom
		// traces/objects set up in Project Settings->Engine->Collision.
		// the interactable collision zones only accept a specific channel,
		// and so should we
		CapsuleComp->SetCollisionObjectType(ECollisionChannel::ECC_EngineTraceChannel2);

		if (CapsuleCollider.BoneId == BoneToTestCollisions)
		{
			CollisionCapsulesForBone.Add(CapsuleCollider);
			CapsuleComp->SetGenerateOverlapEvents(true);
			CapsuleComp->SetCollisionResponseToAllChannels(
				ECollisionResponse::ECR_Ignore);
			CapsuleComp->SetCollisionResponseToChannel(
				ECollisionChannel::ECC_GameTraceChannel3,
				ECollisionResponse::ECR_Overlap);
		}
		else
		{
			CapsuleComp->SetGenerateOverlapEvents(false);
			CapsuleComp->SetCollisionResponseToAllChannels(
				ECollisionResponse::ECR_Ignore);
		}
	}

	TriggerLogic = GetWorld()->SpawnActor<ABoneCapsuleTriggerLogic>(
		ABoneCapsuleTriggerLogic::StaticClass(),
		GetActorLocation(),
		GetActorRotation());
	TriggerLogic->InitializeOverlapEvents(CollisionCapsulesForBone,
		this->GetToolTags_Implementation());

	if (CollisionCapsulesForBone.Num() > 0)
	{
		CapsuleToTrack = CollisionCapsulesForBone[0];
	}

	SetVisualEnableState_Implementation(true);
	bIsInitialized = true;
}

void AFingerTipPokeTool::SetVisualEnableState_Implementation(bool NewVisualEnableState)
{
	TargetMesh->SetVisibility(NewVisualEnableState);
}

bool AFingerTipPokeTool::GetVisualEnableState_Implementation()
{
	return TargetMesh->IsVisible();
}

EInteractableToolTags AFingerTipPokeTool::GetToolTags_Implementation()
{
	return EInteractableToolTags::Poke;
}

void AFingerTipPokeTool::RefreshCurrentIntersectingObjects_Implementation()
{
	CurrentIntersectingObjects.Empty();
	TSet<UColliderZone*>& CollidersTouching = TriggerLogic->CollidersTouching;
	for (UColliderZone* ColliderZone : CollidersTouching)
	{
		CurrentIntersectingObjects.Add(FInteractableCollisionInfo(
			ColliderZone, ColliderZone->GetCollisionDepth(),
			this));
	}
}

void AFingerTipPokeTool::UpdateAverageVelocity(float DeltaTime)
{
	FVector CurrentPosition = GetActorLocation();
	FVector CurrentVelocityVec = (CurrentPosition - LastPosition) / DeltaTime;
	LastPosition = CurrentPosition;
	VelocityFrames[CurrentVelocityFrame] = CurrentVelocityVec;
	// if sampled more than allowed, loop back toward the beginning
	CurrentVelocityFrame = (CurrentVelocityFrame + 1) % NumVelocityFrames;

	CalculatedToolVelocity = FVector::ZeroVector;
	/**
	 * Edge case; when we first start up, we will have only
	 * sampled less than the max frames. So only compute the
	 * average over that subset. After that, the max frame samples
	 * will act like and array that loops back toward the
	 * beginning
	 */
	if (!SampledMaxFramesAlready && CurrentVelocityFrame == NumVelocityFrames - 1)
	{
		SampledMaxFramesAlready = true;
	}

	uint32 NumFramesToSample = SampledMaxFramesAlready ? NumVelocityFrames
													   : CurrentVelocityFrame + 1;
	for (uint32 FrameIndex = 0; FrameIndex < NumFramesToSample;
		 FrameIndex++)
	{
		CalculatedToolVelocity += VelocityFrames[FrameIndex];
	}
	CalculatedToolVelocity /= NumFramesToSample;
}

void AFingerTipPokeTool::CheckAndUpdateScale()
{
	float CurrentHandScale = UOculusXRInputFunctionLibrary::GetHandScale(
		IsRightHandedTool ? EOculusXRHandType::HandRight : EOculusXRHandType::HandLeft);
	if (fabs(CurrentHandScale - LastScale) > 0.001f)
	{
		SetActorRelativeScale3D(FVector(CurrentHandScale, CurrentHandScale,
			CurrentHandScale));
		LastScale = CurrentHandScale;
	}
}
