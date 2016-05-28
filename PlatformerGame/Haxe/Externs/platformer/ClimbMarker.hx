package platformer;
import unreal.*;

// This extern definition allows us to reference code that is defined in C++ in Haxe. As we start
// porting the C++ code to Haxe, we will be able to safely delete those
@:glueCppIncludes("UI/PlatformerClimbMarker.h")
@:uname("APlatformerClimbMarker")
@:umodule("PlatformerGame")
@:uextern extern class ClimbMarker extends AActor {
  function GetMesh():UStaticMeshComponent;
}

