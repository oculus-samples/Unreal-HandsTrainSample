// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CollidableInteractable.h"
#include "InteractableEnums.h"
#include "InteractableButton.generated.h"

class AInteractableTool;
class UStaticMeshComponent;
class UButtonMeshHelper;

/**
 * Interactable button that responds to near (touch) and far-field (ray cast)
 * interactions.
 */
UCLASS()
class HANDSTRAINSAMPLE_API AInteractableButton : public ACollidableInteractable
{
	GENERATED_BODY()
public:
	AInteractableButton();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Meshes")
	UStaticMeshComponent* ButtonHousing;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Meshes")
	UStaticMeshComponent* ButtonMeshComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Meshes")
	UButtonMeshHelper* ButtonMeshHelper;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Meshes")
	UStaticMeshComponent* ButtonGlow;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio")
	UAudioComponent* AudioComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	USoundBase* ActionSound;

	UFUNCTION(BlueprintCallable)
	void PlayClickSound();

	UFUNCTION(BlueprintCallable)
	void ToggleButtonGlow(bool isEnabled);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Movement")
	void StopResetLerp();

	// lerp speed from start time to end time
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Movement")
	void ResetPositionLerp(float ResetDuration, float TimeLeftForReset);

protected:
	virtual void BeginPlay() override;
};
