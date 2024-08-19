/*
Copyright (c) Meta Platforms, Inc. and affiliates.
All rights reserved.

This source code is licensed under the license found in the
LICENSE file in the root directory of this source tree.
*/

#include "TrainLocomotive.h"
#include "NormalTrainCar.h"
#include "Components/AudioComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Particles/ParticleSystemComponent.h"

const float ATrainLocomotive::MinSpeed = 20.0f;
const float ATrainLocomotive::MaxSpeed = 270.0f;
const FName ATrainLocomotive::SpawnRateParamName = FName("SpawnRate");

ATrainLocomotive::ATrainLocomotive()
{
	// base class and normal cars don't tick, but locomotive does as it
	// drives the entire train
	PrimaryActorTick.bCanEverTick = true;

	InitialSpeed = 15.0f;
	bIsMoving = false;
}

void ATrainLocomotive::BeginPlay()
{
	Super::BeginPlay();

	bIsMoving = false;
	Distance = 0.0f;
	bIsStartingOrStopping = false;
	CurrentSpeed = 0.0f;
	SpeedDiv = AccelerationSounds.Num() > 0 ? (MaxSpeed - MinSpeed) / (float)AccelerationSounds.Num()
											: 0;

	EngineAudioComp = (UAudioComponent*)GetDefaultSubobjectByName(FName(
		TEXT("EngineAudio")));
	SmokeParticleSystemComp = (UParticleSystemComponent*)GetDefaultSubobjectByName(FName(
		TEXT("SmokeParticleSystem")));
}

void ATrainLocomotive::Initialize(TArray<ANormalTrainCar*> NewChildCars)
{
	ChildCars.Empty();
	ChildCars = NewChildCars;

	for (ANormalTrainCar* ChildTrainCar : ChildCars)
	{
		ChildTrainCar->ParentLocomotive = this;
	}
}

void ATrainLocomotive::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	UpdateState(DeltaTime);
}

void ATrainLocomotive::UpdateState(float DeltaTime)
{
	if (!bIsMoving)
	{
		return;
	}

	if (IsValid(TrainTrack))
	{
		UpdateDistance(DeltaTime);
		UpdateCarPosition();
		RotateCarWheels();
	}

	// update children after locomotive moves
	for (auto TrainCar : ChildCars)
	{
		if (!IsValid(TrainCar))
		{
			continue;
		}
		TrainCar->UpdateState(DeltaTime);
	}
}

void ATrainLocomotive::UpdateDistance(float DeltaTime)
{
	if (!IsValid(TrainTrack))
	{
		return;
	}
	auto SignedSpeed = bInReverse ? -CurrentSpeed : CurrentSpeed;
	Distance = fmod(Distance + SignedSpeed * DeltaTime,
		TrainTrack->GetTrackLength());
}

void ATrainLocomotive::StartStopTrainStateChanged()
{
	if (!bIsStartingOrStopping)
	{
		StartStopTrain(!bIsMoving);
	}
}

void ATrainLocomotive::StartStopTrain(bool bStartTrain)
{
	float EndSpeed = bStartTrain ? InitialSpeed : 0.0f;
	float TimePeriodForSpeedChange = 3.0f;
	bIsStartingOrStopping = true;

	// Note that the audio clip played will automatically loop or not. Start and stop
	// sounds don't loop by default based on their audio settings.
	if (bStartTrain)
	{
		SmokeParticleSystemComp->Activate(true);
		bIsMoving = true;
		SmokeParticleSystemComp->SetFloatParameter(SpawnRateParamName,
			0.0f);
		TimePeriodForSpeedChange = PlayEngineSoundAndGetLength(EEngineSoundState::Start);
	}
	else
	{
		TimePeriodForSpeedChange = PlayEngineSoundAndGetLength(EEngineSoundState::Stop);
	}

	// Make the animation time period a little shorter;
	// if the animation was close to the same length, we might animate the train
	// and the startup sound might stop for a moment before the normal
	// engine sound would have a chance to begin playing.
	TimePeriodForSpeedChange *= 0.9f;
	// This animate function will complete start and stop sequence and adjust
	// the locomotive state.
	AnimateStartStop(bStartTrain, CurrentSpeed, EndSpeed, TimePeriodForSpeedChange,
		TimePeriodForSpeedChange);
}

void ATrainLocomotive::DecreaseSpeed()
{
	if (!bIsStartingOrStopping && bIsMoving)
	{
		CurrentSpeed = FMath::Clamp(CurrentSpeed - SpeedDiv, MinSpeed, MaxSpeed);
		UpdateSmokeEmissionBasedOnSpeed();
		PlayEngineSoundAndGetLength(EEngineSoundState::AccelerateOrSetProperSpeed);
	}
}

void ATrainLocomotive::IncreaseSpeed()
{
	if (!bIsStartingOrStopping && bIsMoving)
	{
		CurrentSpeed = FMath::Clamp(CurrentSpeed + SpeedDiv, MinSpeed, MaxSpeed);
		UpdateSmokeEmissionBasedOnSpeed();
		PlayEngineSoundAndGetLength(EEngineSoundState::AccelerateOrSetProperSpeed);
	}
}

float ATrainLocomotive::PlayEngineSoundAndGetLength(EEngineSoundState NewSoundState)
{
	USoundBase* AudioClip = nullptr;

	if (NewSoundState == EEngineSoundState::Start)
	{
		AudioClip = StartUpSound;
	}
	else
	{
		TArray<USoundBase*> Sounds = NewSoundState == EEngineSoundState::AccelerateOrSetProperSpeed
			? AccelerationSounds
			: DecelerationSounds;
		auto NumSounds = Sounds.Num();
		auto SpeedIndex = FMath::RoundToInt((CurrentSpeed - MinSpeed) / SpeedDiv);
		AudioClip = Sounds[FMath::Clamp(SpeedIndex, 0, NumSounds - 1)];
	}

	// if audio is already playing and we are playing the same track, don't interrupt
	if (EngineAudioComp->Sound == AudioClip && EngineAudioComp->IsPlaying() && NewSoundState == EEngineSoundState::AccelerateOrSetProperSpeed)
	{
		return 0.0f;
	}

	EngineAudioComp->SetSound(AudioClip);
	EngineAudioComp->SetActive(true);
	EngineAudioComp->Play(0.0f);
	return AudioClip->Duration;
}

void ATrainLocomotive::UpdateSmokeEmissionBasedOnSpeed()
{
	SmokeParticleSystemComp->SetFloatParameter(SpawnRateParamName,
		GetCurrentSmokeEmissionLerpValue());
}

float ATrainLocomotive::GetCurrentSmokeEmissionLerpValue()
{
	return (CurrentSpeed - MinSpeed) / (MaxSpeed - MinSpeed);
}

void ATrainLocomotive::Reverse()
{
	bInReverse = !bInReverse;
}
