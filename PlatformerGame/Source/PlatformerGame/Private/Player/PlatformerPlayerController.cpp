// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "PlatformerGame.h"
#include "PlatformerIngameMenu.h"
#include "PlatformerPlayerController.h"
#include "PlatformerPlayerCameraManager.h"
#include "PlatformerGameMode.h"


APlatformerPlayerController::APlatformerPlayerController(const FObjectInitializer& ObjectInitializer) 
	: Super(ObjectInitializer)
{
	PlayerCameraManagerClass = APlatformerPlayerCameraManager::StaticClass();
	bEnableClickEvents = true;
	bEnableTouchEvents = true;
}

void APlatformerPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// UI input
	InputComponent->BindAction("InGameMenu", IE_Pressed, this, &APlatformerPlayerController::OnToggleInGameMenu);
}

void APlatformerPlayerController::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	//Build menu only after game is initialized
	// @note Initialize in FPlatformerGameModule::StartupModule is not enough - it won't execute in cooked game
	IGameMenuBuilderModule::Get();
	PlatformerIngameMenu = MakeShareable(new FPlatformerIngameMenu());
	PlatformerIngameMenu->MakeMenu(this);
}

void APlatformerPlayerController::OnToggleInGameMenu()
{
	APlatformerGameMode* MyGame = GetWorld()->GetAuthGameMode<APlatformerGameMode>();
	if (PlatformerIngameMenu.IsValid() && MyGame && MyGame->GetGameState() != EGameState::Finished)
	{
		PlatformerIngameMenu->ToggleGameMenu();		
	}
}

bool APlatformerPlayerController::TryStartingGame()
{
	APlatformerGameMode* MyGame = GetWorld()->GetAuthGameMode<APlatformerGameMode>();
	if (MyGame)
	{
		const EGameState GameState = MyGame->GetGameState();
		switch (GameState)
		{
			case EGameState::Waiting:
				MyGame->StartRound();
				return true;

			case EGameState::Finished:
				MyGame->TryRestartRound();
				return true;
		}
	}

	return false;
}
