// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.
#pragma once
#include "SlateBasics.h"
#include "SlateExtras.h"
#include "PlatformerOptions.h"

#include "GameMenuBuilder.h"

class FPlatformerMainMenu : public FGameMenuPage
{
public:
	/** build menu */
	void MakeMenu(APlayerController* InPCOwner);
	void OnQuit();
protected:
	
	/** platformer options */
	TSharedPtr<class FPlatformerOptions> PlatformerOptions;
};