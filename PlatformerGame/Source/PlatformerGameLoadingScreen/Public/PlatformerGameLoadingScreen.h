// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#pragma once


// @todo ttp 323321

#include "ModuleInterface.h"


/** Module interface for this game's loading screens */
class IPlatformerGameLoadingScreenModule : public IModuleInterface
{
public:
	/** Kicks off the loading screen for in game loading (not startup) */
	virtual void StartInGameLoadingScreen() = 0;
};

