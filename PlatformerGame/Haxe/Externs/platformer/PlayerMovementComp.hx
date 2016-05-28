package platformer;
import unreal.*;

// This extern definition allows us to reference code that is defined in C++ in Haxe. As we start
// porting the C++ code to Haxe, we will be able to safely delete those
@:glueCppIncludes("Player/PlatformerPlayerMovementComp.h")
@:uname("UPlatformerPlayerMovementComp")
@:umodule("PlatformerGame")
@:uextern extern class PlayerMovementComp extends UCharacterMovementComponent {
  function IsSliding():Bool;
  function TryToEndSlide():Void;
  function RestoreMovement():Void;
  function PauseMovementForObstacleHit():Void;
  function PauseMovementForLedgeGrab():Void;
}
