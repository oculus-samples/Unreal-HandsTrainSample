// Fill out your copyright notice in the Description page of Project Settings.

#include "ControllerBox.h"
#include "TrainLocomotive.h"
#include "CowCar.h"
#include "IXRTrackingSystem.h"
#include "Engine/Engine.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "HandsVisualizationSwitcher.h"
#include <Components/StaticMeshComponent.h>

const float AControllerBox::TotalFollowDuration = 3.0f;
const float AControllerBox::HmdMovementThreshold = 30.0f;

AControllerBox::AControllerBox()
{
	PrimaryActorTick.bCanEverTick = true;

	MaxDistanceFromHMDToLastPos = 30.0f;
	MinDistanceToHMD = 5.0f;
	MinDepthToHMD = 5.0f;
	LastMovedToPos = FVector::ZeroVector;
	PanelOffsetFromHMD = FVector(30.0f, 0.0f, -40.0f);
}

void AControllerBox::FindLocomotive()
{
	if (!IsValid(Locomotive))
	{
		Locomotive = (ATrainLocomotive*)UGameplayStatics::GetActorOfClass(GetWorld(),
			ATrainLocomotive::StaticClass());
		if (!IsValid(Locomotive))
		{
			UE_LOG(LogTemp, Error, TEXT("Controller box found no locomotive actor!"));
		}
	}
}

void AControllerBox::FindCowCar()
{
	if (!IsValid(CowCar))
	{
		CowCar = (ACowCar*)UGameplayStatics::GetActorOfClass(GetWorld(),
			ACowCar::StaticClass());
		if (!IsValid(CowCar))
		{
			UE_LOG(LogTemp, Error, TEXT("Controller box found no cow car actor!"));
		}
	}
}

void AControllerBox::FindHandsVisualizationSwitcher()
{
	if (!IsValid(HandsVisSwitcher))
	{
		HandsVisSwitcher = (AHandsVisualizationSwitcher*)
			UGameplayStatics::GetActorOfClass(GetWorld(),
				AHandsVisualizationSwitcher::StaticClass());
		if (!IsValid(HandsVisSwitcher))
		{
			UE_LOG(LogTemp, Error, TEXT("Controller box found no hands switcher!"));
		}
	}
}

void AControllerBox::BeginPlay()
{
	Super::BeginPlay();

	bIsLerpingToHMD = false;

	FindAnchors();
	SpawnButtonsAtAnchorPositions();
	HookUpButtonEvents();
}

void AControllerBox::FindAnchors()
{
	/**
	 * These anchors should are found from the blueprint
	 * based on the position of the buttons in the FBX.
	 * The button actors are then spawned at runtime,
	 * so that they are easier to access compared to
	 * child actor components.
	 */
	SmokeButtonAnchorComp = (USceneComponent*)GetDefaultSubobjectByName(
		FName(TEXT("SmokeButtonAnchor")));
	WhistleButtonAnchorComp = (USceneComponent*)GetDefaultSubobjectByName(
		FName(TEXT("WhistleButtonAnchor")));
	MooCowButtonAnchorComp = (USceneComponent*)GetDefaultSubobjectByName(
		FName(TEXT("MooCowButtonAnchor")));
	HandStyleButtonAnchorComp = (USceneComponent*)GetDefaultSubobjectByName(
		FName(TEXT("HandStyleButtonAnchor")));
	ReverseButtonAnchorComp = (USceneComponent*)GetDefaultSubobjectByName(
		FName(TEXT("ReverseButtonAnchor")));
	SpeedUpButtonAnchorComp = (USceneComponent*)GetDefaultSubobjectByName(
		FName(TEXT("SpeedUpButtonAnchor")));
	SlowDownButtonAnchorComp = (USceneComponent*)GetDefaultSubobjectByName(
		FName(TEXT("SlowDownButtonAnchor")));
	StartStopButtonAnchorComp = (USceneComponent*)GetDefaultSubobjectByName(
		FName(TEXT("StartStopButtonAnchor")));
}

void AControllerBox::SpawnButtonsAtAnchorPositions()
{
	SmokeButtonActor = GetWorld()->SpawnActor<AInteractableButton>(
		InteractableButtonBP,
		SmokeButtonAnchorComp->GetComponentLocation(),
		SmokeButtonAnchorComp->GetComponentRotation());
	SmokeButtonActor->AttachToActor(this,
		FAttachmentTransformRules::KeepWorldTransform);
	WhistleButtonActor = GetWorld()->SpawnActor<AInteractableButton>(
		InteractableButtonBP,
		WhistleButtonAnchorComp->GetComponentLocation(),
		WhistleButtonAnchorComp->GetComponentRotation());
	WhistleButtonActor->AttachToActor(this,
		FAttachmentTransformRules::KeepWorldTransform);
	MooCowButtonActor = GetWorld()->SpawnActor<AInteractableButton>(
		InteractableButtonBP,
		MooCowButtonAnchorComp->GetComponentLocation(),
		MooCowButtonAnchorComp->GetComponentRotation());
	MooCowButtonActor->AttachToActor(this,
		FAttachmentTransformRules::KeepWorldTransform);
	HandStyleButtonActor = GetWorld()->SpawnActor<AInteractableButton>(
		InteractableButtonBP,
		HandStyleButtonAnchorComp->GetComponentLocation(),
		HandStyleButtonAnchorComp->GetComponentRotation());
	HandStyleButtonActor->AttachToActor(this,
		FAttachmentTransformRules::KeepWorldTransform);

	ReverseButtonActor = GetWorld()->SpawnActor<AInteractableButton>(
		InteractableButtonBP,
		ReverseButtonAnchorComp->GetComponentLocation(),
		ReverseButtonAnchorComp->GetComponentRotation());
	ReverseButtonActor->AttachToActor(this,
		FAttachmentTransformRules::KeepWorldTransform);
	SpeedUpButtonActor = GetWorld()->SpawnActor<AInteractableButton>(
		InteractableButtonBP,
		SpeedUpButtonAnchorComp->GetComponentLocation(),
		SpeedUpButtonAnchorComp->GetComponentRotation());
	SpeedUpButtonActor->AttachToActor(this,
		FAttachmentTransformRules::KeepWorldTransform);
	SlowDownButtonActor = GetWorld()->SpawnActor<AInteractableButton>(
		InteractableButtonBP,
		SlowDownButtonAnchorComp->GetComponentLocation(),
		SlowDownButtonAnchorComp->GetComponentRotation());
	SlowDownButtonActor->AttachToActor(this,
		FAttachmentTransformRules::KeepWorldTransform);
	StartStopButtonActor = GetWorld()->SpawnActor<AInteractableButton>(
		InteractableButtonBP,
		StartStopButtonAnchorComp->GetComponentLocation(),
		StartStopButtonAnchorComp->GetComponentRotation());
	StartStopButtonActor->AttachToActor(this,
		FAttachmentTransformRules::KeepWorldTransform);
}

void AControllerBox::HookUpButtonEvents()
{
	SmokeButtonActor->OnInteractableStateChanged.AddDynamic(this,
		&AControllerBox::SmokeButtonStateChanged);

	WhistleButtonActor->OnInteractableStateChanged.AddDynamic(this,
		&AControllerBox::WhistleButtonStateChanged);

	MooCowButtonActor->OnInteractableStateChanged.AddDynamic(this,
		&AControllerBox::GoCowGo);

	HandStyleButtonActor->OnInteractableStateChanged.AddDynamic(this,
		&AControllerBox::HandStyleButtonStateChanged);

	ReverseButtonActor->OnInteractableStateChanged.AddDynamic(this,
		&AControllerBox::ReverseButtonStateChanged);

	SpeedUpButtonActor->OnInteractableStateChanged.AddDynamic(this,
		&AControllerBox::SpeedUpButtonStateChanged);

	SlowDownButtonActor->OnInteractableStateChanged.AddDynamic(this,
		&AControllerBox::SlowDownButtonStateChanged);

	StartStopButtonActor->OnInteractableStateChanged.AddDynamic(this,
		&AControllerBox::StartStopButtonStateChanged);

	// useful for testing button actions on PC
	// in case hand tracking doesn't work in editor
#if WITH_EDITOR
	EnableInput(GetWorld()->GetFirstPlayerController());
	InputComponent->BindAction("ActionTrainStartStop",
		EInputEvent::IE_Pressed, this,
		&AControllerBox::StartStopTrain);

	InputComponent->BindAction("ActionTrainSmoke",
		EInputEvent::IE_Pressed, this,
		&AControllerBox::TrainBlowSmoke);

	InputComponent->BindAction("ActionCowMoo",
		EInputEvent::IE_Pressed, this,
		&AControllerBox::CowMoo);

	InputComponent->BindAction("ActionTrainWhistle",
		EInputEvent::IE_Pressed, this,
		&AControllerBox::TrainBlowWhistle);

	InputComponent->BindAction("ActionTrainSpeedUp",
		EInputEvent::IE_Pressed, this,
		&AControllerBox::TrainSpeedUp);

	InputComponent->BindAction("ActionTrainSlowDown",
		EInputEvent::IE_Pressed, this,
		&AControllerBox::TrainSlowDown);

	InputComponent->BindAction("ActionTrainReverse",
		EInputEvent::IE_Pressed, this,
		&AControllerBox::TrainReverse);
#endif
}

void AControllerBox::SmokeButtonStateChanged(const FInteractableStateArgs&
		StateArgs)
{
	if (StateArgs.NewInteractableState == EInteractableState::ActionState)
	{
		TrainBlowSmoke();
	}
}

void AControllerBox::WhistleButtonStateChanged(const FInteractableStateArgs&
		StateArgs)
{
	if (StateArgs.NewInteractableState == EInteractableState::ActionState)
	{
		TrainBlowWhistle();
	}
}

void AControllerBox::GoCowGo(const FInteractableStateArgs&
		StateArgs)
{
	if (StateArgs.NewInteractableState == EInteractableState::ActionState)
	{
		CowMoo();
	}
}

void AControllerBox::HandStyleButtonStateChanged(const FInteractableStateArgs&
		StateArgs)
{
	FindHandsVisualizationSwitcher();
	if (StateArgs.NewInteractableState == EInteractableState::ActionState && IsValid(HandsVisSwitcher))
	{
		HandsVisSwitcher->SwitchHandsVisualization();
	}
}

void AControllerBox::ReverseButtonStateChanged(const FInteractableStateArgs&
		StateArgs)
{
	if (StateArgs.NewInteractableState == EInteractableState::ActionState)
	{
		TrainReverse();
	}
}

void AControllerBox::SpeedUpButtonStateChanged(const FInteractableStateArgs&
		StateArgs)
{
	if (StateArgs.NewInteractableState == EInteractableState::ActionState)
	{
		TrainSpeedUp();
	}
}

void AControllerBox::SlowDownButtonStateChanged(const FInteractableStateArgs&
		StateArgs)
{
	if (StateArgs.NewInteractableState == EInteractableState::ActionState)
	{
		TrainSlowDown();
	}
}

void AControllerBox::StartStopButtonStateChanged(const FInteractableStateArgs&
		StateArgs)
{
	if (StateArgs.NewInteractableState == EInteractableState::ActionState)
	{
		StartStopTrain();
	}
}

void AControllerBox::StartStopTrain()
{
	FindLocomotive();
	if (IsValid(Locomotive))
	{
		Locomotive->StartStopTrainStateChanged();
	}
}

void AControllerBox::TrainBlowSmoke()
{
	FindLocomotive();
	if (IsValid(Locomotive))
	{
		Locomotive->BlowLotsOfSmoke();
	}
}

void AControllerBox::CowMoo()
{
	FindCowCar();
	if (IsValid(CowCar))
	{
		CowCar->GoMooCowGo();
	}
}

void AControllerBox::TrainBlowWhistle()
{
	FindLocomotive();
	if (IsValid(Locomotive))
	{
		Locomotive->WhistleButtonStateChanged();
	}
}

void AControllerBox::TrainSpeedUp()
{
	FindLocomotive();
	if (IsValid(Locomotive))
	{
		Locomotive->IncreaseSpeed();
	}
}

void AControllerBox::TrainSlowDown()
{
	FindLocomotive();
	if (IsValid(Locomotive))
	{
		Locomotive->DecreaseSpeed();
	}
}

void AControllerBox::TrainReverse()
{
	FindLocomotive();
	if (IsValid(Locomotive))
	{
		Locomotive->Reverse();
	}
}

void AControllerBox::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	auto CurrentHMDPosition = GetHMDPosition();
	float DistanceFromHMDPosToLastPos = FVector::Dist(CurrentHMDPosition,
		LastMovedToPos);
	float HeadMovementSpeed = FVector::Dist(CurrentHMDPosition, PrevPos) / DeltaTime;

	FVector CurrentLocation = GetActorLocation();
	FVector HMDToMyLocation = CurrentLocation - CurrentHMDPosition;
	float CurrDistanceFromHMD = HMDToMyLocation.Size();

	/**
	 * 1) Check if HMD moves too far from its original stable position. -OR-
	 * 2) Check if HMD is too close to controller box. -OR-
	 * 3) Check if depth is too far relative to controller box. -AND-
	 * 4) Make sure head stabilizes and isn't moving too fast currently.
	 * -AND-
	 * 5) We are not lerping
	 */
	if (((DistanceFromHMDPosToLastPos > MaxDistanceFromHMDToLastPos) || (MinDistanceToHMD > CurrDistanceFromHMD) || (MinDepthToHMD > HMDToMyLocation.X)) && (HeadMovementSpeed < HmdMovementThreshold)
		&& !bIsLerpingToHMD)
	{
		LastMovedToPos = CurrentHMDPosition;
		bIsLerpingToHMD = true;
		LerpToPosition(TotalFollowDuration, TotalFollowDuration,
			CurrentLocation, GetIdealAnchorPosition());
	}

	PrevPos = CurrentHMDPosition;
}

FVector AControllerBox::GetIdealAnchorPosition()
{
	FVector HMDPosition = GetHMDPosition();
	return HMDPosition + PanelOffsetFromHMD;
}

FVector AControllerBox::GetHMDPosition()
{
	FRotator DeviceRotation;
	FVector DevicePosition;
	UHeadMountedDisplayFunctionLibrary::GetOrientationAndPosition(DeviceRotation, DevicePosition);
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (!IsValid(PlayerPawn))
	{
		return FVector::ZeroVector;
	}
	FTransform PlayerTransform = PlayerPawn->GetTransform();
	FVector TransformedPos = PlayerTransform.TransformPosition(DevicePosition);
	return TransformedPos;
}
