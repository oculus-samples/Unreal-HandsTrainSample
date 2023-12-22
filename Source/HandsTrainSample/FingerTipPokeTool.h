// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InteractableTool.h"
#include "OculusXRInputFunctionLibrary.h"
#include "FingerTipPokeTool.generated.h"

class UOculusXRHandComponent;

UENUM(BlueprintType)
enum class EHandFinger : uint8
{
	Thumb = 0,
	Index = 1,
	Middle = 2,
	Ring = 3,
	Pinky = 4,
	Max = 5,
};

class ABoneCapsuleTriggerLogic;
class USceneComponent;
class UStaticMeshComponent;

/**
 * Poke tool used for near-field (touching) interactions. Useful for
 * poking buttons with finger tips, for instance.
 */
UCLASS()
class HANDSTRAINSAMPLE_API AFingerTipPokeTool : public AInteractableTool
{
	GENERATED_BODY()
public:
	AFingerTipPokeTool();

	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Meshes")
	USceneComponent* RootSceneComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Meshes")
	UStaticMeshComponent* TargetMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tool Properties")
	EHandFinger FingerToFollow;

	virtual void Initialize_Implementation(UOculusXRHandComponent* HandComponent)
		override;

	void SetVisualEnableState_Implementation(bool NewVisualEnableState) override;

	bool GetVisualEnableState_Implementation() override;

	virtual EInteractableToolTags GetToolTags_Implementation() override;

	void RefreshCurrentIntersectingObjects_Implementation() override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	ABoneCapsuleTriggerLogic* TriggerLogic;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tool Properties")
	TArray<FOculusXRCapsuleCollider> CollisionCapsulesForBone;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tool Properties")
	UOculusXRHandComponent* HandToTrack;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tool Properties")
	FOculusXRCapsuleCollider CapsuleToTrack;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tool Properties")
	EOculusXRBone BoneToTestCollisions;

private:
	const static uint32 NumVelocityFrames = 10;
	FVector VelocityFrames[NumVelocityFrames];
	uint32 CurrentVelocityFrame;

	float SphereRadius;

	bool bIsInitialized;
	float LastScale;

	FVector LastPosition;
	bool SampledMaxFramesAlready;

	void UpdateAverageVelocity(float DeltaTime);
	void CheckAndUpdateScale();
};
