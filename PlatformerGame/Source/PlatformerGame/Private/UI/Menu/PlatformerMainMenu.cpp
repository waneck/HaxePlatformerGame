// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "PlatformerGame.h"
#include "PlatformerMainMenu.h"
#include "PlatformerOptions.h"
#include "PlatformerLevelSelect.h"

#define LOCTEXT_NAMESPACE "PlatformerGame.HUD.Menu"

void FPlatformerMainMenu::MakeMenu(APlayerController* InPCOwner)
{
	TSharedRef<FPlatformerOptions> Options = MakeShareable(new FPlatformerOptions());
	Options->MakeMenu(InPCOwner);
	Options->ApplySettings();
	
	TSharedRef<FPlatformerLevelSelect> LevelSelect = MakeShareable(new FPlatformerLevelSelect());
	LevelSelect->MakeMenu(InPCOwner);
	
	if (InitialiseRootMenu(InPCOwner, TEXT("/Game/UI/Styles/PlatformerMenuStyle"), GEngine->GameViewport) == true)
	{
		MenuTitle = LOCTEXT("MainMenu", "MAIN MENU");

		AddMenuItem(LOCTEXT("PlayGame", "PLAY GAME"), LevelSelect);
		AddMenuItem(LOCTEXT("Options", "OPTIONS"), Options);
		AddMenuItem(LOCTEXT("Quit", "QUIT"), this, &FPlatformerMainMenu::OnQuit);
	}
}

void FPlatformerMainMenu::OnQuit()
{
	PCOwner->ConsoleCommand("quit");
}


#undef LOCTEXT_NAMESPACE
