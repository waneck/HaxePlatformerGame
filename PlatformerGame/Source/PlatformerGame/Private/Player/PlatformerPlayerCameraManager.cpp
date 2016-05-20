// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "PlatformerGame.h"
#include "PlatformerPlayerCameraManager.h"
#include "PlatformerCharacter.h"

APlatformerPlayerCameraManager::APlatformerPlayerCameraManager(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{	
	MinCameraZoomOffset = FVector(240.0f, 600.0f, 0.0f);
	MaxCameraZoomOffset = MinCameraZoomOffset * 4.0f;
	CurrentZoomAlpha = 0.1f;

	DesiredCameraOffsetZ = 0.0f;
	CurrentCameraOffsetZ = 0.0f;
	CameraOffsetInterpSpeed = 5.0f;

	CameraFixedRotation = FRotator(0.0f, -90.0f, 0.0f);
	FixedCameraOffsetZ = 130.0f;
}

void APlatformerPlayerCameraManager::UpdateViewTargetInternal(FTViewTarget& OutVT, float DeltaTime)
{
	FVector ViewLoc;
	FRotator ViewRot;
	OutVT.Target->GetActorEyesViewPoint(ViewLoc, ViewRot);
	ViewLoc.Z = CalcCameraOffsetZ(DeltaTime);
	ViewLoc.Z += FixedCameraOffsetZ;

	FVector CurrentCameraZoomOffset = MinCameraZoomOffset + CurrentZoomAlpha * (MaxCameraZoomOffset - MinCameraZoomOffset);
	OutVT.POV.Location = ViewLoc + CurrentCameraZoomOffset;
	OutVT.POV.Rotation = CameraFixedRotation;
}

void APlatformerPlayerCameraManager::SetFixedCameraOffsetZ(float InOffset)
{
	FixedCameraOffsetZ = InOffset;
}

void APlatformerPlayerCameraManager::SetCameraZoom(float ZoomAlpha)
{
	CurrentZoomAlpha = FMath::Clamp(ZoomAlpha, 0.0f, 1.0f);
}

float APlatformerPlayerCameraManager::GetCameraZoom() const
{
	return CurrentZoomAlpha;
}

float APlatformerPlayerCameraManager::CalcCameraOffsetZ(float DeltaTime)
{
	APlatformerCharacter* MyPawn = PCOwner ? Cast<APlatformerCharacter>(PCOwner->GetPawn()) : NULL;
	if (MyPawn)
	{
		float LocZ = MyPawn->GetActorLocation().Z;
		if (MyPawn->GetCharacterMovement() && MyPawn->GetCharacterMovement()->IsFalling())
		{
			if (LocZ < DesiredCameraOffsetZ)
			{
				DesiredCameraOffsetZ = LocZ;
			}
			else if (LocZ > DesiredCameraOffsetZ + MyPawn->GetCameraHeightChangeThreshold())
			{
				DesiredCameraOffsetZ = LocZ;
			}
		}
		else
		{
			DesiredCameraOffsetZ = LocZ;
		}
	}

	if (CurrentCameraOffsetZ != DesiredCameraOffsetZ)
	{
		CurrentCameraOffsetZ = FMath::FInterpTo(CurrentCameraOffsetZ, DesiredCameraOffsetZ, DeltaTime, CameraOffsetInterpSpeed);
	}

	return CurrentCameraOffsetZ;
}
