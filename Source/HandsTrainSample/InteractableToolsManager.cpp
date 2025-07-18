/*
Copyright (c) Meta Platforms, Inc. and affiliates.
All rights reserved.

This source code is licensed under the license found in the
LICENSE file in the root directory of this source tree.
*/

#include "InteractableToolsManager.h"
#include "OculusXRHandComponent.h"
#include "InteractableTool.h"
#include "MotionControllerComponent.h"
#include "FingerTipPokeTool.h"
#include "Rendering/SkeletalMeshRenderData.h"
#include "Components/SkeletalMeshComponent.h"

AInteractableToolsManager::AInteractableToolsManager()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AInteractableToolsManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	InputRouter.UpdateTools(LeftHand, RightHand,
		LeftHandNearTools, LeftHandFarTools,
		RightHandNearTools, RightHandFarTools);
}

void AInteractableToolsManager::AssociateToolWithHand(UOculusXRHandComponent* Hand,
	AInteractableTool* Tool)
{
	bool ToolIsRightHanded = Hand->MeshType == EOculusXRHandType::HandRight;
	Tool->IsRightHandedTool = ToolIsRightHanded;
	Tool->Initialize(Hand);
	RegisterInteractableTool(Tool);
	Tool->OnInteractableToolDeathEvent.AddDynamic(this,
		&AInteractableToolsManager::UnRegisterInteractableTool);

	// set hands if not already set
	if (LeftHand == nullptr && !ToolIsRightHanded)
	{
		LeftHand = Hand;
	}
	if (RightHand == nullptr && ToolIsRightHanded)
	{
		RightHand = Hand;
	}
}

void AInteractableToolsManager::ClearHandCapsulesCollision(UOculusXRHandComponent* Hand)
{
	TArray<FOculusXRCapsuleCollider>& HandCapsules = Hand->CollisionCapsules;

	for (auto CapsuleCollider : HandCapsules)
	{
		auto CapsuleComp = CapsuleCollider.Capsule;
		CapsuleComp->CanCharacterStepUpOn = ECanBeCharacterBase::ECB_No;
		CapsuleComp->SetCanEverAffectNavigation(false);
		CapsuleComp->SetGenerateOverlapEvents(false);
		CapsuleComp->SetCollisionResponseToAllChannels(
			ECollisionResponse::ECR_Ignore);
	}
}

void AInteractableToolsManager::RegisterInteractableTool(AInteractableTool* InteractableTool)
{
	if (InteractableTool->IsRightHandedTool)
	{
		if (InteractableTool->IsFarFieldTool)
		{
			RightHandFarTools.Add(InteractableTool);
		}
		else
		{
			RightHandNearTools.Add(InteractableTool);
		}
	}
	else
	{
		if (InteractableTool->IsFarFieldTool)
		{
			LeftHandFarTools.Add(InteractableTool);
		}
		else
		{
			LeftHandNearTools.Add(InteractableTool);
		}
	}
}

void AInteractableToolsManager::UnRegisterInteractableTool(AInteractableTool* InteractableTool)
{
	if (InteractableTool->IsRightHandedTool)
	{
		if (InteractableTool->IsFarFieldTool)
		{
			RightHandFarTools.Remove(InteractableTool);
		}
		else
		{
			RightHandNearTools.Remove(InteractableTool);
		}
	}
	else
	{
		if (InteractableTool->IsFarFieldTool)
		{
			LeftHandFarTools.Remove(InteractableTool);
		}
		else
		{
			LeftHandNearTools.Remove(InteractableTool);
		}
	}
}
