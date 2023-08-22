// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InteractableButton.h"
#include "ControllerBox.generated.h"

class ATrainLocomotive;
class ACowCar;
class UStaticMeshComponent;
class AInteractableButton;
class AHandsVisualizationSwitcher;

/**
 * Manages array of controller box buttons. This was created from an FBX
 * via "File->Import Into Level..." because the scene graph of
 * ControllerBox.fbx must be respected (especially in relation to
 * button placement. The resulting blueprint created from the import
 * process was modified to accommodate this class.
 */
UCLASS()
class HANDSTRAINSAMPLE_API AControllerBox : public AActor
{
	GENERATED_BODY()

public:
	AControllerBox();

	virtual void Tick(float DeltaTime) override;

	UFUNCTION(Category = "Button Events")
	void SmokeButtonStateChanged(const FInteractableStateArgs& StateArgs);

	UFUNCTION(Category = "Button Events")
	void WhistleButtonStateChanged(const FInteractableStateArgs& StateArgs);

	UFUNCTION(Category = "Button Events")
	void GoCowGo(const FInteractableStateArgs& StateArgs);

	UFUNCTION(Category = "Button Events")
	void HandStyleButtonStateChanged(const FInteractableStateArgs& StateArgs);

	UFUNCTION(Category = "Button Events")
	void ReverseButtonStateChanged(const FInteractableStateArgs& StateArgs);

	UFUNCTION(Category = "Button Events")
	void SpeedUpButtonStateChanged(const FInteractableStateArgs& StateArgs);

	UFUNCTION(Category = "Button Events")
	void SlowDownButtonStateChanged(const FInteractableStateArgs& StateArgs);

	UFUNCTION(Category = "Button Events")
	void StartStopButtonStateChanged(const FInteractableStateArgs& StateArgs);

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, Category = "Button Spawning")
	TSubclassOf<AInteractableButton> InteractableButtonBP;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Button Placement")
	USceneComponent* SmokeButtonAnchorComp;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Button Placement")
	USceneComponent* WhistleButtonAnchorComp;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Button Placement")
	USceneComponent* MooCowButtonAnchorComp;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Button Placement")
	USceneComponent* HandStyleButtonAnchorComp;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Button Placement")
	USceneComponent* ReverseButtonAnchorComp;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Button Placement")
	USceneComponent* SpeedUpButtonAnchorComp;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Button Placement")
	USceneComponent* SlowDownButtonAnchorComp;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Button Placement")
	USceneComponent* StartStopButtonAnchorComp;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Button Placement")
	AInteractableButton* SmokeButtonActor;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Button Placement")
	AInteractableButton* WhistleButtonActor;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Button Placement")
	AInteractableButton* MooCowButtonActor;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Button Placement")
	AInteractableButton* HandStyleButtonActor;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Button Placement")
	AInteractableButton* ReverseButtonActor;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Button Placement")
	AInteractableButton* SpeedUpButtonActor;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Button Placement")
	AInteractableButton* SlowDownButtonActor;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Button Placement")
	AInteractableButton* StartStopButtonActor;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Distances")
	float MaxDistanceFromHMDToLastPos;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Distances")
	float MinDistanceToHMD;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Distances")
	float MinDepthToHMD;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Positioning")
	FVector PanelOffsetFromHMD;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Positioning")
	FVector LastMovedToPos;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Positioning")
	FVector PrevPos;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Lerping")
	bool bIsLerpingToHMD;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	ATrainLocomotive* Locomotive;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	ACowCar* CowCar;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	AHandsVisualizationSwitcher* HandsVisSwitcher;

	UFUNCTION(BlueprintCallable)
	void FindLocomotive();

	UFUNCTION(BlueprintCallable)
	void FindCowCar();

	UFUNCTION(BlueprintCallable)
	void FindHandsVisualizationSwitcher();

	UFUNCTION(BlueprintCallable)
	FVector GetHMDPosition();

	UFUNCTION(BlueprintCallable)
	FVector GetIdealAnchorPosition();

	// This is a blueprint event because it's easy to do delays in blueprints.
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable,
		Category = "Animation")
	void LerpToPosition(float LerpTotalDuration, float TimeLeftForLerp,
		FVector StartPosition, FVector EndPosition);

private:
	const static float TotalFollowDuration;
	const static float HmdMovementThreshold;

	void FindAnchors();
	void SpawnButtonsAtAnchorPositions();
	void HookUpButtonEvents();

	void StartStopTrain();
	void TrainBlowSmoke();
	void CowMoo();
	void TrainBlowWhistle();

	void TrainSpeedUp();
	void TrainSlowDown();
	void TrainReverse();
};
