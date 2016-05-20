// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "PlatformerGame.h"
#include "PlatformerGameUserSettings.h"


UPlatformerGameUserSettings::UPlatformerGameUserSettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SoundVolume = 1.0f;
}

void UPlatformerGameUserSettings::ApplySettings(bool bCheckForCommandLineOverrides)
{
	Super::ApplySettings(bCheckForCommandLineOverrides);
	if (GEngine && GEngine->GetMainAudioDevice())
	{
		GEngine->GetMainAudioDevice()->TransientMasterVolume = SoundVolume;
	}
}

bool UPlatformerGameUserSettings::IsSoundVolumeDirty() const
{
	bool bIsDirty = false;
	if (GEngine && GEngine->GetMainAudioDevice())
	{
		float CurrentSoundVolume = GEngine->GetMainAudioDevice()->TransientMasterVolume;
		bIsDirty = CurrentSoundVolume != GetSoundVolume();
	}
	return bIsDirty;
}

bool UPlatformerGameUserSettings::IsDirty() const
{
	return Super::IsDirty() || IsSoundVolumeDirty();
}