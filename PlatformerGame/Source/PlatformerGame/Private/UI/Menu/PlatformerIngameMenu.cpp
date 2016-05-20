// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "PlatformerGame.h"
#include "PlatformerIngameMenu.h"
#include "PlatformerOptions.h"
#include "PlatformerPlayerController.h"
#include "PlatformerGameMode.h"

#define LOCTEXT_NAMESPACE "PlatformerGame.HUD.Menu"

// @note Construct isn't a good name as it can be confused with Slate interface
void FPlatformerIngameMenu::MakeMenu(APlatformerPlayerController* InPCOwner)
{
	if (!GEngine || !GEngine->GameViewport)
	{
		return;
	}
	bWasActorHidden = false;
	TSharedRef<FPlatformerOptions> PlatformerOptions = MakeShareable(new FPlatformerOptions());
	PlatformerOptions->MakeMenu(InPCOwner);
	PlatformerOptions->ApplySettings();

	if (InitialiseRootMenu(InPCOwner, TEXT("/Game/UI/Styles/PlatformerMenuStyle"), GEngine->GameViewport) == true)
	{
		AddMenuItem(LOCTEXT("ResumeGame", "RESUME GAME"), this, &FPlatformerIngameMenu::ResumeGame);
		AddMenuItem(LOCTEXT("Options", "OPTIONS"), PlatformerOptions);
		AddMenuItem(LOCTEXT("Quit", "QUIT"), this, &FPlatformerIngameMenu::CloseAndExit);

		bIsGameMenuUp = false;

		SetCancelHandler<FPlatformerIngameMenu>(this, &FPlatformerIngameMenu::ResumeGame);
	}
}

void FPlatformerIngameMenu::ResumeGame()
{
	if (bIsGameMenuUp == false)
	{
		return;
	}
	
	bIsGameMenuUp = false;
	//Start hiding animation
	RootMenuPageWidget->HideMenu();
	//enable player controls during hide animation
	FSlateApplication::Get().SetAllUserFocusToGameViewport();
	PCOwner->SetCinematicMode(false, false, false, true, true);

	// Leaving Cinematic mode will always unhide the player pawn, we don't want this on the intro or when game is finished.
	APlatformerGameMode* const MyGame = PCOwner->GetWorld()->GetAuthGameMode<APlatformerGameMode>();
	if (MyGame != nullptr)
	{
		MyGame->SetGamePaused(false);
		PCOwner->GetPawn()->SetActorHiddenInGame(bWasActorHidden);
	}	
}

void FPlatformerIngameMenu::DetachGameMenu()
{
	DestroyRootMenu();
}

bool FPlatformerIngameMenu::GetIsGameMenuUp() const
{
	return bIsGameMenuUp;
}


void FPlatformerIngameMenu::ToggleGameMenu()
{
	if ((!RootMenuPageWidget.IsValid() || (bIsGameMenuUp) || (!PCOwner.IsValid())))
 	{
 		return;
 	}
	bIsGameMenuUp = true; 
	
	APlatformerGameMode* const MyGame = PCOwner->GetWorld()->GetAuthGameMode<APlatformerGameMode>();
	if (MyGame != nullptr)
	{
		MyGame->SetGamePaused(true);
		bWasActorHidden = PCOwner->GetPawn()->bHidden;
	}
	ShowRootMenu();
	
	PCOwner->SetCinematicMode(bIsGameMenuUp,false,false,true,true);
}

void FPlatformerIngameMenu::CloseAndExit()
{
	HideMenu();
	SetOnHiddenHandler<FPlatformerIngameMenu>(this, &FPlatformerIngameMenu::ReturnToMainMenu);
}

void FPlatformerIngameMenu::ReturnToMainMenu()
{	
	APlatformerPlayerController * PlatformPC = (PCOwner.IsValid())? Cast<APlatformerPlayerController>(PCOwner.Get()) : NULL;
	FString RemoteReturnReason = NSLOCTEXT("NetworkErrors", "HostHasLeft", "Host has left the game.").ToString();
	FString LocalReturnReason(TEXT(""));

	if ( PlatformPC )
	{
		if (PlatformPC->GetNetMode() < NM_Client)
		{
			for(auto Iterator = PlatformPC->GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
			{
				APlayerController* Controller = *Iterator;
				if (Controller && Controller->IsPrimaryPlayer() && Controller != PlatformPC) 
				{
					Controller->ClientReturnToMainMenu(RemoteReturnReason);
				}
			}
			PlatformPC->ClientReturnToMainMenu(LocalReturnReason);
		}
		else
		{
			PlatformPC->ClientReturnToMainMenu(LocalReturnReason);
		}
	}
	DestroyRootMenu();

}

#undef LOCTEXT_NAMESPACE
