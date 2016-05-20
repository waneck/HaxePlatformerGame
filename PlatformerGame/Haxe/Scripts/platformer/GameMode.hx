// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

package platformer;
import unreal.*;

using unreal.CoreAPI;

// define a new delegate
typedef FRoundFinishedDelegate = DynamicMulticastDelegate<FRoundFinishedDelegate, Void->Void>;

// you can define your own UENUMs through Haxe
@:uenum
enum EGameState {
  Intro;
  Waiting;
  Playing;
  Finished;
  Restarting;
}

@:uclass
// you can use @:uname to define the target name, without having to use that
@:uname("APlatformerGameMode")
class GameMode extends AGameMode {
  /** delegate to broadcast about finished round */
  @:uproperty(BlueprintAssignable)
  public var OnRoundFinished:FRoundFinishedDelegate;

  /** true if game is paused */
  public var IsGamePaused(default, set):Bool;

  /** true if player won this round, false otherwise */
  public var RoundWasWon(default, null):Bool;

  /** true when round is in progress */
  public var MyGameState(default, null):EGameState;

  /** full screen picture info */
  @:uexpose var PlatformerPicture:PPtr<Picture>;

  /** Handle for efficient management of StartRound timer */
  var TimerHandle_StartRound:FTimerHandle = FTimerHandle.create();

  /** the time player started this round */
  var RoundStartTime:Float32;


  /** true if round can be restarted after finishing */
  var bCanBeRestarted:Bool;

  /** current checkpoint times */
  var CurrentTimes:TArray<Float32> = TArray.create(); // In Haxe, structs need to be initialized as well, unless they are @:uexpose

  /** best checkpoint times */
  @:uexpose var BestTimes:TArray<Float32>;

  // All UObject-derived constructors take a `wrapped` argument, which should be passed to the `super` constructor
  public function new(wrapped) {
    super(wrapped);

    PlayerControllerClass = PlayerController.StaticClass();
    var PlayerPawnClass:FClassFinder<APawn> = FClassFinder.Find("/Game/Pawn/PlayerPawn");
    DefaultPawnClass = PlayerPawnClass.Class;
    HUDClass = HUD.StaticClass();

    MyGameState = Intro;

    // we do not need to initialize default values on the constructor
    if (UEngine.GEngine != null && UEngine.GEngine.GameViewport != null) {
      UEngine.GEngine.GameViewport.SetSuppressTransitionMessage(true);
    }
  }

  inline private function getPC():PlayerController {
    return UEngine.GEngine.GetFirstLocalPlayerController(GetWorld()).as(PlayerController);
  }

  /** prepare game state and show HUD message */
  @:uexpose public function PrepareRound(bRestarting = false):Void {
    if (bRestarting) {
      OnRoundFinished.Broadcast();
    }

    MyGameState = bRestarting ? Restarting : Waiting;
    RoundWasWon = false;
    RoundStartTime = 0;

    var PC = getPC();
    var Pawn = PC != null ? PC.GetPawn().as(Character) : null;
    if (Pawn != null) {
      Pawn.OnRoundReset();

      var StartSpot = FindPlayerStart(PC, "");
      Pawn.TeleportTo(StartSpot.GetActorLocation(), StartSpot.GetActorRotation(), false, false);

      if (Pawn.bHidden) {
        Pawn.SetActorHiddenInGame(false);
      }
    }
  }

  /** used to start this round */
  @:uexpose public function StartRound():Void {
    RoundStartTime = GetWorld().GetTimeSeconds();
    MyGameState = Playing;
  }

  /** finish current round */
  @:uexpose public function FinishRound():Void {
    MyGameState = Finished;

    // determine game state
    var LastCheckpointIdx = GetNumCheckpoints() - 1;
    var BestTime = GetBestCheckpointTime(LastCheckpointIdx);
    var RoundWasWon = (BestTime < 0) || (GetRoundDuration() < BestTime);

    // notify player
    var PC = getPC(),
        Pawn = PC != null ? PC.GetPawn().as(Character) : null;
    if (Pawn != null)
    {
      Pawn.OnRoundFinished();
    }

    // update best checkpoint times
    while (LastCheckpointIdx >= BestTimes.Num())
    {
      BestTimes.Push(-1);
    }

    for (i in 0...BestTimes.Num())
    {
      if ((BestTimes[i] < 0) || (BestTimes[i] > CurrentTimes[i]))
      {
        BestTimes[i] = CurrentTimes[i];
      }
    }
  }

  /** pauses/unpauses the game */
  private function set_IsGamePaused(value:Bool) {
    var PC = getPC();
    PC.SetPause(value);
    return IsGamePaused = value;
  }

  @:uexpose public function SetGamePaused(value:Bool) {
    IsGamePaused = value;
  }

  /** sets if round can be restarted */
  @:uexpose public function SetCanBeRestarted(bAllowRestart:Bool):Void {
    if (MyGameState == Finished) {
      bCanBeRestarted = bAllowRestart;
    }
  }

  /** returns if round can be restarted */
  @:uexpose @:thisConst public function CanBeRestarted():Bool {
    return MyGameState == Finished && bCanBeRestarted;
  }

  /** tries to restart round */
  @:uexpose public function TryRestartRound():Void {
    if (CanBeRestarted()) {
      PrepareRound(true);
      GetWorldTimerManager().SetTimerWithUObject(TimerHandle_StartRound, this, MethodPointer.fromMethod(StartRound), 2, false, -1);
      bCanBeRestarted = false;
    }
  }

  /** save current time for checkpoint */
  @:uexpose public function SaveCheckpointTime(CheckpointID:Int32):Void {
    if (CheckpointID < 0) {
      return;
    }

    while (CheckpointID >= CurrentTimes.Num()) {
      CurrentTimes.Push(-1);
    }

    CurrentTimes[CheckpointID] = GetRoundDuration();
  }

  /** get checkpoint time: current round */
  @:uexpose @:thisConst public function GetCurrentCheckpointTime(CheckpointID:Int32):Float32 {
    return CheckpointID >= 0 && CheckpointID < CurrentTimes.Num() ? CurrentTimes[CheckpointID] : -1;
  }

  /** get checkpoint time: best */
  @:uexpose public function GetBestCheckpointTime(CheckpointID:Int32):Float32 {
    return CheckpointID >= 0 && CheckpointID < BestTimes.Num() ? BestTimes[CheckpointID] : -1;
  }

  /** get number of checkpoints */
  @:uexpose @:thisConst public function GetNumCheckpoints():Int32 {
    var i1 = CurrentTimes.Num(),
        i2 = BestTimes.Num();
    return i1 > i2 ? i1 : i2;
  }

  /**
   * returns time that passed since round has started (in seconds)
   * if the round has already ended returns round duration
   */
  @:uexpose @:thisConst public function GetRoundDuration():Float32 {
    if (IsRoundInProgress()) {
      var CurrTime = GetWorld().GetTimeSeconds();
      return CurrTime - RoundStartTime;
    }

    var LastCheckpoint = GetNumCheckpoints() - 1;
    return GetCurrentCheckpointTime(LastCheckpoint);
  }

  /** increases/decreases round duration by DeltaTime */
  @:uexpose public function ModifyRoundDuration(DeltaTime:Float32, bIncrease:Bool):Void {
    if (IsRoundInProgress())
    {
      var PrevRoundStartTime = RoundStartTime;
      var Delta = Math.abs(DeltaTime);
      if (bIncrease)
      {
        RoundStartTime -= Delta;
      } else {
        var CurrTime = GetWorld().GetTimeSeconds();
        RoundStartTime += Delta;
        RoundStartTime = Math.min(RoundStartTime, CurrTime);
      }

      var PC = getPC();
      var HUD = PC != null ? PC.MyHUD.as(HUD) : null;
      if (HUD != null)
      {
        HUD.NotifyRoundTimeModified(PrevRoundStartTime - RoundStartTime);
      }
    }
  }

  /** returns true if round is in progress - player is still moving */
  @:uexpose @:thisConst public function IsRoundInProgress():Bool {
    return MyGameState == Playing;
  }

  @:uexpose public function GetGameState():EGameState {
    return MyGameState;
  }

  @:uexpose public function IsRoundWon():Bool {
    return RoundWasWon;
  }
}
