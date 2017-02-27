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
	FAudioDevice* MainAudioDevice = (GEngine ? GEngine->GetMainAudioDevice() : nullptr);
	if (MainAudioDevice)
	{
		MainAudioDevice->SetTransientMasterVolume(SoundVolume);
	}
}

bool UPlatformerGameUserSettings::IsSoundVolumeDirty() const
{
	bool bIsDirty = false;
	FAudioDevice* MainAudioDevice = (GEngine ? GEngine->GetMainAudioDevice() : nullptr);
	if (MainAudioDevice)
	{
		// This will cause a stall, but it is relatively rare and there isn't really a better option right now
		FAudioThreadSuspendContext AudioThreadSuspend;
		const float CurrentSoundVolume = MainAudioDevice->GetTransientMasterVolume();
		bIsDirty = (CurrentSoundVolume != GetSoundVolume());
	}
	return bIsDirty;
}

bool UPlatformerGameUserSettings::IsDirty() const
{
	return Super::IsDirty() || IsSoundVolumeDirty();
}
