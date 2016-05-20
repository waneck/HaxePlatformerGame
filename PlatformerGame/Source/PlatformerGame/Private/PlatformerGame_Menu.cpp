// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "PlatformerGame.h"
#include "PlatformerGame_Menu.h"
#include "Player/PlatformerPlayerController_Menu.h"

APlatformerGame_Menu::APlatformerGame_Menu(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	PlayerControllerClass = APlatformerPlayerController_Menu::StaticClass();
}

void APlatformerGame_Menu::RestartPlayer(class AController* NewPlayer)
{
	// don't restart
}
