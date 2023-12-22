// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HandsActiveChecker.generated.h"

class UMotionControllerComponent;

UCLASS()
class HANDSTRAINSAMPLE_API AHandsActiveChecker : public AActor
{
	GENERATED_BODY()

public:
	AHandsActiveChecker();

	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Meshes")
	USceneComponent* RootSceneComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Meshes")
	UStaticMeshComponent* NotificationMesh;

	UPROPERTY()
	APawn* PlayerPawn;

	UPROPERTY()
	UStaticMeshComponent* LeftMesh;

	UPROPERTY()
	UStaticMeshComponent* RightMesh;

private:
	void FindPawnAndControllers();
};
