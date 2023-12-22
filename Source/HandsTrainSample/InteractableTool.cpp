// Fill out your copyright notice in the Description page of Project Settings.

#include "InteractableTool.h"
#include "ColliderZone.h"
#include "Interactable.h"

AInteractableTool::AInteractableTool()
{
	PrimaryActorTick.bCanEverTick = false;
}

/** Should be overridden. */
void AInteractableTool::Initialize_Implementation(UOculusXRHandComponent* HandComponent)
{
}

/** Should be overridden. */
EToolInputState AInteractableTool::GetCurrInputState_Implementation()
{
	return EToolInputState::Inactive;
}

/** Should be overridden. */
void AInteractableTool::SetVisualEnableState_Implementation(bool NewEnableState)
{
}

/** Should be overridden. */
bool AInteractableTool::GetVisualEnableState_Implementation()
{
	return false;
}

/** Should be overridden. */
void AInteractableTool::RefreshCurrentIntersectingObjects_Implementation()
{
}

/** Should be overridden. */
void AInteractableTool::FocusOnInteractable_Implementation(
	AInteractable* FocusedInteractable, UColliderZone* ColliderZone)
{
}

/** Should be overridden. */
void AInteractableTool::DeFocus_Implementation()
{
}

/** Should be overridden. */
EInteractableToolTags AInteractableTool::GetToolTags_Implementation()
{
	return EInteractableToolTags::All;
}

FCollisionInfoKeyValuePair AInteractableTool::
	GetFirstCurrentCollisionInfoClosestToPosition(FVector WorldPosition)
{
	FCollisionInfoKeyValuePair FirstCollisionInfo;
	// initial value is -1 -- distance squared is always positive,
	// so -1.0 means not initialized
	float ClosestDistanceSqr = -1.0f;
	for (auto& Elem : CurrInteractableToCollisionInfos)
	{
		auto CurrentInteractable = Elem.Key;
		float CurrentDistanceSqr = FVector::DistSquared(
			WorldPosition, CurrentInteractable->GetActorLocation());
		if (ClosestDistanceSqr < 0.0f || CurrentDistanceSqr < ClosestDistanceSqr)
		{
			ClosestDistanceSqr = CurrentDistanceSqr;
			FirstCollisionInfo.Interactable = Elem.Key;
			FirstCollisionInfo.CollisionInfo = Elem.Value;
		}
		break;
	}
	return FirstCollisionInfo;
}

void AInteractableTool::UpdateCurrentCollisionsMapBasedOnDepth()
{
	CurrInteractableToCollisionInfos.Empty();
	for (auto& LatestCollisionInfo : CurrentIntersectingObjects)
	{
		AInteractable* CurrInteractable =
			LatestCollisionInfo.InteractableCollider->ParentInteractable;
		EInteractableCollisionDepth CurrDepth =
			LatestCollisionInfo.CollisionDepth;

		if (!CurrInteractableToCollisionInfos.Contains(CurrInteractable))
		{
			CurrInteractableToCollisionInfos.Add(CurrInteractable,
				LatestCollisionInfo);
		}
		else
		{
			FInteractableCollisionInfo& CollisionInfoFromMap =
				CurrInteractableToCollisionInfos[CurrInteractable];
			if (CollisionInfoFromMap.CollisionDepth < CurrDepth)
			{
				CollisionInfoFromMap.InteractableCollider =
					LatestCollisionInfo.InteractableCollider;
				CollisionInfoFromMap.CollisionDepth = CurrDepth;
			}
		}
	}
}

void AInteractableTool::UpdateCurrentCollisionsMap(AInteractable* CurrInteractable,
	FInteractableCollisionInfo CollisionInfo)
{
	if (CurrInteractableToCollisionInfos.Contains(CurrInteractable))
	{
		CurrInteractableToCollisionInfos[CurrInteractable] =
			CollisionInfo;
	}
}

void AInteractableTool::SyncLatestCollisionDataWithInteractables()
{
	AddedInteractables.Empty();
	RemovedInteractables.Empty();
	RemainingInteractables.Empty();

	for (auto& Elem : CurrInteractableToCollisionInfos)
	{
		AInteractable* Key = Elem.Key;
		bool IsNewItem = !PrevInteractableToCollisionInfos.Contains(Key);
		if (IsNewItem)
		{
			AddedInteractables.Add(Key);
		}
		else
		{
			RemainingInteractables.Add(Key);
		}
	}

	for (auto& Elem : PrevInteractableToCollisionInfos)
	{
		AInteractable* Key = Elem.Key;
		bool RemovedItem = !CurrInteractableToCollisionInfos.Contains(Key);
		if (RemovedItem)
		{
			RemovedInteractables.Add(Key);
		}
	}

	// Tell removed interactables that we have left them
	for (AInteractable* RemovedInteractable : RemovedInteractables)
	{
		if (IsValid(RemovedInteractable))
		{
			RemovedInteractable->UpdateCollisionDepth(
				this,
				PrevInteractableToCollisionInfos[RemovedInteractable].CollisionDepth,
				EInteractableCollisionDepth::None);
		}
	}

	// Tell added interactables that we are now interacting with the tool
	for (AInteractable* AddedInteractable : AddedInteractables)
	{
		if (IsValid(AddedInteractable))
		{
			FInteractableCollisionInfo AddedCollisionInfo =
				CurrInteractableToCollisionInfos[AddedInteractable];
			EInteractableCollisionDepth CollisionDepth =
				AddedCollisionInfo.CollisionDepth;
			AddedInteractable->UpdateCollisionDepth(
				this,
				EInteractableCollisionDepth::None,
				CollisionDepth);
		}
	}

	// Remaining interactables must now updated depth
	for (AInteractable* RemainingInteractable : RemainingInteractables)
	{
		if (IsValid(RemainingInteractable))
		{
			EInteractableCollisionDepth OldCollisionDepth = PrevInteractableToCollisionInfos[RemainingInteractable].CollisionDepth;
			FInteractableCollisionInfo NewCollisionInfo =
				CurrInteractableToCollisionInfos[RemainingInteractable];
			EInteractableCollisionDepth NewCollisionDepth = NewCollisionInfo.CollisionDepth;
			;
			RemainingInteractable->UpdateCollisionDepth(this, OldCollisionDepth,
				NewCollisionDepth);
		}
	}

	PrevInteractableToCollisionInfos.Empty();
	PrevInteractableToCollisionInfos = CurrInteractableToCollisionInfos;
}

void AInteractableTool::BeginDestroy()
{
	Super::BeginDestroy();
	OnInteractableToolDeathEvent.Broadcast(this);
}
