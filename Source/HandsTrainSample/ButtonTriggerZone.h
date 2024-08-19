/*
Copyright (c) Meta Platforms, Inc. and affiliates.
All rights reserved.

This source code is licensed under the license found in the
LICENSE file in the root directory of this source tree.
*/

#pragma once

#include "CoreMinimal.h"
#include "ColliderZone.h"
#include "ButtonTriggerZone.generated.h"

class AInteractable;

/**
 * Trigger zone for buttons.
 */
UCLASS()
class HANDSTRAINSAMPLE_API UButtonTriggerZone : public UColliderZone
{
	GENERATED_BODY()
public:
	virtual EInteractableCollisionDepth GetCollisionDepth_Implementation() const override;
};
