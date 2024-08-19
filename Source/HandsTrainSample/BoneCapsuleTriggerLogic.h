/*
Copyright (c) Meta Platforms, Inc. and affiliates.
All rights reserved.

This source code is licensed under the license found in the
LICENSE file in the root directory of this source tree.
*/

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InteractableEnums.h"
#include "OculusXRInputFunctionLibrary.h"
#include "BoneCapsuleTriggerLogic.generated.h"

class UColliderZone;

UCLASS()
class HANDSTRAINSAMPLE_API ABoneCapsuleTriggerLogic : public AActor
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Properties")
	TSet<UColliderZone*> CollidersTouching;

	ABoneCapsuleTriggerLogic();

	UFUNCTION(BlueprintCallable)
	void InitializeOverlapEvents(TArray<FOculusXRCapsuleCollider>& BoneCollisionCapsules,
		EInteractableToolTags ToolTagsToSet);

	UFUNCTION(BlueprintCallable)
	void OnOverlapBegin(class UPrimitiveComponent* OverlappedComp,
		class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION(BlueprintCallable)
	void OnOverlapEnd(class UPrimitiveComponent* OverlappedComp,
		class AActor* OtherActor, class UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex);

	virtual void Tick(float DeltaTime) override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Properties")
	EInteractableToolTags ToolTags;

	UPROPERTY()
	TArray<FOculusXRCapsuleCollider> BoneCollisionCapsules;

private:
	// this variable is used for house keeping. declared ahead of time.
	TArray<UColliderZone*> ElementsToCleanUp;

	/**
	 * Sometimes colliders get disabled and trigger exit doesn't get called.
	 * Take care of that edge case.
	 */
	void CleanUpDeadColliders();
};
