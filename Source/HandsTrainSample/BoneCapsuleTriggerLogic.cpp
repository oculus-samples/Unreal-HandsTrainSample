// Fill out your copyright notice in the Description page of Project Settings.

#include "BoneCapsuleTriggerLogic.h"
#include "ColliderZone.h"
#include "ButtonTriggerZone.h"
#include "Interactable.h"

ABoneCapsuleTriggerLogic::ABoneCapsuleTriggerLogic()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ABoneCapsuleTriggerLogic::InitializeOverlapEvents(
	TArray<FOculusXRCapsuleCollider>& Capsules,
	EInteractableToolTags ToolTagsToSet)
{
	BoneCollisionCapsules = Capsules;
	ToolTags = ToolTagsToSet;

	for (auto& CollisionCapsule : BoneCollisionCapsules)
	{
		auto Capsule = CollisionCapsule.Capsule;
		if (IsValid(Capsule))
		{
			Capsule->OnComponentBeginOverlap.AddDynamic(this,
				&ABoneCapsuleTriggerLogic::OnOverlapBegin);
			Capsule->OnComponentEndOverlap.AddDynamic(this,
				&ABoneCapsuleTriggerLogic::OnOverlapEnd);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Capsule for bone %d is not valid!"),
				CollisionCapsule.BoneId);
		}
	}
}

void ABoneCapsuleTriggerLogic::OnOverlapBegin(class UPrimitiveComponent* OverlappedComp,
	class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult)
{
	if (!IsValid(OtherActor) || !IsValid(OtherComp))
	{
		return;
	}
	UColliderZone* ColliderZoneOverlapped = Cast<UColliderZone>(OtherComp);

	if (ColliderZoneOverlapped != nullptr && (ColliderZoneOverlapped->ParentInteractable->GetValidToolTagsMask() & (int)ToolTags) != 0)
	{
		CollidersTouching.Add(ColliderZoneOverlapped);
	}
}

void ABoneCapsuleTriggerLogic::OnOverlapEnd(class UPrimitiveComponent* OverlappedComp,
	class AActor* OtherActor, class UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{
	if (!IsValid(OtherActor) || !IsValid(OtherComp))
	{
		return;
	}
	UColliderZone* ColliderZoneOverlapped = Cast<UColliderZone>(OtherComp);

	if (ColliderZoneOverlapped != nullptr && (ColliderZoneOverlapped->ParentInteractable->GetValidToolTagsMask() & (int)ToolTags) != 0)
	{
		CollidersTouching.Remove(ColliderZoneOverlapped);
	}
}

void ABoneCapsuleTriggerLogic::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	CleanUpDeadColliders();
}

void ABoneCapsuleTriggerLogic::CleanUpDeadColliders()
{
	ElementsToCleanUp.Empty();
	for (auto ColliderTouching : CollidersTouching)
	{
		if (!IsValid(ColliderTouching))
		{
			ElementsToCleanUp.Add(ColliderTouching);
		}
	}

	for (auto ColliderToRemove : ElementsToCleanUp)
	{
		CollidersTouching.Remove(ColliderToRemove);
	}
}
