/*
Copyright (c) Meta Platforms, Inc. and affiliates.
All rights reserved.

This source code is licensed under the license found in the
LICENSE file in the root directory of this source tree.
*/

#include "InteractableButton.h"
#include "InteractableTool.h"
#include "ButtonMeshHelper.h"
#include "Components/StaticMeshComponent.h"
#include "Components/AudioComponent.h"

AInteractableButton::AInteractableButton()
{
	ButtonHousing = CreateDefaultSubobject<UStaticMeshComponent>(
		FName(TEXT("ButtonHousing")));
	ButtonHousing->SetupAttachment(RootComponent);

	ButtonMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(
		FName(TEXT("ButtonMeshComp")));
	ButtonMeshComp->SetupAttachment(RootComponent);

	ButtonMeshHelper = CreateDefaultSubobject<UButtonMeshHelper>(
		FName(TEXT("MeshHelper")));

	ButtonGlow = CreateDefaultSubobject<UStaticMeshComponent>(FName(
		TEXT("ButtonGlow")));
	ButtonGlow->SetupAttachment(RootComponent);

	AudioComp = CreateDefaultSubobject<UAudioComponent>(
		FName(TEXT("AudioSource")));
	AudioComp->SetupAttachment(RootComponent);
}

void AInteractableButton::BeginPlay()
{
	Super::BeginPlay();

	ButtonMeshHelper->Initialize(this, ContactZone,
		ButtonMeshComp);

	ToggleButtonGlow(false);
}

void AInteractableButton::PlayClickSound()
{
	AudioComp->SetSound(ActionSound);
	AudioComp->Activate(false);
	AudioComp->Play();
}

void AInteractableButton::ToggleButtonGlow(bool isEnabled)
{
	ButtonGlow->SetVisibility(isEnabled);
}
