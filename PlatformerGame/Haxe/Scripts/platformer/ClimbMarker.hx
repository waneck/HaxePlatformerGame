package platformer;
import unreal.*;

@:uclass
@:uname("APlatformerClimbMarker")
class ClimbMarker extends AActor {
  @:uproperty(VisibleDefaultsOnly, BlueprintReadOnly, Category=Mesh, meta=[AllowPrivateAccess="true"])
  var Mesh:UStaticMeshComponent;

  public function new(wrapped) {
    super(wrapped);

    var ClimbMarkerOb:FObjectFinder<UStaticMesh> = FObjectFinder.Find("/Game/Environment/meshes/ClimbMarker");

    var init = unreal.FObjectInitializer.Get();
    var SceneComp = init.CreateDefaultSubobject(new TypeParam<USceneComponent>(), this, "SceneComp", false);
    RootComponent = SceneComp;

    Mesh = init.CreateDefaultSubobject(new TypeParam<UStaticMeshComponent>(), this, "ClimbMesh", false);
    Mesh.SetStaticMesh(ClimbMarkerOb.Object);
    Mesh.RelativeScale3D = new FVector(0.25, 1.0, 0.25);
    Mesh.SetupAttachment(SceneComp, FName.createInt(NAME_None));
  }

  inline public function GetMesh() {
    return Mesh;
  }
}
