package platformer;
import unreal.*;

@:glueCppIncludes("Private/UI/Menu/PlatformerIngameMenu.h")
@:uextern extern class FPlatformerIngameMenu {
  @:uname("new") static function createNew():POwnedPtr<FPlatformerIngameMenu>;
  function MakeMenu(pc:PlayerController):Void;
  function ToggleGameMenu():Void;
}

