/*
Copyright (c) Meta Platforms, Inc. and affiliates.
All rights reserved.

This source code is licensed under the license found in the
LICENSE file in the root directory of this source tree.
*/

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

	UFUNCTION(BlueprintCallable, Category = "Initialization")
	void ClearHandCapsulesCollision(UOculusXRHandComponent* Hand);

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
