// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "Interactable.h"
#include "ButtonMeshHelper.generated.h"

class AInteractableButton;
class UStaticMeshComponent;
class UColliderZone;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class HANDSTRAINSAMPLE_API UButtonMeshHelper : public UActorComponent
{
	GENERATED_BODY()

public:
	UButtonMeshHelper();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Button")
	FVector OldRelativePosition;

	UFUNCTION(BlueprintCallable, Category = "Button")
	void Initialize(AInteractableButton* ParentButton, UColliderZone* ButtonContact, UStaticMeshComponent* StaticMeshComponent);

	UFUNCTION()
	void InteractableStateChanged(const FInteractableStateArgs& StateArgs);

	/** 
	  * When the button is in action or contact events it has to respond by
	  * moving up and down.
	  */
	UFUNCTION()
	void ActionOrInContactZoneStayEvent(const FColliderZoneArgs& ZoneArgs);

protected:
	/**
	 * Color of button when something starts to interact with it (like a touch).
	 * think of this as an selection color.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Colors")
	FColor ButtonContactColor;
	/* Color of button when it clicks and performs an action. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Colors")
	FColor ButtonActionColor;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Colors")
	FColor ButtonDefaultColor;

	/* The owning button and contact component of button. Set from the button.*/
	UPROPERTY()
	AInteractableButton* Button;

	UPROPERTY()
	UColliderZone* ButtonContactComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Button")
	float ContactMaxDisplacementDistance;

private:
	const static float LerpToOldPositionDuration;
	const static FName TintColorParameterName;
	const static float ButtonDepth;

	float ButtonInContactOrActionStates;
	UStaticMeshComponent* MeshComponent;
};
