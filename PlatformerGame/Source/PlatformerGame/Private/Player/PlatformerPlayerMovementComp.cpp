// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "PlatformerGame.h"
#include "PlatformerPlayerMovementComp.h"
#include "PlatformerCharacter.h"
#include "PlatformerGameMode.h"

UPlatformerPlayerMovementComp::UPlatformerPlayerMovementComp(const FObjectInitializer& ObjectInitializer) 
	: Super(ObjectInitializer)
{
	MaxAcceleration = 200.0f;
	BrakingDecelerationWalking = MaxAcceleration;
	MaxWalkSpeed = 900.0f;

	SlideVelocityReduction = 30.0f;
	SlideHeight = 60.0f;
	SlideMeshRelativeLocationOffset = FVector(0.0f, 0.0f, 34.0f);
	bWantsSlideMeshRelativeLocationOffset = true;
	MinSlideSpeed = 200.0f;
	MaxSlideSpeed = MaxWalkSpeed + 200.0f;

	ModSpeedObstacleHit = 0.0f;
	ModSpeedLedgeGrab = 0.8f;
}

void UPlatformerPlayerMovementComp::StartFalling(int32 Iterations, float remainingTime, float timeTick, const FVector& Delta, const FVector& subLoc)
{
	Super::StartFalling(Iterations, remainingTime, timeTick, Delta, subLoc);

	if (MovementMode == MOVE_Falling && IsSliding())
	{
		TryToEndSlide();
	}
}

FVector UPlatformerPlayerMovementComp::ScaleInputAcceleration(const FVector& InputAcceleration) const
{
	FVector NewAccel = InputAcceleration;

	APlatformerGameMode* PlatGameMode = GetWorld()->GetAuthGameMode<APlatformerGameMode>();
	if (PlatGameMode && PlatGameMode->IsRoundInProgress())
	{
		NewAccel.X = 1.0f;
	}

	return Super::ScaleInputAcceleration(NewAccel);
}

void UPlatformerPlayerMovementComp::PhysWalking(float deltaTime, int32 Iterations)
{
	APlatformerCharacter* MyPawn = Cast<APlatformerCharacter>(PawnOwner);
	if (MyPawn)
	{
		const bool bWantsToSlide = MyPawn->WantsToSlide();
		if (IsSliding())
		{
			CalcCurrentSlideVelocityReduction(deltaTime);
			CalcSlideVelocity(Velocity);

			const float CurrentSpeedSq = Velocity.SizeSquared();
			if (CurrentSpeedSq <= FMath::Square(MinSlideSpeed))
			{
				// slide has min speed - try to end it
				TryToEndSlide();
			}
		}
		else if (bWantsToSlide)
		{
			if (!IsFlying() &&
				Velocity.SizeSquared() > FMath::Square(MinSlideSpeed * 2.0f)) // make sure pawn has some velocity
			{
				StartSlide();
			}
		}
	}

	Super::PhysWalking(deltaTime, Iterations);
}

void UPlatformerPlayerMovementComp::CalcCurrentSlideVelocityReduction(float DeltaTime)
{
	float ReductionCoef = 0.0f;

	const float FloorDotVelocity = FVector::DotProduct(CurrentFloor.HitResult.ImpactNormal, Velocity.GetSafeNormal());
	const bool bNeedsSlopeAdjustment = (FloorDotVelocity != 0.0f);

	if (bNeedsSlopeAdjustment)
	{
		const float Multiplier = 1.0f + FMath::Abs<float>(FloorDotVelocity);
		if (FloorDotVelocity > 0.0f)
		{
			ReductionCoef += SlideVelocityReduction * Multiplier; // increasing speed when sliding down a slope
		}
		else
		{
			ReductionCoef -= SlideVelocityReduction * Multiplier; // reducing speed when sliding up a slope
		}
	}
	else
	{
		ReductionCoef -= SlideVelocityReduction; // reducing speed on flat ground
	}

	float TimeDilation = GetWorld()->GetWorldSettings()->GetEffectiveTimeDilation();
	CurrentSlideVelocityReduction += (ReductionCoef * TimeDilation * DeltaTime);
}

void UPlatformerPlayerMovementComp::CalcSlideVelocity(FVector& OutVelocity) const
{
	const FVector VelocityDir = Velocity.GetSafeNormal();
	FVector NewVelocity = Velocity + CurrentSlideVelocityReduction * VelocityDir;
	
	const float NewSpeedSq = NewVelocity.SizeSquared();
	if (NewSpeedSq > FMath::Square(MaxSlideSpeed))
	{
		NewVelocity = VelocityDir * MaxSlideSpeed;
	}
	else if (NewSpeedSq < FMath::Square(MinSlideSpeed))
	{
		NewVelocity = VelocityDir * MinSlideSpeed;
	}

	OutVelocity = NewVelocity;
}

void UPlatformerPlayerMovementComp::StartSlide()
{
	if (!bInSlide)
	{
		bInSlide = true;
		CurrentSlideVelocityReduction = 0.0f;
		SetSlideCollisionHeight();

		APlatformerCharacter* MyOwner = Cast<APlatformerCharacter>(PawnOwner);
		if (MyOwner)
		{
			MyOwner->PlaySlideStarted();
		}
	}
}

void UPlatformerPlayerMovementComp::TryToEndSlide()
{
	// end slide if collisions allow
	if (bInSlide)
	{
		if (RestoreCollisionHeightAfterSlide())
		{
			bInSlide = false;

			APlatformerCharacter* MyOwner = Cast<APlatformerCharacter>(PawnOwner);
			if (MyOwner)
			{
				MyOwner->PlaySlideFinished();
			}
		}
	}
}

void UPlatformerPlayerMovementComp::SetSlideCollisionHeight()
{
	if (!CharacterOwner || SlideHeight <= 0.0f)
	{
		return;
	}

	// Do not perform if collision is already at desired size.
	if (CharacterOwner->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight() == SlideHeight)
	{
		return;
	}

	// Change collision size to new value
	CharacterOwner->GetCapsuleComponent()->SetCapsuleSize(CharacterOwner->GetCapsuleComponent()->GetUnscaledCapsuleRadius(), SlideHeight);

	// applying correction to PawnOwner mesh relative location
	if (bWantsSlideMeshRelativeLocationOffset)	
	{
		ACharacter* DefCharacter = CharacterOwner->GetClass()->GetDefaultObject<ACharacter>();
		const FVector Correction = DefCharacter->GetMesh()->RelativeLocation + SlideMeshRelativeLocationOffset;
		CharacterOwner->GetMesh()->SetRelativeLocation(Correction);
	}
}

bool UPlatformerPlayerMovementComp::RestoreCollisionHeightAfterSlide()
{
	ACharacter* CharacterOwner = Cast<ACharacter>(PawnOwner);
	if (!CharacterOwner || !UpdatedPrimitive)
	{
		return false;
	}

	ACharacter* DefCharacter = CharacterOwner->GetClass()->GetDefaultObject<ACharacter>();
	const float DefHalfHeight = DefCharacter->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();
	const float DefRadius = DefCharacter->GetCapsuleComponent()->GetUnscaledCapsuleRadius();

	// Do not perform if collision is already at desired size.
	if (CharacterOwner->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight() == DefHalfHeight)
	{
		return true;
	}	

	const float HeightAdjust = DefHalfHeight - CharacterOwner->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();
	const FVector NewLocation = CharacterOwner->GetActorLocation() + FVector(0.0f, 0.0f, HeightAdjust);

	// check if there is enough space for default capsule size
	FCollisionQueryParams TraceParams(TEXT("FinishSlide"), false, CharacterOwner);
	FCollisionResponseParams ResponseParam;
	InitCollisionParams(TraceParams, ResponseParam);
	const bool bBlocked = GetWorld()->OverlapBlockingTestByChannel(NewLocation, FQuat::Identity, UpdatedPrimitive->GetCollisionObjectType(), FCollisionShape::MakeCapsule(DefRadius, DefHalfHeight), TraceParams);
	if (bBlocked)
	{
		return false;
	}

	// restore capsule size and move up to adjusted location
	CharacterOwner->TeleportTo(NewLocation, CharacterOwner->GetActorRotation(), false, true);
	CharacterOwner->GetCapsuleComponent()->SetCapsuleSize(DefRadius, DefHalfHeight);

	// restoring original PawnOwner mesh relative location
	if (bWantsSlideMeshRelativeLocationOffset)
	{
		CharacterOwner->GetMesh()->SetRelativeLocation(DefCharacter->GetMesh()->RelativeLocation);
	}

	return true;
}

void UPlatformerPlayerMovementComp::PauseMovementForObstacleHit()
{
	SavedSpeed = Velocity.Size() * ModSpeedObstacleHit;

	StopMovementImmediately();
	DisableMovement();
	TryToEndSlide();
}

void UPlatformerPlayerMovementComp::PauseMovementForLedgeGrab()
{
	SavedSpeed = Velocity.Size() * ModSpeedLedgeGrab;

	StopMovementImmediately();
	DisableMovement();
	TryToEndSlide();
}

void UPlatformerPlayerMovementComp::RestoreMovement()
{
	SetMovementMode(MOVE_Walking);

	if (SavedSpeed > 0)
	{
		Velocity = PawnOwner->GetActorForwardVector() * SavedSpeed;
	}
}

bool UPlatformerPlayerMovementComp::IsSliding() const
{
	return bInSlide;
};
