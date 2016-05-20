// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "PlatformerPlayerController.generated.h"

UCLASS()
class APlatformerPlayerController : public APlayerController
{
	GENERATED_UCLASS_BODY()

	// Begin Actor interface
	/** after all game elements are created */
	virtual void PostInitializeComponents() override;
	// End Actor interface

public:

	/** try starting game **/
	UFUNCTION(BlueprintCallable, Category="Game")
	bool TryStartingGame();

	/** toggle InGameMenu handler */
	void OnToggleInGameMenu();

protected:

	/** platformer in-game menu */
	TSharedPtr<class FPlatformerIngameMenu> PlatformerIngameMenu;

	// Begin PlayerController interface
	virtual void SetupInputComponent() override;
	// End PlayerController interface
};
