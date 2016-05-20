// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.
#pragma once
#include "SlateBasics.h"
#include "SlateExtras.h"
#include "PlatformerOptions.h"

class APlatformerPlayerController;

class FPlatformerIngameMenu : public FGameMenuPage
{
public:
	/** sets owning player controller */
	void MakeMenu(APlatformerPlayerController* InPCOwner);

	/** toggles in game menu */
	void ToggleGameMenu();

	/** is game menu currently active? */
	bool GetIsGameMenuUp() const;

	void ResumeGame();

	void ReturnToMainMenu();

	void DetachGameMenu();

protected:

	/** game menu container widget - used for removing */
	TSharedPtr<class SWeakWidget> GameMenuContainer;

	/** if game menu is currently opened*/
	uint32 bIsGameMenuUp : 1;

	/** called when going back to previous menu */
	void OnMenuGoBack();

	/** Close the menu and return to main menu */
	void CloseAndExit();

	/* Cached actor hidden state. */
	uint32 bWasActorHidden : 1;

};