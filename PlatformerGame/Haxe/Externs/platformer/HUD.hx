package platformer;
import unreal.*;

@:glueCppIncludes("UI/PlatformerHUD.h")
@:uname("APlatformerHUD")
@:umodule("PlatformerGame")
@:uextern extern class HUD extends AHUD {
  function NotifyRoundTimeModified(DeltaTime:Float32):Void;
  function AddMessage(message:FString, displayDuration:Float32, posX:Float32, posY:Float32, textScale:Float32, bRedBorder:Bool):Void;
  function ShowHighscore(Times:TArray<Float32>, Names:TArray<FString>):Void;
  function HideHighscore():Void;
  function ShowHighscorePrompt():Void;
}


