package platformer;
import unreal.*;

@:glueCppIncludes("UI/PlatformerHUD.h")
@:uname("APlatformerHUD")
@:umodule("PlatformerGame")
@:uextern extern class HUD extends AHUD {
  function NotifyRoundTimeModified(DeltaTime:Float32):Void;
}


