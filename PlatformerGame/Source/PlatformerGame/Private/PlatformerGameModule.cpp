// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "PlatformerGame.h"


class FPlatformerGameModule : public FDefaultGameModuleImpl
{
	virtual void StartupModule() override
	{

	}

	virtual void ShutdownModule() override
	{
	
	}
};

IMPLEMENT_PRIMARY_GAME_MODULE(FPlatformerGameModule, PlatformerGame, "PlatformerGame");

DEFINE_LOG_CATEGORY(LogPlatformer)
