package platformer;
import unreal.*;
import unreal.gamemenubuilder.IGameMenuBuilderModule;

using unreal.CoreAPI;

@:uclass
@:uname("APlatformerPlayerController")
class PlayerController extends APlayerController {
  public function new(wrapper) {
    super(wrapper);

    PlayerCameraManagerClass = platformer.PlayerCameraManager.StaticClass();
    bEnableClickEvents = true;
    bEnableTouchEvents = true;
  }

  override public function PostInitializeComponents() {
    super.PostInitializeComponents();

    //Build menu only after game is initialized
    // @note Initialize in FPlatformerGameModule::StartupModule is not enough - it won't execute in cooked game
    IGameMenuBuilderModule.Get();
    PlatformerIngameMenu = FPlatformerIngameMenu.createNew().toSharedPtr();
    PlatformerIngameMenu.Get().MakeMenu(this);

  }

  @:ufunction(BlueprintCallable, Category="Game")
  public function TryStartingGame():Bool {
    var MyGame = GetWorld().GetAuthGameMode().as(GameMode);
    if (MyGame != null)
    {
      var GameState = MyGame.GetGameState();
      switch (GameState)
      {
        case Waiting:
          MyGame.StartRound();
          return true;
        case Finished:
          MyGame.TryRestartRound();
          return true;
        case _:
      }
    }

    return false;
  }

  @:uexpose function OnToggleInGameMenu() {
    var MyGame = GetWorld().GetAuthGameMode().as(GameMode);
    if (PlatformerIngameMenu.IsValid() && MyGame != null && MyGame.GetGameState() != Finished)
    {
      PlatformerIngameMenu.Get().ToggleGameMenu();
    }
  }

  var PlatformerIngameMenu:TSharedPtr<FPlatformerIngameMenu>;

  override function SetupInputComponent() {
    super.SetupInputComponent();

    // UI input
    InputComponent.BindAction("InGameMenu", IE_Pressed, this, MethodPointer.fromMethod(OnToggleInGameMenu));
  }
}
