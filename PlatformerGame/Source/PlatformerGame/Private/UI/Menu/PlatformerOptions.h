// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.
#pragma once

#include "GameMenuBuilder.h"

/** supported resolutions count*/
const int32 PlatformerResCount = 4;

/** supported resolutions */
const FIntPoint PlatformerResolutions[PlatformerResCount] = { FIntPoint(800,600), FIntPoint(1024,768), FIntPoint(1280,720), FIntPoint(1920,1080) };

/** delegate called when changes are applied */
DECLARE_DELEGATE(FOnOptionsClosing);

class UPlatformerGameUserSettings;

class FPlatformerOptions : public FGameMenuPage
{
public:
	/** sets owning player controller */
	void MakeMenu(TWeakObjectPtr<APlayerController> _PCOwner);

	/** get current options values for display */
	void UpdateOptions();

	/** plays sound and calls ApplySettings */
	void OnApplySettings();

	/** applies changes in game settings */
	void ApplySettings();

	/** discard changes and go back */
	void DiscardSettings();

	/** reverts non-saved changes in game settings */
	void RevertChanges();

	
	void OnAcceptSettings();

	void OnDiscardSettings();

protected:
	/** User settings pointer */
	TWeakObjectPtr<UPlatformerGameUserSettings> UserSettings;

	/** video resolution option changed handler */
	void VideoResolutionOptionChanged(TSharedPtr<FGameMenuItem> MenuItem, int32 MultiOptionIndex);

	/** full screen option changed handler */
	void FullScreenOptionChanged(TSharedPtr<FGameMenuItem> MenuItem, int32 MultiOptionIndex);

	/** music volume option changed handler */
	void MusicVolumeOptionChanged(TSharedPtr<FGameMenuItem> MenuItem, int32 MultiOptionIndex);

	/** sound volume option changed handler */
	void SoundVolumeOptionChanged(TSharedPtr<FGameMenuItem> MenuItem, int32 MultiOptionIndex);

	/** try to match current resolution with selected index */
	int32 GetCurrentResolutionIndex(FIntPoint CurrentRes);

	/** Owning player controller */
	TWeakObjectPtr<APlayerController> PCOwner;


	/** holds volume option menu item */
	TSharedPtr<FGameMenuItem> SoundVolumeOption;

	/** holds video resolution menu item */
	TSharedPtr<FGameMenuItem> VideoResolutionOption;

	/** holds full screen option menu item */
	TSharedPtr<FGameMenuItem> FullScreenOption;

	/** full screen setting set in options */
	EWindowMode::Type bFullScreenOpt;

	/** resolution setting set in options */
	FIntPoint ResolutionOpt;

	/** sound volume set in options */
	float SoundVolumeOpt;

	/** Sound to play when changes are accepted */
	FSlateSound AcceptChangesSound;

	/** Sound to play when changes are discarded */
	FSlateSound DiscardChangesSound;
};