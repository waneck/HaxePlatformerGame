// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "PlatformerBlueprintLibrary.generated.h"

UCLASS()
class UPlatformerBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_UCLASS_BODY()

	/** switch to waiting state, player will be able to start race by himself */
	UFUNCTION(BlueprintCallable, Category=Game, meta=(WorldContext="WorldContextObject"))
	static void PrepareRace(class UObject* WorldContextObject);

	/** finish round, player will be able to replay, returns true when scoring new record */
	UFUNCTION(BlueprintCallable, Category=Game, meta=(WorldContext="WorldContextObject"))
	static bool FinishRace(class UObject* WorldContextObject);

	/** allow player to restart round */
	UFUNCTION(BlueprintCallable, Category=Game, meta=(WorldContext="WorldContextObject"))
	static void AllowToRestartRace(class UObject* WorldContextObject);

	/** allows changing round duration, while round is still in progress - used to give players time bonuses */
	UFUNCTION(BlueprintCallable, Category=Game, meta=(WorldContext="WorldContextObject"))
	static void DecreaseRoundDuration(class UObject* WorldContextObject, float DeltaTime);

	/** returns delta between current checkpoint time and best (negative = better time) */
	UFUNCTION(BlueprintCallable, Category=Game, meta=(WorldContext="WorldContextObject"))
	static float MarkCheckpointTime(class UObject* WorldContextObject, int32 CheckpointID);

	/** returns checkpoint time saved in current round */
	UFUNCTION(BlueprintPure, Category=Game, meta=(WorldContext="WorldContextObject"))
	static float GetCurrentCheckpointTime(class UObject* WorldContextObject, int32 CheckpointID);

	/** returns best time on given checkpoint */
	UFUNCTION(BlueprintPure, Category=Game, meta=(WorldContext="WorldContextObject"))
	static float GetBestCheckpointTime(class UObject* WorldContextObject, int32 CheckpointID);

	/** returns index of last saved checkpoint */
	UFUNCTION(BlueprintPure, Category=Game, meta=(WorldContext="WorldContextObject"))
	static int32 GetLastCheckpoint(class UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, Category=HUD, meta=(WorldContext="WorldContextObject"))
	static void DisplayMessage(class UObject* WorldContextObject, FString Message, float DisplayDuration = 1.0f, float PosX = 0.5f, float PosY = 0.5f, float TextScale = 1.0f, bool bRedBorder = false);

	/** converts time to string in mm:ss.sss format */
	UFUNCTION(BlueprintPure, Category=HUD)
	static FString DescribeTime(float TimeSeconds, bool bShowSign = true);

	/** displays specified texture covering entire screen */
	UFUNCTION(BlueprintCallable, Category=HUD, meta=(WorldContext="WorldContextObject"))
	static void ShowPicture(class UObject* WorldContextObject, UTexture2D* Picture, float FadeInTime = 0.3f, float ScreenCoverage = 1.0f, bool bKeepAspectRatio = false);

	/** hides previously displayed picture */
	UFUNCTION(BlueprintCallable, Category=HUD, meta=(WorldContext="WorldContextObject"))
	static void HidePicture(class UObject* WorldContextObject, float FadeOutTime = 0.3f);

	/** shows highscore with provided data */
	UFUNCTION(BlueprintCallable, Category=HUD, meta=(WorldContext="WorldContextObject"))
	static void ShowHighscore(class UObject* WorldContextObject, TArray<float> Times, TArray<FString> Names);

	/** hides the highscore */
	UFUNCTION(BlueprintCallable, Category=HUD, meta=(WorldContext="WorldContextObject"))
	static void HideHighscore(class UObject* WorldContextObject);

	/** shows highscore prompt, calls HighscoreNameAccepted when user is done */
	UFUNCTION(BlueprintCallable, Category=HUD, meta=(WorldContext="WorldContextObject"))
	static void ShowHighscorePrompt(class UObject* WorldContextObject);

	/* 
	 * Sorts this highscores. 
	 * 
	 * @param	InTimes		Array of times to story
	 * @param	InNames		Names linked to each time
	 * @param	OutTimes	Sorted times
	 * @param	OutNames	Names sorted with times
	 * @param	MaxScores	Limit output times to this amount
	 */
	UFUNCTION(BlueprintCallable, Category = Game)
	static void SortHighscores(TArray<float> InTimes, TArray<FString> InNames, TArray<float>& OutTimes, TArray<FString>& OutNames, int32 MaxScores);
};
