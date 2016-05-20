// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "PlatformerPlayerController_Menu.generated.h"

UCLASS()
class APlatformerPlayerController_Menu : public APlayerController
{
	GENERATED_UCLASS_BODY()

	/** After game is initialized */
	virtual void PostInitializeComponents() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
protected:
	
	/** Platformer main menu */
	TSharedPtr<class FPlatformerMainMenu> PlatformerMainMenu;
};

