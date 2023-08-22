// Fill out your copyright notice in the Description page of Project Settings.

#include "RayTool.h"
#include "Interactable.h"
#include "Math/UnrealMathUtility.h"
#include "OculusXRHandComponent.h"
#include "OculusXRInputFunctionLibrary.h"
#include "RayToolViewHelper.h"
#include "ColliderZone.h"
#include "Kismet/GameplayStatics.h"
#include "Components/InstancedStaticMeshComponent.h"
#include <cmath>

const float ARayTool::MinimumRaycastDistance = 80.0f;
const float ARayTool::ColliderRadius = 1.0f;

ARayTool::ARayTool()
{
	PrimaryActorTick.bCanEverTick = true;

	RootSceneComponent = CreateDefaultSubobject<USceneComponent>(FName(TEXT("Root")));
	RootComponent = RootSceneComponent;

	TargetMesh = CreateDefaultSubobject<UStaticMeshComponent>(
		FName(TEXT("TargetMesh")));
	TargetMesh->SetupAttachment(RootComponent);
	TargetMesh->SetMobility(EComponentMobility::Movable);

	RayMesh = CreateDefaultSubobject<UInstancedStaticMeshComponent>(
		FName(TEXT("RayMesh")));
	RayMesh->SetupAttachment(RootComponent);
	RayMesh->SetMobility(EComponentMobility::Movable);

	RayToolViewHelperComp = CreateDefaultSubobject<URayToolViewHelper>(
		FName(TEXT("RayToolViewHelperComp")));

	ConeAngleDegrees = 20.0f;
	FarFieldMaxDistance = 500.0f;
	bIsInitialized = false;
}

EInteractableToolTags ARayTool::GetToolTags_Implementation()
{
	return EInteractableToolTags::Ray;
}

void ARayTool::BeginPlay()
{
	Super::BeginPlay();

	/**
	 * Tool only releases something if the angle goes beyond a
	 * certain range. If we used an exact angle, the ray would
	 * focus an object then release it over and over again.
	 */
	ConeAngleReleaseDegrees = ConeAngleDegrees * 1.2f;
}

void ARayTool::Initialize_Implementation(UOculusXRHandComponent* HandComponent)
{
	Hand = HandComponent;
	RayToolViewHelperComp->Initialize(this, TargetMesh, RayMesh);
	bIsInitialized = true;

	EnableInput(GetWorld()->GetFirstPlayerController());

	InputComponent->BindAxis(IsRightHandedTool ? FName(TEXT("OculusRHandIndexPinchStrength")) : FName(TEXT("OculusLHandIndexPinchStrength")));
}

void ARayTool::SetVisualEnableState_Implementation(bool NewVisualEnableState)
{
	RayToolViewHelperComp->SetEnableState(NewVisualEnableState);
}

bool ARayTool::GetVisualEnableState_Implementation()
{
	return RayToolViewHelperComp->GetEnableState();
}

EToolInputState ARayTool::GetCurrInputState_Implementation()
{
	if (CurrPinchState.PinchDownOnFocusedObject())
	{
		return EToolInputState::PrimaryInputDown;
	}
	if (CurrPinchState.PinchSteadyOnFocusedObject())
	{
		return EToolInputState::PrimaryInputDownStay;
	}
	if (CurrPinchState.GetPinchUpAndDownOnFocusedObject())
	{
		return EToolInputState::PrimaryInputUp;
	}

	return EToolInputState::Inactive;
}

void ARayTool::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!IsValid(Hand) || !bIsInitialized || !UOculusXRInputFunctionLibrary::IsPointerPoseValid(Hand->SkeletonType))
	{
		return;
	}

	Hand->SetRenderCustomDepth(true);
	FTransform PointerPoseTransform =
		UOculusXRInputFunctionLibrary::GetPointerPose(Hand->SkeletonType);
	FVector CurrentPosition = PointerPoseTransform.GetLocation();
	APawn* MainPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	CurrentPosition += MainPawn->GetActorLocation();
	SetActorLocation(CurrentPosition);
	SetActorRotation(PointerPoseTransform.GetRotation());

	auto PrevPosition = InteractionPosition;
	CalculatedToolVelocity = (CurrentPosition - PrevPosition) / DeltaTime;
	InteractionPosition = CurrentPosition;
	float PinchStrength = GetInputAxisValue(IsRightHandedTool ? FName(TEXT("OculusRHandIndexPinchStrength")) : FName(TEXT("OculusLHandIndexPinchStrength")));

	CurrPinchState.UpdateState(PinchStrength, FocusedInteractable, IsRightHandedTool);
	RayToolViewHelperComp->SetToolActiveState(CurrPinchState.PinchSteadyOnFocusedObject() || CurrPinchState.PinchDownOnFocusedObject());
}

void ARayTool::RefreshCurrentIntersectingObjects_Implementation()
{
	if (!bIsInitialized)
	{
		return;
	}

	// If already focused on something, keep it until the angle between
	// our forward direction and object vector becomes too large
	if (IsValid(CurrInteractableRaycastedAgainst) && HasRayReleasedInteractable(CurrInteractableRaycastedAgainst))
	{
		CurrInteractableRaycastedAgainst = nullptr;
	}

	// Find target interactable if we haven't found one before
	if (!IsValid(CurrInteractableRaycastedAgainst))
	{
		CurrentIntersectingObjects.Empty();
		CurrInteractableRaycastedAgainst = FindTargetInteractable();

		// Found one? Query collision zones.
		if (IsValid(CurrInteractableRaycastedAgainst))
		{
			auto TargetHitPoint = CurrInteractableRaycastedAgainst->GetActorLocation();
			UWorld* TheWorld = this->GetWorld();
			TArray<FOverlapResult> SphereOverlapResults;
			FCollisionShape SphereShape = FCollisionShape::MakeSphere(ColliderRadius);

			// See DefaultEngine.ini for a mapping between this enum and the custom
			// traces set up in Project Settings->Engine->Collision
			TheWorld->OverlapMultiByChannel(SphereOverlapResults,
				TargetHitPoint,
				GetActorRotation().Quaternion(),
				ECollisionChannel::ECC_GameTraceChannel1,
				SphereShape);

			// Find all components encountered and focus only one the ones
			// belonging to the targe telement
			for (auto& CurrentOverlap : SphereOverlapResults)
			{
				UColliderZone* HitColliderZone =
					Cast<UColliderZone>(CurrentOverlap.Component.Get());
				if (!IsValid(HitColliderZone))
				{
					continue;
				}

				AInteractable* CurrInteractable = HitColliderZone->ParentInteractable;
				if (!IsValid(CurrInteractable) || CurrInteractable != CurrInteractableRaycastedAgainst)
				{
					continue;
				}

				FInteractableCollisionInfo CollisionInfo(HitColliderZone,
					HitColliderZone->GetCollisionDepth(), this);
				CurrentIntersectingObjects.Add(CollisionInfo);
			}

			// Clear interactable casted against if no colliders found on it
			if (CurrentIntersectingObjects.Num() == 0)
			{
				CurrInteractableRaycastedAgainst = nullptr;
			}
		}
	}
}

void ARayTool::FocusOnInteractable_Implementation(
	AInteractable* NewFocusedInteractable, UColliderZone* ColliderZone)
{
	RayToolViewHelperComp->SetFocusedInteractable(NewFocusedInteractable);
	this->FocusedInteractable = NewFocusedInteractable;
}

void ARayTool::DeFocus_Implementation()
{
	RayToolViewHelperComp->SetFocusedInteractable(nullptr);
	this->FocusedInteractable = nullptr;
}

FVector ARayTool::GetRaycastOrigin()
{
	return GetActorLocation() + MinimumRaycastDistance * GetActorForwardVector();
}

bool ARayTool::HasRayReleasedInteractable(AInteractable* NewFocusedInteractable)
{
	auto ToolPosition = GetActorLocation();
	auto ToolForwardDirection = GetActorForwardVector();
	float HysteresisDotThreshold = cos(FMath::DegreesToRadians(
		ConeAngleReleaseDegrees));
	auto VectorToFocusedObject = NewFocusedInteractable->GetActorLocation()
		- ToolPosition;
	VectorToFocusedObject.Normalize();

	float DotProdLineOfSight = FVector::DotProduct(VectorToFocusedObject,
		ToolForwardDirection);
	// If dot product is smaller, that we are getting closer to perpendicular
	// So that means that angle has become too large.
	return DotProdLineOfSight < HysteresisDotThreshold;
}

AInteractable* ARayTool::FindTargetInteractable()
{
	auto RayOrigin = GetRaycastOrigin();
	auto RayDirection = GetActorForwardVector();
	AInteractable* TargetInteractable = nullptr;

	TargetInteractable = FindPrimaryRaycastHit(RayOrigin,
		RayDirection);

	// Try secondary cone test if primary ray cast above failed.
	if (!IsValid(TargetInteractable))
	{
		TargetInteractable = FindInteractableViaConeTest(RayOrigin,
			RayDirection);
	}
	else
	{
	}

	return TargetInteractable;
}

AInteractable* ARayTool::FindPrimaryRaycastHit(FVector RayOrigin, FVector RayDirection)
{
	AInteractable* InteractableCastedAgainst = nullptr;
	UWorld* TheWorld = this->GetWorld();
	TArray<FHitResult> Hits;

	// See DefaultEngine.ini for a mapping between this enum and the custom
	// traces set up in Project Settings->Engine->Collision
	TheWorld->LineTraceMultiByChannel(Hits, RayOrigin,
		RayOrigin + RayDirection * 10000.0f,
		ECollisionChannel::ECC_GameTraceChannel1);

	float MinDistance = 0.0f;
	for (auto& CurrentHit : Hits)
	{
		UColliderZone* HitColliderZone = Cast<UColliderZone>(CurrentHit.Component.Get());
		if (!IsValid(HitColliderZone))
		{
			continue;
		}

		// Only consider an interactable that is compatible with
		// tool
		AInteractable* CurrInteractable = HitColliderZone->ParentInteractable;
		if (!IsValid(CurrInteractable) || (CurrInteractable->GetValidToolTagsMask() & (int)GetToolTags()) == 0)
		{
			continue;
		}

		auto VectorToInteractable = CurrInteractable->GetActorLocation() - RayOrigin;
		float DistanceToInteractable = VectorToInteractable.Size();
		if (InteractableCastedAgainst == nullptr || DistanceToInteractable < MinDistance)
		{
			InteractableCastedAgainst = CurrInteractable;
			MinDistance = DistanceToInteractable;
		}
	}

	return InteractableCastedAgainst;
}

AInteractable* ARayTool::FindInteractableViaConeTest(FVector RayOrigin,
	FVector RayDirection)
{
	AInteractable* TargetInteractable = nullptr;

	float MinDistance = 0.0f;
	float MinDotProductThreshold = cos(FMath::DegreesToRadians(
		ConeAngleDegrees));
	// cone extends from center line, where angle is split between
	// top and bottom half
	float HalfAngle = FMath::DegreesToRadians(ConeAngleDegrees * 0.5f);
	float ConeRadius = tan(HalfAngle) * FarFieldMaxDistance;

	UWorld* TheWorld = this->GetWorld();
	TArray<FOverlapResult> Results;
	FCollisionShape BoxShape = FCollisionShape::MakeBox(
		FVector(FarFieldMaxDistance * 0.5,
			ConeRadius, ConeRadius));

	// See DefaultEngine.ini for a mapping between this enum and the custom
	// traces set up in Project Settings->Engine->Collision
	TheWorld->OverlapMultiByChannel(Results,
		RayOrigin + RayDirection * FarFieldMaxDistance * 0.5f, // center
		GetActorRotation().Quaternion(),
		ECollisionChannel::ECC_GameTraceChannel1,
		BoxShape);

	for (auto& Result : Results)
	{
		UColliderZone* HitColliderZone = Cast<UColliderZone>(Result.Component.Get());
		if (!IsValid(HitColliderZone))
		{
			continue;
		}

		AInteractable* InteractableComponent = HitColliderZone->ParentInteractable;
		if (!IsValid(InteractableComponent) || (InteractableComponent->GetValidToolTagsMask() & (int)GetToolTags()) == 0)
		{
			continue;
		}

		auto VectorToInteractable = InteractableComponent->GetActorLocation()
			- RayOrigin;
		auto DistanceToInteractable = VectorToInteractable.Size();
		VectorToInteractable /= DistanceToInteractable;
		float DotProduct = FVector::DotProduct(VectorToInteractable, RayDirection);
		// Must be inside cone. Bail if too small.
		if (DotProduct < MinDotProductThreshold)
		{
			continue;
		}

		if (TargetInteractable == nullptr || DistanceToInteractable < MinDistance)
		{
			TargetInteractable = InteractableComponent;
			MinDistance = DistanceToInteractable;
		}
	}

	return TargetInteractable;
}
