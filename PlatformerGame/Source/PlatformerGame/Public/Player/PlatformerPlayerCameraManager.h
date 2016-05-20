// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "PlatformerPlayerCameraManager.generated.h"

UCLASS()
class APlatformerPlayerCameraManager : public APlayerCameraManager
{
	GENERATED_UCLASS_BODY()

public:

	/** sets new value of FixedCameraOffsetZ */
	UFUNCTION(BlueprintCallable, Category = "Game|Player")
	void SetFixedCameraOffsetZ(float InOffset);

	/** sets new value of CurrentZoomAlpha <0, 1> */
	UFUNCTION(BlueprintCallable, Category="Game|Player")
	void SetCameraZoom(float ZoomAlpha);

	/** gets current value of CurrentZoomAlpha */
	UFUNCTION(BlueprintCallable, Category="Game|Player")
	float GetCameraZoom() const;

protected:
	// APlayerCameraManager interface

	/** handle camera updates */
	virtual void UpdateViewTargetInternal(FTViewTarget& OutVT, float DeltaTime) override;

	// End of APlayerCameraManager interface

	/** calculates camera Z axis offset dependent on player pawn movement */
	float CalcCameraOffsetZ(float DeltaTime);

private:

	/** fixed maximal camera distance from player pawn ; used for zoom */
	FVector MaxCameraZoomOffset;

	/** fixed minimal camera distance from player pawn ; used for zoom */
	FVector MinCameraZoomOffset;

	/** interpolation speed for changing camera Z axis offset */
	float CameraOffsetInterpSpeed;

	/** fixed camera Z axis offset from player pawn */
	float FixedCameraOffsetZ;

	/** Fixed rotation of the camera relative to player pawn  */
	FRotator CameraFixedRotation;

	/** current value of zoom <0, 1> (0 means MinCameraZoomOffset will be used, 1 means MaxCameraZoomOffset will) */
	float CurrentZoomAlpha;

	/** currently used camera Z axis offset */
	float CurrentCameraOffsetZ;

	/** Z axis offset camera is supposed to achieve */
	float DesiredCameraOffsetZ;	
};
