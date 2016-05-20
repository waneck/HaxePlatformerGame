// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "PlatformerGame.h"
#include "PlatformerGameMode.h"
#include "UI/PlatformerHUD.h"
#include "PlatformerPlayerController.h"
#include "PlatformerCharacter.h"

APlatformerGameMode::APlatformerGameMode(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PlayerControllerClass = APlatformerPlayerController::StaticClass();

	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClass(TEXT("/Game/Pawn/PlayerPawn"));
	DefaultPawnClass = PlayerPawnClass.Class;
	HUDClass = APlatformerHUD::StaticClass();

	GameState = EGameState::Intro;
	bRoundWasWon = false;
	RoundStartTime = 0.0f;
	bIsGamePaused = false;
	if ((GEngine != NULL) && (GEngine->GameViewport != NULL))
	{
		GEngine->GameViewport->SetSuppressTransitionMessage(true);
	}
}

void APlatformerGameMode::PrepareRound(bool bRestarting)
{
	if (bRestarting)
	{
		OnRoundFinished.Broadcast();
	}	

	GameState = bRestarting ? EGameState::Restarting : EGameState::Waiting;
	bRoundWasWon = false;
	RoundStartTime = 0.0f;

	APlatformerPlayerController* PC = Cast<APlatformerPlayerController>(GEngine->GetFirstLocalPlayerController(GetWorld()));
	APlatformerCharacter* Pawn = PC ? Cast<APlatformerCharacter>(PC->GetPawn()) : NULL;
	if (Pawn)
	{
		Pawn->OnRoundReset();

		AActor* StartSpot = FindPlayerStart(PC);
		Pawn->TeleportTo(StartSpot->GetActorLocation(), StartSpot->GetActorRotation());

		if (Pawn->bHidden)
		{
			Pawn->SetActorHiddenInGame(false);
		}
	}
}

void APlatformerGameMode::SetCanBeRestarted(bool bAllowRestart)
{
	if (GameState == EGameState::Finished)
	{
		bCanBeRestarted = bAllowRestart;
	}
}

bool APlatformerGameMode::CanBeRestarted() const
{
	return (GameState == EGameState::Finished && bCanBeRestarted);
}

void APlatformerGameMode::TryRestartRound()
{
	if (CanBeRestarted())
	{
		PrepareRound(true);
		GetWorldTimerManager().SetTimer(TimerHandle_StartRound, this, &APlatformerGameMode::StartRound, 2.0f, false);
		bCanBeRestarted = false;
	}
}

void APlatformerGameMode::StartRound()
{
	RoundStartTime = GetWorld()->GetTimeSeconds();
	GameState = EGameState::Playing;
}

void APlatformerGameMode::FinishRound()
{
	GameState = EGameState::Finished;
	
	// determine game state
	const int32 LastCheckpointIdx = GetNumCheckpoints() - 1;	
	const float BestTime = GetBestCheckpointTime(LastCheckpointIdx);
	bRoundWasWon = (BestTime < 0) || (GetRoundDuration() < BestTime);

	// notify player
	APlayerController* PC = GEngine->GetFirstLocalPlayerController(GetWorld());
	APlatformerCharacter* Pawn = PC ? Cast<APlatformerCharacter>(PC->GetPawn()) : NULL;
	if (Pawn)
	{
		Pawn->OnRoundFinished();
	}

	// update best checkpoint times
	while (LastCheckpointIdx >= BestTimes.Num())
	{
		BestTimes.Add(-1.0f);
	}

	for (int32 i = 0; i < BestTimes.Num(); i++)
	{
		if ((BestTimes[i] < 0) || (BestTimes[i] > CurrentTimes[i]))
		{
			BestTimes[i] = CurrentTimes[i];
		}
	}
}

void APlatformerGameMode::ModifyRoundDuration(float DeltaTime, bool bIncrease)
{
	if (IsRoundInProgress())
	{
		const float PrevRoundStartTime = RoundStartTime;
		const float Delta = FMath::Abs(DeltaTime);
		if (bIncrease)
		{
			RoundStartTime -= Delta;
		}
		else
		{
			const float CurrTime = GetWorld()->GetTimeSeconds();
			RoundStartTime += Delta;
			RoundStartTime = FMath::Min(RoundStartTime, CurrTime);
		}

		APlayerController* PC = GEngine->GetFirstLocalPlayerController(GetWorld());
		APlatformerHUD* HUD = PC ? Cast<APlatformerHUD>(PC->MyHUD) : NULL;
		if (HUD)
		{
			HUD->NotifyRoundTimeModified(PrevRoundStartTime - RoundStartTime);
		}
	}
}

float APlatformerGameMode::GetRoundDuration() const
{
	if (IsRoundInProgress())
	{
		const float CurrTime = GetWorld()->GetTimeSeconds();
		return (CurrTime - RoundStartTime);
	}

	const int32 LastCheckpoint = GetNumCheckpoints() - 1;
	return GetCurrentCheckpointTime(LastCheckpoint);
}

void APlatformerGameMode::SaveCheckpointTime(int32 CheckpointID)
{
	if (CheckpointID < 0)
	{
		return;
	}

	// save current time
	while (CheckpointID >= CurrentTimes.Num())
	{
		CurrentTimes.Add(-1.0f);
	}

	CurrentTimes[CheckpointID] = GetRoundDuration();
}

void APlatformerGameMode::SetGamePaused(bool bIsPaused)
{
	APlayerController* PC = GEngine->GetFirstLocalPlayerController(GetWorld());
	PC->SetPause(bIsPaused);
	bIsGamePaused = bIsPaused;
}

bool APlatformerGameMode::IsGamePaused() const
{
	return bIsGamePaused;
}

bool APlatformerGameMode::IsRoundInProgress() const
{
	return GameState == EGameState::Playing;
}

bool APlatformerGameMode::IsRoundWon() const
{
	return bRoundWasWon;
}

EGameState::Type APlatformerGameMode::GetGameState() const
{
	return GameState;
}

float APlatformerGameMode::GetCurrentCheckpointTime(int32 CheckpointID) const
{
	return CurrentTimes.IsValidIndex(CheckpointID) ? CurrentTimes[CheckpointID] : -1.0f;
}

float APlatformerGameMode::GetBestCheckpointTime(int32 CheckpointID) const
{
	return BestTimes.IsValidIndex(CheckpointID) ? BestTimes[CheckpointID] : -1.0f;
}

int32 APlatformerGameMode::GetNumCheckpoints() const
{
	return FMath::Max(CurrentTimes.Num(), BestTimes.Num());
}
