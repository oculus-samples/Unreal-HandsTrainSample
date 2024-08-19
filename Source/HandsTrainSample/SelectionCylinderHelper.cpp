/*
Copyright (c) Meta Platforms, Inc. and affiliates.
All rights reserved.

This source code is licensed under the license found in the
LICENSE file in the root directory of this source tree.
*/

#include "SelectionCylinderHelper.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

const FName USelectionCylinderHelper::BaseMaterialName = FName("Base");
const FName USelectionCylinderHelper::TintParamName = FName("TintColor");

USelectionCylinderHelper::USelectionCylinderHelper()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void USelectionCylinderHelper::Initialize(UStaticMeshComponent* StaticMeshComponent)
{
	this->MeshComponent = StaticMeshComponent;
	CurrSelectionState = ESelectionState::Off;
}

ESelectionState USelectionCylinderHelper::GetSelectionState()
{
	return CurrSelectionState;
}

void USelectionCylinderHelper::SetSelectionState(ESelectionState SelectionState)
{
	auto OldState = CurrSelectionState;
	CurrSelectionState = SelectionState;

	if (OldState != CurrSelectionState && IsValid(MeshComponent))
	{
		if (CurrSelectionState > ESelectionState::Off)
		{
			MeshComponent->SetVisibility(true);
			AffectSelectionColor(CurrSelectionState == ESelectionState::Selected);
		}
		else
		{
			MeshComponent->SetVisibility(false);
		}
	}
}

void USelectionCylinderHelper::AffectSelectionColor(bool bIsSelectedState)
{
	const TArray<UMaterialInterface*> MaterialInterfaces = MeshComponent->GetMaterials();
	for (int32 MaterialIndex = 0; MaterialIndex < MaterialInterfaces.Num(); ++MaterialIndex)
	{
		UMaterialInterface* MaterialInterface = MaterialInterfaces[MaterialIndex];
		if (MaterialInterface)
		{
			UMaterialInstanceDynamic* DynamicMaterial = Cast<UMaterialInstanceDynamic>(MaterialInterface);
			if (!DynamicMaterial)
			{
				DynamicMaterial = MeshComponent->CreateAndSetMaterialInstanceDynamic(MaterialIndex);
			}

			if (DynamicMaterial->GetName().Contains(BaseMaterialName.ToString()))
			{
				DynamicMaterial->SetVectorParameterValue(
					TintParamName, bIsSelectedState ? FVector(DefaultColorBase) : FVector(HighlightColorBase));
			}
			else
			{
				DynamicMaterial->SetVectorParameterValue(
					TintParamName, bIsSelectedState ? FVector(DefaultColor) : FVector(HighlightColor));
			}
		}
	}
}
