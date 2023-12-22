// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interactable.h"
#include "TrainCrossing.generated.h"

class ACollidableInteractable;
class AInteractableTool;

UCLASS()
class HANDSTRAINSAMPLE_API ATrainCrossing : public AActor
{
	GENERATED_BODY()

public:
	ATrainCrossing();

	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "Animation")
	void ActivateTrainCrossing();

	UFUNCTION()
	void InteractableStateChanged(const FInteractableStateArgs& StateArgs);

protected:
	// This is a blueprint event because it's easy to do delays in blueprints.
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Animation")
	void BlinkLights(float TimeLeftUntilNextBlink, float LightBlinkDuration, float TimeLeftForAnimation);

	UFUNCTION(BlueprintCallable, Category = "Animation")
	void ToggleLightObjects(bool enableState);

	// since this is creating from an fbx scene object, find
	// components dynamically during initialization
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Meshes")
	class UStaticMeshComponent* RRXingMeshComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Meshes")
	class UStaticMeshComponent* SelectionMeshComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Meshes")
	class UStaticMeshComponent* Light1Mesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Meshes")
	class UStaticMeshComponent* Light2Mesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class USelectionCylinderHelper* SelectionCylinderHelperComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio")
	UAudioComponent* AudioComp;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	USceneComponent* CollidableAnchor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	USoundBase* CrossingSound;

	UPROPERTY(BlueprintReadWrite)
	bool BlinkFirstLight;

	UPROPERTY(BlueprintReadOnly)
	ACollidableInteractable* CollidableLogic;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<ACollidableInteractable> CollidableInteractableBP;

	UPROPERTY(BlueprintReadOnly)
	AInteractableTool* ToolInteractingWithMe;

	virtual void BeginPlay() override;
};
