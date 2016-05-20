// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "PlatformerGame.h"
#include "PlatformerLevelSelect.h"
#include "PlatformerGameLoadingScreen.h"

#define LOCTEXT_NAMESPACE "PlatformerGame.HUD.Menu"

// @note Construct isn't a good name as it can be confused with Slate interface
void FPlatformerLevelSelect::MakeMenu(TWeakObjectPtr<APlayerController> _PCOwner)
{
	PCOwner = _PCOwner;
	MenuTitle = LOCTEXT("LevelSelect", "SELECT LEVEL");
	AddMenuItem(LOCTEXT("Streets", "STREETS"), this, &FPlatformerLevelSelect::OnUIPlayStreets);
	AddMenuItem(LOCTEXT("Back", "BACK"), this, &FPlatformerLevelSelect::GoBack);
}

void FPlatformerLevelSelect::OnUIPlayStreets()
{
	if (GEngine)
	{
		SetOnHiddenHandler<FPlatformerLevelSelect>(this, &FPlatformerLevelSelect::OnMenuHidden);
		HideMenu();		
	}
}

void FPlatformerLevelSelect::OnMenuHidden()
{
	DestroyRootMenu();
	GEngine->SetClientTravel(PCOwner->GetWorld(), TEXT("/Game/Maps/Platformer_StreetSection"), TRAVEL_Absolute);
	FSlateApplication::Get().SetAllUserFocusToGameViewport();
	ShowLoadingScreen();
}

void FPlatformerLevelSelect::GoBack()
{
	RootMenuPageWidget->MenuGoBack(false);
}

void FPlatformerLevelSelect::ShowLoadingScreen()
{
	IPlatformerGameLoadingScreenModule* LoadingScreenModule = FModuleManager::LoadModulePtr<IPlatformerGameLoadingScreenModule>("PlatformerGameLoadingScreen");
	if (LoadingScreenModule != NULL)
	{
		LoadingScreenModule->StartInGameLoadingScreen();
	}
}

#undef LOCTEXT_NAMESPACE
