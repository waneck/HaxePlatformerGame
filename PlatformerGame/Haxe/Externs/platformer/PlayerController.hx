package platformer;
import unreal.*;

// This extern definition allows us to reference code that is defined in C++ in Haxe. As we start
// porting the C++ code to Haxe, we will be able to safely delete those
@:glueCppIncludes("Player/PlatformerPlayerController.h")
@:uname("APlatformerPlayerController")
@:umodule("PlatformerGame")
@:uextern extern class PlayerController extends APlayerController {
  function TryStartingGame():Bool;
}

