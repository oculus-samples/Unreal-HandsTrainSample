// Fill out your copyright notice in the Description page of Project Settings.

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
