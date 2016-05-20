package platformer;
import unreal.*;

// This extern definition allows us to reference code that is defined in C++ in Haxe. As we start
// porting the C++ code to Haxe, we will be able to safely delete those
@:glueCppIncludes("Private/UI/Widgets/FPlatformerPicture.h")
@:uname("FPlatformerPicture")
@:umodule("PlatformerGame")
@:uextern extern class Picture {
  @:uname(".ctor") static function create(world:UWorld):Picture;

  function Show(Picture:UTexture2D, FadeInTime:Float32, ScreenCoverage:Float32, bKeepAspectRatio:Bool):Void;

  function Hide(FadeOutTime:Float32):Void;
}
