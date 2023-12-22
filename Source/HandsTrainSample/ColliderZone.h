// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/BoxComponent.h"
#include "CoreMinimal.h"
#include "InteractableEnums.h"
#include "ColliderZone.generated.h"

/**
 * Base class for all collider components of interactables.
 * The primitive component must have physics enabled on it.
 * Not meant to be used directly. It is must be inherited from.
 */
UCLASS()
class HANDSTRAINSAMPLE_API UColliderZone : public UBoxComponent
{
	GENERATED_BODY()
public:
	virtual ~UColliderZone()
	{
	}

	// TODO: verify if we need editanywhere
	/** Interactable that we belong to. Should be assigned by interactable. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hierarchy")
	class AInteractable* ParentInteractable;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, BlueprintPure)
	EInteractableCollisionDepth GetCollisionDepth() const;
};
