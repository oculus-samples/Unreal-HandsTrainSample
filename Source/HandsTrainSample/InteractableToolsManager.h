// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InteractableToolsInputRouter.h"
#include "InteractableToolsManager.generated.h"

class AInteractableTool;
class UOculusXRHandComponent;
class UMotionControllerComponent;

UCLASS()
class HANDSTRAINSAMPLE_API AInteractableToolsManager : public AActor
{
	GENERATED_BODY()

public:
	AInteractableToolsManager();

	UPROPERTY(EditDefaultsOnly, BlueprintReadonly)
	TArray<TSubclassOf<AInteractableTool>> LeftHandTools;

	UPROPERTY(EditDefaultsOnly, BlueprintReadonly)
	TArray<TSubclassOf<AInteractableTool>> RightHandTools;

	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "Tools")
	void RegisterInteractableTool(AInteractableTool* InteractableTool);

	UFUNCTION(BlueprintCallable, Category = "Tools")
	void UnRegisterInteractableTool(AInteractableTool* InteractableTool);

protected:
	UFUNCTION(BlueprintCallable, Category = "Initialization")
	void AssociateToolWithHand(UOculusXRHandComponent* Hand, AInteractableTool* Tool);

	UPROPERTY()
	TSet<AInteractableTool*> LeftHandNearTools;
	UPROPERTY()
	TSet<AInteractableTool*> LeftHandFarTools;
	UPROPERTY()
	TSet<AInteractableTool*> RightHandNearTools;
	UPROPERTY()
	TSet<AInteractableTool*> RightHandFarTools;

	UPROPERTY()
	UOculusXRHandComponent* LeftHand;
	UPROPERTY()
	UOculusXRHandComponent* RightHand;

private:
	InteractableToolsInputRouter InputRouter;
};
