// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TrainCarBase.h"
#include "TrainLocomotive.generated.h"

UENUM(BlueprintType)
enum class EEngineSoundState : uint8
{
	Start,
	AccelerateOrSetProperSpeed,
	Stop,
};

class ANormalTrainCar;

/**
 * The train locomotive is the lead car. It accelerates and manually
 * "pulls" its child cars by updating their distances and state.
 */
UCLASS()
class HANDSTRAINSAMPLE_API ATrainLocomotive : public ATrainCarBase
{
	GENERATED_BODY()
public:
	ATrainLocomotive();

	virtual void Tick(float DeltaTime) override;

	virtual void UpdateState(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "Initialization")
	void Initialize(TArray<ANormalTrainCar*> NewChildCars);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Behaviors")
	void WhistleButtonStateChanged();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Behaviors")
	void BlowLotsOfSmoke();

	UFUNCTION(BlueprintCallable, Category = "Behaviors")
	void StartStopTrain(bool bStartTrain);

	UFUNCTION(BlueprintCallable, Category = "Behaviors")
	void StartStopTrainStateChanged();

	UFUNCTION(BlueprintCallable, Category = "Behaviors")
	void DecreaseSpeed();

	UFUNCTION(BlueprintCallable, Category = "Behaviors")
	void IncreaseSpeed();

	UFUNCTION(BlueprintCallable, Category = "Behaviors")
	void Reverse();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Motion")
	float InitialSpeed;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Motion")
	float CurrentSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
	bool bIsStartingOrStopping;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
	bool bIsMoving;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
	bool bInReverse;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cars")
	TArray<ANormalTrainCar*> ChildCars;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio")
	UAudioComponent* EngineAudioComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio")
	UParticleSystemComponent* SmokeParticleSystemComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio")
	UParticleSystemComponent* BillowParticleSystemComp;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound")
	TArray<USoundBase*> DecelerationSounds;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound")
	TArray<USoundBase*> AccelerationSounds;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound")
	USoundBase* StartUpSound;

	UFUNCTION(BlueprintCallable)
	float PlayEngineSoundAndGetLength(EEngineSoundState NewSoundState);

	UFUNCTION(BlueprintCallable)
	void UpdateSmokeEmissionBasedOnSpeed();

	UFUNCTION(BlueprintPure, BlueprintCallable, Category = "Behaviors")
	float GetCurrentSmokeEmissionLerpValue();

	// Wee use a blueprint event because we need a delay node to animate over time
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Behaviors")
	void AnimateStartStop(bool bStartTrain, float StartSpeed, float GoalSpeed, float SpeedChangeDuration,
		float SpeedChangeTimeLeft);

private:
	const static float MinSpeed;
	const static float MaxSpeed;

	const static FName SpawnRateParamName;

	float SpeedDiv;
	float StandardEmissionRate;

	void UpdateDistance(float DeltaTime);
};
