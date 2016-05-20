// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "PlatformerCharacter.generated.h"

class APlatformerClimbMarker;

UCLASS(Abstract)
class APlatformerCharacter : public ACharacter
{
	GENERATED_UCLASS_BODY()

public:
	
	/** player pawn initialization */
	virtual void PostInitializeComponents() override;

	/** perform position adjustments */
	virtual void Tick(float DeltaSeconds) override;

	/** setup input bindings */
	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;

	/** used to make pawn jump ; overridden to handle additional jump input functionality */
	virtual void CheckJumpInput(float DeltaTime) override;

	/** notify from movement about hitting an obstacle while running */
	virtual void MoveBlockedBy(const FHitResult& Impact) override;

	/** play end of round if game has finished with character in mid air */
	virtual void Landed(const FHitResult& Hit) override;

		/** try playing end of round animation */
	void OnRoundFinished();

	/** stop any active animations, reset movement state */
	void OnRoundReset();

	/** returns true when pawn is sliding ; used in AnimBlueprint */
	UFUNCTION(BlueprintCallable, Category="Pawn|Character")
	bool IsSliding() const;

	/** gets bPressedSlide value */
	bool WantsToSlide() const;

	/** event called when player presses jump button */
	void OnStartJump();

	/** event called when player releases jump button */
	void OnStopJump();

	/** event called when player presses slide button */
	void OnStartSlide();

	/** event called when player releases slide button */
	void OnStopSlide();

	/** handle effects when slide starts */
	void PlaySlideStarted();

	/** handle effects when slide is finished */
	void PlaySlideFinished();

	/** gets CameraHeightChangeThreshold value */
	float GetCameraHeightChangeThreshold() const;

private:
	/** 
	 * Camera is fixed to the ground, even when player jumps.
	 * But, if player jumps higher than this threshold, camera will start to follow.
	 */
	UPROPERTY(EditDefaultsOnly, Category=Config)
	float CameraHeightChangeThreshold;

	/** animation for winning game */
	UPROPERTY(EditDefaultsOnly, Category=Animation)
	UAnimMontage* WonMontage;

	/** animation for loosing game */
	UPROPERTY(EditDefaultsOnly, Category=Animation)
	UAnimMontage* LostMontage;

	/** animation for running into an obstacle */
	UPROPERTY(EditDefaultsOnly, Category=Animation)
	UAnimMontage* HitWallMontage;

	/** minimal speed for pawn to play hit wall animation */
	UPROPERTY(EditDefaultsOnly, Category=Animation)
	float MinSpeedForHittingWall;

	/** animation for climbing over small obstacle */
	UPROPERTY(EditDefaultsOnly, Category=Animation)
	UAnimMontage* ClimbOverSmallMontage;

	/** height of small obstacle */
	UPROPERTY(EditDefaultsOnly, Category=Animation)
	float ClimbOverSmallHeight;

	/** animation for climbing over mid obstacle */
	UPROPERTY(EditDefaultsOnly, Category=Animation)
	UAnimMontage* ClimbOverMidMontage;

	/** height of mid obstacle */
	UPROPERTY(EditDefaultsOnly, Category=Animation)
	float ClimbOverMidHeight;

	/** animation for climbing over big obstacle */
	UPROPERTY(EditDefaultsOnly, Category=Animation)
	UAnimMontage* ClimbOverBigMontage;

	/** height of big obstacle */
	UPROPERTY(EditDefaultsOnly, Category=Animation)
	float ClimbOverBigHeight;

	/** animation for climbing to ledge */
	UPROPERTY(EditDefaultsOnly, Category=Animation)
	UAnimMontage* ClimbLedgeMontage;

	/** root offset in climb legde animation */
	UPROPERTY(EditDefaultsOnly, Category=Animation)
	FVector ClimbLedgeRootOffset;

	/** grab point offset along X axis in climb legde animation */
	UPROPERTY(EditDefaultsOnly, Category=Animation)
	float ClimbLedgeGrabOffsetX;

	/** mesh translation used for position adjustments */
	FVector AnimPositionAdjustment;

	/** root motion translation from previous tick */
	FVector PrevRootMotionPosition;

	/** looped slide sound */
	UPROPERTY(EditDefaultsOnly, Category=Sound)
	USoundCue* SlideSound;

	/** audio component playing looped slide sound */
	UPROPERTY()
	UAudioComponent* SlideAC;

	/** true when player is holding slide button */
	uint32 bPressedSlide:1;

	/** ClimbMarker (or to be exact its mesh component - the movable part) we are climbing to */
	UPROPERTY()
	UStaticMeshComponent* ClimbToMarker;
 
	/** location of ClimbMarker we are climbing to */
 	FVector ClimbToMarkerLocation;

	/** Handle for efficient management of ClimbOverObstacle timer */
	FTimerHandle TimerHandle_ClimbOverObstacle;

	/** Handle for efficient management of ResumeMovement timer */
	FTimerHandle TimerHandle_ResumeMovement;

	/** determine obstacle height type and play animation */
	void ClimbOverObstacle();

	/** position pawn on ledge and play animation with position adjustment */
	void ClimbToLedge(const APlatformerClimbMarker* MoveToMarker);

	/** restore pawn's movement state */
	void ResumeMovement();

	/** play end of round animation */
	void PlayRoundFinished();
};

