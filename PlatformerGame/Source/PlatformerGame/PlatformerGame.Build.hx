import unrealbuildtool.*;
using Helpers;

class PlatformerGame extends HaxeModuleRules {
  public function new(target) {
    super(target);

    PublicDependencyModuleNames.Add("GameMenuBuilder");
    PrivateDependencyModuleNames.addRange(["PlatformerGameLoadingScreen", "Slate", "SlateCore"]);
    PrivateIncludePaths.Add("PlatformerGame/Private/UI/Menu");
  }

  override private function getConfig():HaxeModuleConfig {
    // add your custom configuration here
    return super.getConfig();
  }
}
