// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "PlatformerHUD.generated.h"

struct FPlatformerMessageData
{
	/** text to display */
	FString Message;
	
	/** how long this FMessageData will be displayed in seconds */
	float DisplayDuration;

	/** TimeSeconds when this FMessageData was first shown */
	float DisplayStartTime;
	
	/** x axis position on screen <0, 1> (0 means left side of the screen) ; text will be centered */
	float PosX;
	
	/** y axis position on screen <0, 1> (0 means top of the screen) ; text will be centered */
	float PosY;
	
	/** text scale */
	float TextScale;
	
	/** if red border should be drawn instead of blue */
	bool bRedBorder;
};

struct FBorderTextures
{
	/** border texture */
	UTexture2D* Border;

	/** border background texture */
	UTexture2D* Background;

	/** left border texture */
	UTexture2D* LeftBorder;

	/** right border texture */
	UTexture2D* RightBorder;

	/** top border texture */
	UTexture2D* TopBorder;

	/** bottom border texture */
	UTexture2D* BottomBorder;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHighscoreNameAccepted, const FString&, NewHighscoreName);

UCLASS()
class APlatformerHUD : public AHUD
{
	GENERATED_UCLASS_BODY()

public:

	/** called when OK was hit while highscore name prompt was active */
	UPROPERTY(BlueprintAssignable)
	FOnHighscoreNameAccepted OnHighscoreNameAccepted;

	/** main HUD update loop */
	virtual void DrawHUD() override;

	/** used to add new message to ActiveMessages array */
	void AddMessage(FString Message, float DisplayDuration = 1.f, float PosX = 0.5f, float PosY = 0.5f, float TextScale = 1.f, bool bRedBorder = false);

	void NotifyRoundTimeModified(float DeltaTime);

	/** Called when a hit box is clicked on. Provides the name associated with that box. */
	virtual void NotifyHitBoxClick(FName BoxName) override;

	/** sets the data and shows the highscore */
	void ShowHighscore(TArray<float> Times, TArray<FString> Names);

	/** hides highscore */
	void HideHighscore();

	/** shows highscore prompt, calls HighscoreNameAccepted blueprint implementable event when user is done */
	void ShowHighscorePrompt();

protected:

	/** used to display main game timer - top middle of the screen */
	void DisplayRoundTimer();

	void DisplayRoundTimeModification();

	/** used to display active messages and removing expired ones */
	void DrawActiveMessages();

	/** used to display single text message with specified parameters */
	void DrawMessage(FString Message, float PosX, float PosY, float TextScale, FLinearColor TextColor, bool bRedBorder=false);

	/** draws 3x3 border with tiled background*/
	void DrawBorder(float PosX, float PosY, float Width, float Height, float BorderScale, FBorderTextures& BorderTextures);

	/** draws high score entry prompt */
	void DrawHighscoreEntryPrompt();

	/** draws high score */
	void DrawHighscore();

private:

	/** array of messages that should be displayed on screen for a fixed time */
	TArray<struct FPlatformerMessageData> ActiveMessages;

	/** summary messages */
	TArray<struct FPlatformerMessageData> EndingMessages;

	float RoundTimeModification;

	float RoundTimeModificationTime;

	/** roboto Light 48p font */
	UFont* HUDFont;

	/** blue themed border textures */
	FBorderTextures BlueBorder;

	/** red themed border textures */
	FBorderTextures RedBorder;

	/** up button texture */
	UTexture2D* UpButtonTexture;

	/** down button texture*/
	UTexture2D* DownButtonTexture;

	/** screen resolution */
	FIntPoint ScreenRes;

	/** current UI scale */
	float UIScale;

	/** current highscore name */
	TArray<char> HighScoreName;

	/** current letter to change while entering highscore name */
	uint8 CurrentLetter;

	/* if we should show enter name prompt */
	uint32 bEnterNamePromptActive : 1;

	/** if highscore is currently displayed */
	uint32 bHighscoreActive : 1;

	/** highscore times */
	TArray<float> HighscoreTimes;

	/** highscore names */
	TArray<FString> HighscoreNames;
};
