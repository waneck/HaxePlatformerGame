// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "PlatformerGame_Menu.generated.h"

UCLASS()
class APlatformerGame_Menu : public AGameMode
{
	GENERATED_UCLASS_BODY()

	/** skip it, menu doesn't require player start or pawn */
	virtual void RestartPlayer(class AController* NewPlayer) override;
};
