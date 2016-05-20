// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "PlatformerGameMode.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FRoundFinishedDelegate);

namespace EGameState
{
	enum Type
	{
		Intro,
		Waiting,
		Playing,
		Finished,
		Restarting,
	};
}

UCLASS()
class APlatformerGameMode : public AGameMode
{
	GENERATED_UCLASS_BODY()

public:

	/** prepare game state and show HUD message */
	void PrepareRound(bool bRestarting = false);

	/** used to start this round */
	void StartRound();

	/** finish current round */
	void FinishRound();

	/** is game paused? */
	bool IsGamePaused() const;

	/** pauses/unpauses the game */
	void SetGamePaused(bool bIsPaused);

	/** sets if round can be restarted */
	void SetCanBeRestarted(bool bAllowRestart);

	/** returns if round can be restarted */
	bool CanBeRestarted() const;

	/** tries to restart round */
	void TryRestartRound();

	/** save current time for checkpoint */
	void SaveCheckpointTime(int32 CheckpointID);

	/** get checkpoint time: current round */
	float GetCurrentCheckpointTime(int32 CheckpointID) const;

	/** get checkpoint time: best */
	float GetBestCheckpointTime(int32 CheckpointID) const;

	/** get number of checkpoints */
	int32 GetNumCheckpoints() const;

	/** 
	 * returns time that passed since round has started (in seconds)
	 * if the round has already ended returns round duration
	 */
	float GetRoundDuration() const;

	/** increases/decreases round duration by DeltaTime */
	void ModifyRoundDuration(float DeltaTime, bool bIncrease);

	/** returns true if round is in progress - player is still moving */
	bool IsRoundInProgress() const;

	/** returns true if round was won (best time) */
	bool IsRoundWon() const;

	/** get current state of game */
	EGameState::Type GetGameState() const;

	/** delegate to broadcast about finished round */
	UPROPERTY(BlueprintAssignable)
	FRoundFinishedDelegate OnRoundFinished;

	/** full screen picture info */
	class FPlatformerPicture* PlatformerPicture;

private:

	/** Handle for efficient management of StartRound timer */
	FTimerHandle TimerHandle_StartRound;

	/** the time player started this round */
	float RoundStartTime;

	/** true when round is in progress */
	EGameState::Type GameState;

	/** true if player won this round, false otherwise */
	uint32 bRoundWasWon:1;

	/** true if game is paused */
	uint32 bIsGamePaused:1;

	/** true if round can be restarted after finishing */
	uint32 bCanBeRestarted:1;

	/** current checkpoint times */
	TArray<float> CurrentTimes;

	/** best checkpoint times */
	TArray<float> BestTimes;
};

