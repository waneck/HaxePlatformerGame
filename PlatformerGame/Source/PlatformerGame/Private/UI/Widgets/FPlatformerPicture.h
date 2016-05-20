// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#pragma once

//class declare
class FPlatformerPicture
{
public:
	/** picture constructor */
	FPlatformerPicture(UWorld* World);

	/** shows picture */
	void Show(UTexture2D* InTexture, float InFadeInTime, float InScreenCoverage, bool bInKeepAspectRatio);

	/** hides picture */
	void Hide(float FadeOutTime);

	/** used for fade in and fade out */
	void Tick(UCanvas* Canvas);

	/** check if picture is currently visible */
	bool IsVisible() const;

protected:
	/** if picture is currently drawn */
	bool bIsVisible;

	/** if picture is currently fading out */
	bool bIsHiding;

	/** fade in or fade out started time */
	float AnimStartedTime;

	/** how long current animation should be playing (fading in or fading out) */
	float AnimTime;

	/** picture tint color */
	FLinearColor TintColor;

	/** picture scale compared to the size of viewport 1.0f = full screen */
	float ScreenCoverage;
	
	/** if picture should keep aspect ratio when scaling or use viewport aspect ratio */
	bool bKeepAspectRatio;

	/** image resource */
	UTexture2D* Image;

	/** owner world */
	UWorld* OwnerWorld;
};
