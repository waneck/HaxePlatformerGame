// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "PlatformerGameUserSettings.generated.h"

UCLASS()
class UPlatformerGameUserSettings : public UGameUserSettings
{
	GENERATED_UCLASS_BODY()

	/** Applies all current user settings to the game and saves to permanent storage (e.g. file), optionally checking for command line overrides. */
	virtual void ApplySettings(bool bCheckForCommandLineOverrides) override;

	/** Checks if any user settings is different from current */
	virtual bool IsDirty() const;


	/** Getter for the music volume */
	float GetSoundVolume() const
	{
		return SoundVolume;
	}

	void SetSoundVolume(float InVolume)
	{
		SoundVolume = InVolume;
	}

	/** Checks if the Inverted Mouse user setting is different from current */
	bool IsSoundVolumeDirty() const;

private:

	/** Holds the music volume */
	UPROPERTY(config)
	float SoundVolume;
};