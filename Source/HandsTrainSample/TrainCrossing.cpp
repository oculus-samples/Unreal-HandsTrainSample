// Fill out your copyright notice in the Description page of Project Settings.

#include "TrainCrossing.h"
#include "CollidableInteractable.h"
#include "InteractableTool.h"
#include "SelectionCylinderHelper.h"
#include "Components/AudioComponent.h"

ATrainCrossing::ATrainCrossing()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ATrainCrossing::BeginPlay()
{
	Super::BeginPlay();

	RRXingMeshComp = (UStaticMeshComponent*)GetDefaultSubobjectByName(FName(TEXT("RRXing")));
	SelectionMeshComp = (UStaticMeshComponent*)GetDefaultSubobjectByName(FName(TEXT("SelectionMesh")));
	Light1Mesh = (UStaticMeshComponent*)GetDefaultSubobjectByName(FName(TEXT("LightSurface1")));
	Light2Mesh = (UStaticMeshComponent*)GetDefaultSubobjectByName(FName(TEXT("LightSurface2")));
	AudioComp = (UAudioComponent*)GetDefaultSubobjectByName(FName(TEXT("XingAudio")));
	CollidableAnchor = (USceneComponent*)GetDefaultSubobjectByName(
		FName(TEXT("CollidableInteractAnchor")));
	SelectionCylinderHelperComp = (USelectionCylinderHelper*)GetDefaultSubobjectByName(
		FName(TEXT("SelectionCylinderHelper")));

	CollidableLogic = GetWorld()->SpawnActor<ACollidableInteractable>(
		CollidableInteractableBP,
		CollidableAnchor->GetComponentLocation(),
		CollidableAnchor->GetComponentRotation());
	CollidableLogic->AttachToActor(this,
		FAttachmentTransformRules::KeepWorldTransform);

	CollidableLogic->OnInteractableStateChanged.AddDynamic(this,
		&ATrainCrossing::InteractableStateChanged);

	SelectionCylinderHelperComp->Initialize(SelectionMeshComp);
}

void ATrainCrossing::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (ToolInteractingWithMe == nullptr)
	{
		SelectionCylinderHelperComp->SetSelectionState(ESelectionState::Off);
	}
	else
	{
		SelectionCylinderHelperComp->SetSelectionState(
			(ToolInteractingWithMe->GetCurrInputState() == EToolInputState::PrimaryInputDown || ToolInteractingWithMe->GetCurrInputState() == EToolInputState::PrimaryInputDownStay)
				? ESelectionState::Highlighted
				: ESelectionState::Selected);
	}
}

void ATrainCrossing::InteractableStateChanged(const FInteractableStateArgs& StateArgs)
{
	bool bInActionState = StateArgs.NewInteractableState == EInteractableState::ActionState;
	if (bInActionState)
	{
		ActivateTrainCrossing();
	}

	ToolInteractingWithMe = StateArgs.NewInteractableState > EInteractableState::Default ? StateArgs.Tool : nullptr;
}

void ATrainCrossing::ActivateTrainCrossing()
{
	AudioComp->SetSound(CrossingSound);
	AudioComp->Activate(false);
	AudioComp->Play();

	ToggleLightObjects(true);
	float AnimationLength = CrossingSound->Duration * 0.75f;
	float lightBlinkDuration = AnimationLength * 0.1f;
	BlinkFirstLight = true;
	BlinkLights(lightBlinkDuration, lightBlinkDuration, AnimationLength);
}

void ATrainCrossing::ToggleLightObjects(bool enableState)
{
	Light1Mesh->SetVisibility(enableState, true);
	Light2Mesh->SetVisibility(enableState, true);
}
