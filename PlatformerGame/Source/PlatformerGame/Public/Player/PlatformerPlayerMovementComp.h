// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "PlatformerPlayerMovementComp.generated.h"

UCLASS()
class UPlatformerPlayerMovementComp : public UCharacterMovementComponent
{
	GENERATED_UCLASS_BODY()

public:

	/** stop slide when falling */
	virtual void StartFalling(int32 Iterations, float remainingTime, float timeTick, const FVector& Delta, const FVector& subLoc) override;

	/** returns true when pawn is sliding */
	bool IsSliding() const;

	/** attempts to end slide move - fails if collisions above pawn don't allow it */
	void TryToEndSlide();

	/** stop movement and save current speed with obstacle modifier */
	void PauseMovementForObstacleHit();

	/** stop movement and save current speed with ledge grab modifier */
	void PauseMovementForLedgeGrab();

	/** restore movement and saved speed */
	void RestoreMovement();

protected:

	/** update slide */
	virtual void PhysWalking(float deltaTime, int32 Iterations) override;

	/** force movement */
	virtual FVector ScaleInputAcceleration(const FVector& InputAcceleration) const override;

	/** calculates OutVelocity which is new velocity for pawn during slide */
	void CalcSlideVelocity(FVector& OutVelocity) const;

	/** while pawn is sliding calculates new value of CurrentSlideVelocityReduction */
	void CalcCurrentSlideVelocityReduction(float DeltaTime);

	/** handles pawn slide move */
	void HandleSlide(float DeltaTime);

	/** forces pawn to start sliding */
	void StartSlide();	

	/** changes pawn height to SlideHeight and adjusts pawn collisions */
	void SetSlideCollisionHeight();

	/** 
	 * restores pawn height to default after slide, if collisions above pawn allow that 
	 * returns true if height change succeeded, false otherwise
	 */
	bool RestoreCollisionHeightAfterSlide();

private:

	/** speed multiplier after hiting an obstacle */
	UPROPERTY(EditDefaultsOnly, Category=Config)
	float ModSpeedObstacleHit;

	/** speed multiplier after ledge grab */
	UPROPERTY(EditDefaultsOnly, Category=Config)
	float ModSpeedLedgeGrab;

	/** value by which speed will be reduced during slide */
	UPROPERTY(EditDefaultsOnly, Category=Config)
	float SlideVelocityReduction;

	/** minimal speed pawn can slide with */
	UPROPERTY(EditDefaultsOnly, Category=Config)
	float MinSlideSpeed;

	/** maximal speed pawn can slide with */
	UPROPERTY(EditDefaultsOnly, Category=Config)
	float MaxSlideSpeed;

	/** height of pawn while sliding */
	UPROPERTY(EditDefaultsOnly, Category=Config)
	float SlideHeight;

	/** offset value, by which relative location of pawn mesh needs to be changed, when pawn is sliding */
	FVector SlideMeshRelativeLocationOffset;

	/** value by which sliding pawn speed is currently being reduced */	
	float CurrentSlideVelocityReduction;

	/** saved modified value of speed to restore after animation finish */
	float SavedSpeed;

	/** true when pawn is sliding */
	uint32 bInSlide:1;

	/** true if pawn needs to use SlideMeshRelativeLocationOffset while sliding */
	uint32 bWantsSlideMeshRelativeLocationOffset:1;
};

