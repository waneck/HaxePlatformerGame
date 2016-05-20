// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "PlatformerGame.h"
#include "PlatformerCharacter.h"
#include "Player/PlatformerPlayerMovementComp.h"
#include "PlatformerGameMode.h"
#include "PlatformerClimbMarker.h"
#include "PlatformerPlayerController.h"

APlatformerCharacter::APlatformerCharacter(const FObjectInitializer& ObjectInitializer) 
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UPlatformerPlayerMovementComp>(ACharacter::CharacterMovementComponentName))
{
	MinSpeedForHittingWall = 200.0f;
	GetMesh()->MeshComponentUpdateFlag = EMeshComponentUpdateFlag::AlwaysTickPoseAndRefreshBones;
}

void APlatformerCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	// setting initial rotation
	SetActorRotation(FRotator(0.0f, 0.0f, 0.0f));
}

void APlatformerCharacter::SetupPlayerInputComponent(UInputComponent* InputComponent)
{
	InputComponent->BindAction("Jump", IE_Pressed, this, &APlatformerCharacter::OnStartJump);
	InputComponent->BindAction("Jump", IE_Released, this, &APlatformerCharacter::OnStopJump);
	InputComponent->BindAction("Slide", IE_Pressed, this, &APlatformerCharacter::OnStartSlide);
	InputComponent->BindAction("Slide", IE_Released, this, &APlatformerCharacter::OnStopSlide);
}

bool APlatformerCharacter::IsSliding() const
{
	UPlatformerPlayerMovementComp* MoveComp = Cast<UPlatformerPlayerMovementComp>(GetCharacterMovement());
	return MoveComp && MoveComp->IsSliding();
}

void APlatformerCharacter::CheckJumpInput(float DeltaTime)
{
	if (bPressedJump)
	{
		UPlatformerPlayerMovementComp* MoveComp = Cast<UPlatformerPlayerMovementComp>(GetCharacterMovement());
		if (MoveComp && MoveComp->IsSliding())
		{
			MoveComp->TryToEndSlide();
			return;
		}
	}

	Super::CheckJumpInput(DeltaTime);
}

void APlatformerCharacter::Tick(float DeltaSeconds)
{
	// decrease anim position adjustment
	if (!AnimPositionAdjustment.IsNearlyZero())
	{
		AnimPositionAdjustment = FMath::VInterpConstantTo(AnimPositionAdjustment, FVector::ZeroVector, DeltaSeconds, 400.0f);
		GetMesh()->SetRelativeLocation(GetBaseTranslationOffset() + AnimPositionAdjustment);
	}

	if (ClimbToMarker)
	{
		// correction in case climb marker is moving
		const FVector AdjustDelta = ClimbToMarker->GetComponentLocation() - ClimbToMarkerLocation;
		if (!AdjustDelta.IsZero())
		{
			SetActorLocation(GetActorLocation() + AdjustDelta, false);
			ClimbToMarkerLocation += AdjustDelta;
		}
	}

	Super::Tick(DeltaSeconds);

}

void APlatformerCharacter::PlayRoundFinished()
{
	APlatformerGameMode* MyGame = GetWorld()->GetAuthGameMode<APlatformerGameMode>();
	const bool bWon = MyGame && MyGame->IsRoundWon();
	
	PlayAnimMontage(bWon ? WonMontage : LostMontage);

	GetCharacterMovement()->StopMovementImmediately();
	GetCharacterMovement()->DisableMovement();
}

void APlatformerCharacter::OnRoundFinished()
{
	// don't stop in mid air, will be continued from Landed() notify
	if (GetCharacterMovement()->MovementMode != MOVE_Falling)
	{
		PlayRoundFinished();
	}
}

void APlatformerCharacter::OnRoundReset()
{
	// reset animations
	if (GetMesh() && GetMesh()->AnimScriptInstance)
	{
		GetMesh()->AnimScriptInstance->Montage_Stop(0.0f);
	}

	// reset movement properties
	GetCharacterMovement()->StopMovementImmediately();
	GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	bPressedJump = false;
	bPressedSlide = false;
}

void APlatformerCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);

	APlatformerGameMode* MyGame = GetWorld()->GetAuthGameMode<APlatformerGameMode>();
	if (MyGame && MyGame->GetGameState() == EGameState::Finished)
	{
		PlayRoundFinished();
	}
}

void APlatformerCharacter::MoveBlockedBy(const FHitResult& Impact)
{
	const float ForwardDot = FVector::DotProduct(Impact.Normal, FVector::ForwardVector);
	if (GetCharacterMovement()->MovementMode != MOVE_None)
	{
		UE_LOG(LogPlatformer, Log, TEXT("Collision with %s, normal=(%f,%f,%f), dot=%f, %s"),
			*GetNameSafe(Impact.Actor.Get()),
			Impact.Normal.X, Impact.Normal.Y, Impact.Normal.Z,
			ForwardDot,
			*GetCharacterMovement()->GetMovementName());
	}

	if (GetCharacterMovement()->MovementMode == MOVE_Walking && ForwardDot < -0.9f)
	{
		UPlatformerPlayerMovementComp* MyMovement = Cast<UPlatformerPlayerMovementComp>(GetCharacterMovement());
		const float Speed = FMath::Abs(FVector::DotProduct(MyMovement->Velocity, FVector::ForwardVector));
		// if running or sliding: play bump reaction and jump over obstacle

		float Duration = 0.01f;
		if (Speed > MinSpeedForHittingWall)
		{
			Duration = PlayAnimMontage(HitWallMontage);
		}
		GetWorldTimerManager().SetTimer(TimerHandle_ClimbOverObstacle, this, &APlatformerCharacter::ClimbOverObstacle, Duration, false);
		MyMovement->PauseMovementForObstacleHit();
	}
	else if (GetCharacterMovement()->MovementMode == MOVE_Falling)
	{
		// if in mid air: try climbing to hit marker
		APlatformerClimbMarker* Marker = Cast<APlatformerClimbMarker>(Impact.Actor.Get());
		if (Marker)
		{
			ClimbToLedge(Marker);

			UPlatformerPlayerMovementComp* MyMovement = Cast<UPlatformerPlayerMovementComp>(GetCharacterMovement());
			MyMovement->PauseMovementForLedgeGrab();
		}
	}
}

void APlatformerCharacter::ResumeMovement()
{
	SetActorEnableCollision(true);

	// restore movement state and saved speed
	UPlatformerPlayerMovementComp* MyMovement = Cast<UPlatformerPlayerMovementComp>(GetCharacterMovement());
	MyMovement->RestoreMovement();

	ClimbToMarker = NULL;
}

void APlatformerCharacter::ClimbOverObstacle()
{
	// climbing over obstacle:
	// - there are three animations matching with three types of predefined obstacle heights
	// - pawn is moved using root motion, ending up on top of obstacle as animation ends

	const FVector ForwardDir = GetActorForwardVector();
	const FVector TraceStart = GetActorLocation() + ForwardDir * 150.0f + FVector(0,0,1) * (GetCapsuleComponent()->GetScaledCapsuleHalfHeight() + 150.0f);
	const FVector TraceEnd = TraceStart + FVector(0,0,-1) * 500.0f;

	FCollisionQueryParams TraceParams(NAME_None, true);
	FHitResult Hit;
	GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Pawn, TraceParams);

	if (Hit.bBlockingHit)
	{
		const FVector DestPosition = Hit.ImpactPoint + FVector(0, 0, GetCapsuleComponent()->GetScaledCapsuleHalfHeight());
		const float ZDiff = DestPosition.Z - GetActorLocation().Z;
		UE_LOG(LogPlatformer, Log, TEXT("Climb over obstacle, Z difference: %f (%s)"), ZDiff,
			 (ZDiff < ClimbOverMidHeight) ? TEXT("small") : (ZDiff < ClimbOverBigHeight) ? TEXT("mid") : TEXT("big"));

		UAnimMontage* Montage = (ZDiff < ClimbOverMidHeight) ? ClimbOverSmallMontage : (ZDiff < ClimbOverBigHeight) ? ClimbOverMidMontage : ClimbOverBigMontage;
		
		// set flying mode since it needs Z changes. If Walking or Falling, we won't be able to apply Z changes
		// this gets reset in the ResumeMovement
		GetCharacterMovement()->SetMovementMode(MOVE_Flying);
		SetActorEnableCollision(false);
		const float Duration = PlayAnimMontage(Montage);
		GetWorldTimerManager().SetTimer(TimerHandle_ResumeMovement, this, &APlatformerCharacter::ResumeMovement, Duration - 0.1f, false);
	}
	else
	{
		// shouldn't happen
		ResumeMovement();
	}
}

void APlatformerCharacter::ClimbToLedge(const APlatformerClimbMarker* MoveToMarker)
{
	ClimbToMarker = MoveToMarker ? MoveToMarker->FindComponentByClass<UStaticMeshComponent>() : NULL;
	ClimbToMarkerLocation = ClimbToMarker ? ClimbToMarker->GetComponentLocation() : FVector::ZeroVector;

	// place on top left corner of marker, but preserve current Y coordinate
	const FBox MarkerBox = MoveToMarker->GetMesh()->Bounds.GetBox();
	const FVector DesiredPosition(MarkerBox.Min.X, GetActorLocation().Y, MarkerBox.Max.Z);

	// climbing to ledge:
	// - pawn is placed on top of ledge (using ClimbLedgeGrabOffsetX to offset from grab point) immediately
	// - AnimPositionAdjustment modifies mesh relative location to smooth transition
	//   (mesh starts roughly at the same position, additional offset quickly decreases to zero in Tick)

	const FVector StartPosition = GetActorLocation();
	FVector AdjustedPosition = DesiredPosition;
	AdjustedPosition.X += (ClimbLedgeGrabOffsetX * GetMesh()->RelativeScale3D.X) - GetBaseTranslationOffset().X;
	AdjustedPosition.Z += GetCapsuleComponent()->GetScaledCapsuleHalfHeight();

	TeleportTo(AdjustedPosition, GetActorRotation(), false, true);

	AnimPositionAdjustment = StartPosition - (GetActorLocation() - (ClimbLedgeRootOffset * GetMesh()->RelativeScale3D));
	GetMesh()->SetRelativeLocation(GetBaseTranslationOffset() + AnimPositionAdjustment);

	const float Duration = PlayAnimMontage(ClimbLedgeMontage);
	GetWorldTimerManager().SetTimer(TimerHandle_ResumeMovement, this, &APlatformerCharacter::ResumeMovement, Duration - 0.1f, false);
}

void APlatformerCharacter::OnStartJump()
{
	APlatformerGameMode* const MyGame = GetWorld()->GetAuthGameMode<APlatformerGameMode>();
	APlatformerPlayerController* MyPC = Cast<APlatformerPlayerController>(Controller);
	if (MyPC)
	{
		if (MyPC->TryStartingGame())
		{
			return;
		}

		if (!MyPC->IsMoveInputIgnored() &&
			MyGame && MyGame->IsRoundInProgress())
		{
			bPressedJump = true;
		}
	}
}

void APlatformerCharacter::OnStopJump()
{
	bPressedJump = false;
}

void APlatformerCharacter::OnStartSlide()
{
	APlatformerGameMode* MyGame = GetWorld()->GetAuthGameMode<APlatformerGameMode>();
	APlatformerPlayerController* MyPC = Cast<APlatformerPlayerController>(Controller);
	if (MyPC)
	{
		if (MyPC->TryStartingGame())
		{
			return;
		}
		
		if (!MyPC->IsMoveInputIgnored() &&
			MyGame && MyGame->IsRoundInProgress())
		{
			bPressedSlide = true;
		}
	}
}

void APlatformerCharacter::OnStopSlide()
{
	bPressedSlide = false;
}

void APlatformerCharacter::PlaySlideStarted()
{
	if (SlideSound)
	{
		SlideAC = UGameplayStatics::SpawnSoundAttached(SlideSound, GetMesh());
	}
}

void APlatformerCharacter::PlaySlideFinished()
{
	if (SlideAC)
	{
		SlideAC->Stop();
		SlideAC = NULL;
	}
}

bool APlatformerCharacter::WantsToSlide() const
{
	return bPressedSlide;
}

float APlatformerCharacter::GetCameraHeightChangeThreshold() const
{
	return CameraHeightChangeThreshold;
}
