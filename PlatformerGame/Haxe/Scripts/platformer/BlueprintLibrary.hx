package platformer;
import unreal.*;

using unreal.CoreAPI;
using StringTools;

@:uclass
@:uname("UPlatformerBlueprintLibrary")
class BlueprintLibrary extends UBlueprintFunctionLibrary {

  static function GetGameFromContextObject(WorldContextObject:UObject):GameMode
  {
    var MyWorld = UEngine.GEngine.GetWorldFromContextObject(WorldContextObject, false);
    if (MyWorld == null) throw 'assert: MyWorld == null';

    var MyGame = MyWorld.GetAuthGameMode().as(GameMode);
    return MyGame;
  }

  /** switch to waiting state, player will be able to start race by himself */
  @:ufunction(BlueprintCallable, Category=Game, meta=(WorldContext="WorldContextObject"))
  public static function PrepareRace(WorldContextObject:UObject):Void {
    var MyGame = GetGameFromContextObject(WorldContextObject);
    if (MyGame != null)
    {
      MyGame.PrepareRound(false);
    }
  }

  /** finish round, player will be able to replay, returns true when scoring new record */
  @:ufunction(BlueprintCallable, Category=Game, meta=(WorldContext="WorldContextObject"))
  public static function FinishRace(WorldContextObject:UObject):Bool {
    var bHasWon = false;

    var MyGame = GetGameFromContextObject(WorldContextObject);
    if (MyGame != null)
    {
      MyGame.FinishRound();
      bHasWon = MyGame.IsRoundWon();
    }

    return bHasWon;
  }

  /** allow player to restart round */
  @:ufunction(BlueprintCallable, Category=Game, meta=(WorldContext="WorldContextObject"))
  public static function AllowToRestartRace(WorldContextObject:UObject):Void {
    var MyGame = GetGameFromContextObject(WorldContextObject);
    if (MyGame != null)
    {
      MyGame.SetCanBeRestarted(true);
    }
  }

  /** allows changing round duration, while round is still in progress - used to give players time bonuses */
  @:ufunction(BlueprintCallable, Category=Game, meta=(WorldContext="WorldContextObject"))
  public static function DecreaseRoundDuration(WorldContextObject:UObject, DeltaTime:Float32):Void {
    var MyGame = GetGameFromContextObject(WorldContextObject);
    if (MyGame != null)
    {
      var Delta = Math.abs(DeltaTime);
      MyGame.ModifyRoundDuration(Delta, DeltaTime < 0 ? false : true);
    }
  }

  /** returns delta between current checkpoint time and best (negative = better time) */
  @:ufunction(BlueprintCallable, Category=Game, meta=(WorldContext="WorldContextObject"))
  public static function MarkCheckpointTime(WorldContextObject:UObject, CheckpointID:Int32):Float32 {
    var DeltaTime = 0.0;

    var MyGame = GetGameFromContextObject(WorldContextObject);
    if (MyGame != null)
    {
      var PrevBestTime = MyGame.GetBestCheckpointTime(CheckpointID);

      MyGame.SaveCheckpointTime(CheckpointID);

      if (PrevBestTime > 0)
      {
        var CurrentTime = MyGame.GetCurrentCheckpointTime(CheckpointID);
        DeltaTime = (CurrentTime - PrevBestTime);
      }
    }

    return DeltaTime;
  }

  /** returns checkpoint time saved in current round */
  @:ufunction(BlueprintPure, Category=Game, meta=(WorldContext="WorldContextObject"))
  public static function GetCurrentCheckpointTime(WorldContextObject:UObject, CheckpointID:Int32):Float32 {
    var MyGame = GetGameFromContextObject(WorldContextObject);
    if (MyGame != null)
    {
      return MyGame.GetCurrentCheckpointTime(CheckpointID);
    }

    return -1.0;
  }

  /** returns best time on given checkpoint */
  @:ufunction(BlueprintPure, Category=Game, meta=(WorldContext="WorldContextObject"))
  public static function GetBestCheckpointTime(WorldContextObject:UObject, CheckpointID:Int32):Float32 {
    var MyGame = GetGameFromContextObject(WorldContextObject);
    if (MyGame != null)
    {
      return MyGame.GetBestCheckpointTime(CheckpointID);
    }

    return -1.0;
  }

  /** returns index of last saved checkpoint */
  @:ufunction(BlueprintPure, Category=Game, meta=(WorldContext="WorldContextObject"))
  public static function GetLastCheckpoint(WorldContextObject:UObject):Int32 {
    var MyGame = GetGameFromContextObject(WorldContextObject);
    if (MyGame != null)
    {
      return MyGame.GetNumCheckpoints() - 1;
    }

    return 0;
  }

  @:ufunction(BlueprintCallable, Category=HUD, meta=(WorldContext="WorldContextObject"))
  public static function DisplayMessage(WorldContextObject:UObject, Message:FString, DisplayDuration:Float32 = 1, PosX:Float32  = 0.5, PosY:Float32 = 0.5, TextScale:Float32 = 1.0, bRedBorder:Bool = false):Void {
    var LocalPC = UEngine.GEngine.GetFirstLocalPlayerController(UEngine.GEngine.GetWorldFromContextObject(WorldContextObject, false));
    var MyHUD = LocalPC != null ? LocalPC.GetHUD().as(HUD) : null;
    if (MyHUD != null)
    {
      MyHUD.AddMessage(Message, DisplayDuration, PosX, PosY, TextScale/4, bRedBorder);
    }
  }

  /** converts time to string in mm:ss.sss format */
  @:ufunction(BlueprintPure, Category=HUD)
  public static function DescribeTime(TimeSeconds:Float32, bShowSign:Bool = true):FString {
    var AbsTimeSeconds = Math.abs(TimeSeconds);
    var bIsNegative = (TimeSeconds < 0);

    var TotalSeconds = Std.int(AbsTimeSeconds) % 3600;
    var NumMinutes = Std.int(TotalSeconds / 60);
    var NumSeconds = Std.int(TotalSeconds % 60);

    var NumMiliSeconds = Std.int( (AbsTimeSeconds * 1000.0) % 1000 );

    var TimeDesc = (bShowSign ? (bIsNegative ? '-' : '+') : '') +
        Std.string(NumMinutes).lpad('0',2) + ':' +
        Std.string(NumSeconds).lpad('0',2) + '.' +
        Std.string(NumMiliSeconds).lpad('0',3);

    return TimeDesc;
  }

  /** displays specified texture covering entire screen */
  @:ufunction(BlueprintCallable, Category=HUD, meta=(WorldContext="WorldContextObject"))
  public static function ShowPicture(WorldContextObject:UObject, Picture:UTexture2D, FadeInTime:Float32 = 0.3, ScreenCoverage:Float32 = 1.0, bKeepAspectRatio:Bool = false):Void {
    var MyWorld = UEngine.GEngine.GetWorldFromContextObject(WorldContextObject, false);
    var MyGameMode = GetGameFromContextObject(WorldContextObject);

    if (MyGameMode != null)
    {
      if (MyGameMode.PlatformerPicture == null)
      {
        MyGameMode.PlatformerPicture = platformer.Picture.create(MyWorld);
      }
      MyGameMode.PlatformerPicture.Show(Picture, FadeInTime, ScreenCoverage, bKeepAspectRatio);
    }
  }

  /** hides previously displayed picture */
  @:ufunction(BlueprintCallable, Category=HUD, meta=(WorldContext="WorldContextObject"))
  public static function HidePicture(WorldContextObject:UObject, FadeOutTime:Float32 = 0.3):Void {
    var MyWorld = UEngine.GEngine.GetWorldFromContextObject(WorldContextObject, false);
    var MyGameMode = MyWorld.GetAuthGameMode().as(GameMode);

    if (MyGameMode != null && MyGameMode.PlatformerPicture != null)
    {
      MyGameMode.PlatformerPicture.Hide(FadeOutTime);
    }
  }

  /** shows highscore with provided data */
  @:ufunction(BlueprintCallable, Category=HUD, meta=(WorldContext="WorldContextObject"))
  public static function ShowHighscore(WorldContextObject:UObject, Times:TArray<Float32>, Names:TArray<FString> ):Void {
    var LocalPC = UEngine.GEngine.GetFirstLocalPlayerController(UEngine.GEngine.GetWorldFromContextObject(WorldContextObject, false));
    var MyHUD = LocalPC != null ? LocalPC.GetHUD().as(HUD) : null;
    if (MyHUD != null)
    {
      MyHUD.ShowHighscore(Times,Names);
    }
  }

  /** hides the highscore */
  @:ufunction(BlueprintCallable, Category=HUD, meta=(WorldContext="WorldContextObject"))
  public static function HideHighscore(WorldContextObject:UObject):Void {
    var LocalPC = UEngine.GEngine.GetFirstLocalPlayerController(UEngine.GEngine.GetWorldFromContextObject(WorldContextObject, false));
    var MyHUD = LocalPC != null ? LocalPC.GetHUD().as(HUD) : null;
    if (MyHUD != null)
    {
      MyHUD.HideHighscore();
    }
  }

  /** shows highscore prompt, calls HighscoreNameAccepted when user is done */
  @:ufunction(BlueprintCallable, Category=HUD, meta=(WorldContext="WorldContextObject"))
  public static function ShowHighscorePrompt(WorldContextObject:UObject):Void {
    var LocalPC = UEngine.GEngine.GetFirstLocalPlayerController(UEngine.GEngine.GetWorldFromContextObject(WorldContextObject, false));
    var MyHUD = LocalPC != null ? LocalPC.GetHUD().as(HUD) : null;
    if (MyHUD != null)
    {
      MyHUD.ShowHighscorePrompt();
    }
  }

  /*
   * Sorts this highscores.
   *
   * @param InTimes   Array of times to story
   * @param InNames   Names linked to each time
   * @param OutTimes  Sorted times
   * @param OutNames  Names sorted with times
   * @param MaxScores Limit output times to this amount
   */
  @:ufunction(BlueprintCallable, Category = Game)
  public static function SortHighscores(InTimes:TArray<Float32>, InNames:TArray<FString>, OutTimes:PRef<TArray<Float32>>, OutNames:PRef<TArray<FString>>, MaxScores:Int32):Void {
    var tmp = [ for (i in 0...InTimes.Num()) { time:InTimes[i], name:InNames[i] } ];
    tmp.sort(function(e1, e2) return Reflect.compare(e1.time, e2.time));
    OutTimes.Empty();
    OutNames.Empty();
    for (i in 0...tmp.length) {
      OutTimes.Push( tmp[i].time );
      OutNames.Push( tmp[i].name );
    }
  }
}
