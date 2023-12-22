// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "SelectionCylinderHelper.generated.h"

UENUM(BlueprintType)
enum class ESelectionState : uint8
{
	Off = 0,
	Selected,
	Highlighted
};

class UStaticMeshComponent;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class HANDSTRAINSAMPLE_API USelectionCylinderHelper : public UActorComponent
{
	GENERATED_BODY()

public:
	USelectionCylinderHelper();

	UFUNCTION(BlueprintCallable, BlueprintPure)
	ESelectionState GetSelectionState();

	UFUNCTION(BlueprintCallable)
	void SetSelectionState(ESelectionState SelectionState);

	void Initialize(UStaticMeshComponent* StaticMeshComponent);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Meshes")
	UStaticMeshComponent* MeshComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Meshes")
	FColor DefaultColorBase;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Meshes")
	FColor HighlightColorBase;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Meshes")
	FColor DefaultColor;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Meshes")
	FColor HighlightColor;

private:
	const static FName BaseMaterialName;
	const static FName TintParamName;

	ESelectionState CurrSelectionState;

	void AffectSelectionColor(bool bIsSelectedState);
};
