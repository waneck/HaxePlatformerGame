package platformer;
import unreal.*;

using unreal.CoreAPI;
using unreal.FVectorUtils;

@:uclass(Abstract)
@:uname("APlatformerCharacter")
// the following line replaces the FObjectInitializer constructor argument
// e.g. : Super(ObjectInitializer.SetDefaultSubobjectClass<UPlatformerPlayerMovementComp>(ACharacter::CharacterMovementComponentName))
// (see original C++ code)
@:uoverrideSubobject(ACharacter.CharacterMovementComponentName, platformer.PlayerMovementComp)
class Character extends ACharacter {
  public function new(wrapped) {
    super(wrapped);
    MinSpeedForHittingWall = 200;
    GetMesh().MeshComponentUpdateFlag = AlwaysTickPoseAndRefreshBones;
  }

  /** player pawn initialization */
  override public function PostInitializeComponents():Void {
    super.PostInitializeComponents();

    // setting initial rotation
    SetActorRotation(FRotator.createWithValues(0,0,0), None);
  }

  /** perform position adjustments */
  override public function Tick(DeltaSeconds:Float32):Void {
    // decrease anim position adjustment
    if (!AnimPositionAdjustment.IsNearlyZero())
    {
      AnimPositionAdjustment = FMath.VInterpConstantTo(AnimPositionAdjustment, FVector.ZeroVector, DeltaSeconds, 400.0);
      GetMesh().SetRelativeLocation(GetBaseTranslationOffset() + AnimPositionAdjustment, false, null, None);
    }

    if (ClimbToMarker != null)
    {
      // correction in case climb marker is moving
      var AdjustDelta = ClimbToMarker.GetComponentLocation().subeq(ClimbToMarkerLocation);
      if (!AdjustDelta.IsZero())
      {
        SetActorLocation(GetActorLocation().addeq(AdjustDelta), false, null);
        ClimbToMarkerLocation.addeq(AdjustDelta);
      }
    }

    super.Tick(DeltaSeconds);
  }

  /** setup input bindings */
  override public function SetupPlayerInputComponent(InputComponent:UInputComponent):Void {
    InputComponent.BindAction("Jump", IE_Pressed, this, MethodPointer.fromMethod(OnStartJump));
    InputComponent.BindAction("Jump", IE_Released, this, MethodPointer.fromMethod(OnStopJump));
    InputComponent.BindAction("Slide", IE_Pressed, this, MethodPointer.fromMethod(OnStartSlide));
    InputComponent.BindAction("Slide", IE_Released, this, MethodPointer.fromMethod(OnStopSlide));
  }

  /** used to make pawn jump ; overridden to handle additional jump input functionality */
  override public function CheckJumpInput(DeltaTime:Float32):Void {
    if (bPressedJump)
    {
      var MoveComp = GetCharacterMovement().as(PlayerMovementComp);
      if (MoveComp != null && MoveComp.IsSliding())
      {
        MoveComp.TryToEndSlide();
        return;
      }
    }

    super.CheckJumpInput(DeltaTime);
  }

  /** notify from movement about hitting an obstacle while running */
  override public function MoveBlockedBy(Impact:Const<PRef<FHitResult>>):Void {
    var ForwardDot = FVector.DotProduct(Impact.Normal, FVector.ForwardVector);
    if (GetCharacterMovement().MovementMode != MOVE_None)
    {
      trace('Collision with ${Impact.Actor}, normal=(${Impact.Normal.X}, ${Impact.Normal.Y}, ${Impact.Normal.Z}), dot=$ForwardDot, ${GetCharacterMovement().GetMovementName()}');
    }

    if (GetCharacterMovement().MovementMode == MOVE_Walking && ForwardDot < -0.9)
    {
      var MyMovement = GetCharacterMovement().as(PlayerMovementComp);
      var Speed = Math.abs(FVector.DotProduct(MyMovement.Velocity, FVector.ForwardVector));
      // if running or sliding: play bump reaction and jump over obstacle

      var Duration = 0.01;
      if (Speed > MinSpeedForHittingWall)
      {
        Duration = PlayAnimMontage(HitWallMontage, 1, UnrealName.NAME_None);
      }
      GetWorldTimerManager().SetTimerWithUObject(TimerHandle_ClimbOverObstacle, this, MethodPointer.fromMethod(ClimbOverObstacle), Duration, false, -1);
      MyMovement.PauseMovementForObstacleHit();
    }
    else if (GetCharacterMovement().MovementMode == MOVE_Falling)
    {
      // if in mid air: try climbing to hit marker
      // on C++ code, we needed to call Get() on this actor, since it's a TWeakObjectPointer. In Haxe you can access it directly
      var Marker = Impact.Actor.as(ClimbMarker);
      if (Marker != null)
      {
        ClimbToLedge(Marker);

        var MyMovement = GetCharacterMovement().as(PlayerMovementComp);
        MyMovement.PauseMovementForLedgeGrab();
      }
    }
  }

  /** play end of round if game has finished with character in mid air */
  override public function Landed(Hit:Const<PRef<FHitResult>>):Void {
    super.Landed(Hit);

    var MyGame = GetWorld().GetAuthGameMode().as(GameMode);
    if (MyGame != null && MyGame.GetGameState() == Finished)
    {
      PlayRoundFinished();
    }
  }

  /** try playing end of round animation */
  public function OnRoundFinished():Void {
    // don't stop in mid air, will be continued from Landed() notify
    if (GetCharacterMovement().MovementMode != MOVE_Falling)
    {
      PlayRoundFinished();
    }
  }

  /** stop any active animations, reset movement state */
  public function OnRoundReset():Void {
    // reset animations
    if (GetMesh() != null && GetMesh().AnimScriptInstance != null)
    {
      GetMesh().AnimScriptInstance.Montage_Stop(0.0, null);
    }

    // reset movement properties
    GetCharacterMovement().StopMovementImmediately();
    GetCharacterMovement().SetMovementMode(MOVE_Walking, 0);
    bPressedJump = false;
    bPressedSlide = false;
  }

  /** returns true when pawn is sliding ; used in AnimBlueprint */
  @:ufunction(BlueprintCallable, Category="Pawn|Character")
  @:thisConst public function IsSliding():Bool {
    var MoveComp = GetCharacterMovement().as(PlayerMovementComp);
    return MoveComp != null && MoveComp.IsSliding();
  }

  /** gets bPressedSlide value */
  @:uexpose @:thisConst public function WantsToSlide():Bool {
    return bPressedSlide;
  }

  /** event called when player presses jump button */
  @:uexpose public function OnStartJump():Void {
    var MyGame = GetWorld().GetAuthGameMode().as(GameMode);
    var MyPC = Controller.as(PlayerController);
    if (MyPC != null)
    {
      if (MyPC.TryStartingGame())
      {
        return;
      }

      if (!MyPC.IsMoveInputIgnored() &&
          MyGame != null && MyGame.IsRoundInProgress())
      {
        bPressedJump = true;
      }
    }
  }

  /** event called when player releases jump button */
  @:uexpose public function OnStopJump():Void {
    bPressedJump = false;
  }

  /** event called when player presses slide button */
  @:uexpose public function OnStartSlide():Void {
    var MyGame = GetWorld().GetAuthGameMode().as(GameMode);
    var MyPC = Controller.as(PlayerController);
    if (MyPC != null)
    {
      if (MyPC.TryStartingGame())
      {
        return;
      }

      if (!MyPC.IsMoveInputIgnored() &&
          MyGame != null && MyGame.IsRoundInProgress())
      {
        bPressedSlide = true;
      }
    }
  }

  /** event called when player releases slide button */
  @:uexpose public function OnStopSlide():Void {
    bPressedSlide = false;
  }

  /** handle effects when slide starts */
  @:uexpose public function PlaySlideStarted():Void {
    if (SlideSound != null)
    {
      // C++ definition:  static class UAudioComponent* SpawnSoundAttached(class USoundBase* Sound, class USceneComponent* AttachToComponent, FName AttachPointName = NAME_None, FVector Location = FVector(ForceInit), FRotator Rotation = FRotator::ZeroRotator, EAttachLocation::Type LocationType = EAttachLocation::KeepRelativeOffset, bool bStopWhenAttachedToDestroyed = false, float VolumeMultiplier = 1.f, float PitchMultiplier = 1.f, float StartTime = 0.f, class USoundAttenuation*
      // AttenuationSettings = nullptr, class USoundConcurrency* ConcurrencySettings = nullptr);

      SlideAC = UGameplayStatics.SpawnSoundAttached(SlideSound, GetMesh(), UnrealName.NAME_None, new FVector(0,0,0), FRotator.ZeroRotator, KeepRelativeOffset, false, 1, 1, 0, null, null);
    }
  }

  /** handle effects when slide is finished */
  @:uexpose public function PlaySlideFinished():Void
  {
    if (SlideAC != null)
    {
      SlideAC.Stop();
      SlideAC = null;
    }
  }

  /** gets CameraHeightChangeThreshold value */
  @:uexpose @:thisConst public function GetCameraHeightChangeThreshold():Float32 {
    return CameraHeightChangeThreshold;
  }

  /**
   * Camera is fixed to the ground, even when player jumps.
   * But, if player jumps higher than this threshold, camera will start to follow.
   */
  @:uproperty(EditDefaultsOnly, Category=Config)
  var CameraHeightChangeThreshold:Float32;

  /** animation for winning game */
  @:uproperty(EditDefaultsOnly, Category=Animation)
  var WonMontage:UAnimMontage;

  /** animation for loosing game */
  @:uproperty(EditDefaultsOnly, Category=Animation)
  var LostMontage:UAnimMontage;

  /** animation for running into an obstacle */
  @:uproperty(EditDefaultsOnly, Category=Animation)
  var HitWallMontage:UAnimMontage;

  /** minimal speed for pawn to play hit wall animation */
  @:uproperty(EditDefaultsOnly, Category=Animation)
  var MinSpeedForHittingWall:Float32;

  /** animation for climbing over small obstacle */
  @:uproperty(EditDefaultsOnly, Category=Animation)
  var ClimbOverSmallMontage:UAnimMontage;

  /** height of small obstacle */
  @:uproperty(EditDefaultsOnly, Category=Animation)
  var ClimbOverSmallHeight:Float32;

  /** animation for climbing over mid obstacle */
  @:uproperty(EditDefaultsOnly, Category=Animation)
  var ClimbOverMidMontage:UAnimMontage;

  /** height of mid obstacle */
  @:uproperty(EditDefaultsOnly, Category=Animation)
  var ClimbOverMidHeight:Float32;

  /** animation for climbing over big obstacle */
  @:uproperty(EditDefaultsOnly, Category=Animation)
  var ClimbOverBigMontage:UAnimMontage;

  /** height of big obstacle */
  @:uproperty(EditDefaultsOnly, Category=Animation)
  var ClimbOverBigHeight:Float32;

  /** animation for climbing to ledge */
  @:uproperty(EditDefaultsOnly, Category=Animation)
  var ClimbLedgeMontage:UAnimMontage;

  /** root offset in climb legde animation */
  @:uproperty(EditDefaultsOnly, Category=Animation)
  var ClimbLedgeRootOffset:FVector;

  /** grab point offset along X axis in climb legde animation */
  @:uproperty(EditDefaultsOnly, Category=Animation)
  var ClimbLedgeGrabOffsetX:Float32;

  /** mesh translation used for position adjustments */
  var AnimPositionAdjustment:FVector = new FVector(0,0,0); // unlike C++, you need to make a default value for structs, otherwise they will be null

  /** root motion translation from previous tick */
  var PrevRootMotionPosition:FVector = new FVector(0,0,0);

  /** looped slide sound */
  @:uproperty(EditDefaultsOnly, Category=Sound)
  var SlideSound:USoundCue;

  /** audio component playing looped slide sound */
  @:uproperty()
  var SlideAC:UAudioComponent;

  /** true when player is holding slide button */
  var bPressedSlide:Bool;

  /** ClimbMarker (or to be exact its mesh component - the movable part) we are climbing to */
  @:uproperty()
  var ClimbToMarker:UStaticMeshComponent;

  /** location of ClimbMarker we are climbing to */
  var ClimbToMarkerLocation:FVector = new FVector(0,0,0);

  /** Handle for efficient management of ClimbOverObstacle timer */
  var TimerHandle_ClimbOverObstacle:FTimerHandle = new FTimerHandle();

  /** Handle for efficient management of ResumeMovement timer */
  var TimerHandle_ResumeMovement:FTimerHandle = new FTimerHandle();

  /** determine obstacle height type and play animation */
  @:uexpose private function ClimbOverObstacle():Void {
    // climbing over obstacle:
    // - there are three animations matching with three types of predefined obstacle heights
    // - pawn is moved using root motion, ending up on top of obstacle as animation ends

    var ForwardDir = GetActorForwardVector();
    var TraceStart = GetActorLocation() + ForwardDir * 150.0 + new FVector(0,0,1) * (GetCapsuleComponent().GetScaledCapsuleHalfHeight() + 150.0);
    var TraceEnd = TraceStart + new FVector(0,0,-1) * 500.0;

    var TraceParams = FCollisionQueryParams.createWithParams("", true, null);

    // we need to create a new FHitResult to pass to `LineTraceSingleByChannel` - which takes a reference
    // the original C++ code was just: `FHitResult Hit;`. This however wouldn't work in Haxe, because
    // everything is still being passed by reference
    var Hit = new FHitResult(ForceInit);
    GetWorld().LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Pawn, TraceParams);

    if (Hit.bBlockingHit)
    {
      var DestPosition = Hit.ImpactPoint + new FVector(0, 0, GetCapsuleComponent().GetScaledCapsuleHalfHeight());
      var ZDiff = DestPosition.Z - GetActorLocation().Z;
      trace('Climb over obstacle. Z difference: $ZDiff (${
        ZDiff < ClimbOverMidHeight ? "small" : (ZDiff < ClimbOverBigHeight ? "mid" : "big")})');

      var Montage = (ZDiff < ClimbOverMidHeight) ? ClimbOverSmallMontage : (ZDiff < ClimbOverBigHeight) ? ClimbOverMidMontage : ClimbOverBigMontage;

      // set flying mode since it needs Z changes. If Walking or Falling, we won't be able to apply Z changes
      // this gets reset in the ResumeMovement
      GetCharacterMovement().SetMovementMode(MOVE_Flying, 0);
      SetActorEnableCollision(false);
      var Duration = PlayAnimMontage(Montage, 1, UnrealName.NAME_None);
      GetWorldTimerManager().SetTimerWithUObject(TimerHandle_ResumeMovement, this, MethodPointer.fromMethod(ResumeMovement), Duration - 0.1, false, -1);
    }
    else
    {
      // shouldn't happen
      ResumeMovement();
    }
  }

  /** position pawn on ledge and play animation with position adjustment */
  private function ClimbToLedge(MoveToMarker:PPtr<Const<ClimbMarker>>):Void {
    ClimbToMarker = MoveToMarker != null ? cast MoveToMarker.GetComponentByClass(UStaticMeshComponent.StaticClass()) : null;
    ClimbToMarkerLocation = ClimbToMarker != null ? ClimbToMarker.GetComponentLocation() : FVector.ZeroVector;

    // place on top left corner of marker, but preserve current Y coordinate
    var MarkerBox = MoveToMarker.GetMesh().Bounds.GetBox();
    var DesiredPosition = new FVector(MarkerBox.Min.X, GetActorLocation().Y, MarkerBox.Max.Z);

    // climbing to ledge:
    // - pawn is placed on top of ledge (using ClimbLedgeGrabOffsetX to offset from grab point) immediately
    // - AnimPositionAdjustment modifies mesh relative location to smooth transition
    //   (mesh starts roughly at the same position, additional offset quickly decreases to zero in Tick)

    var StartPosition = GetActorLocation();
    var AdjustedPosition = DesiredPosition.copy(); // we need to copy this, because in Haxe everything is passed by reference
    AdjustedPosition.X += (ClimbLedgeGrabOffsetX * GetMesh().RelativeScale3D.X) - GetBaseTranslationOffset().X;
    AdjustedPosition.Z += GetCapsuleComponent().GetScaledCapsuleHalfHeight();

    TeleportTo(AdjustedPosition, GetActorRotation(), false, true);

    AnimPositionAdjustment = StartPosition - (GetActorLocation() - (ClimbLedgeRootOffset * GetMesh().RelativeScale3D));
    GetMesh().SetRelativeLocation(GetBaseTranslationOffset() + AnimPositionAdjustment, false, null, None);

    var Duration = PlayAnimMontage(ClimbLedgeMontage, 1, UnrealName.NAME_None);
    GetWorldTimerManager().SetTimerWithUObject(TimerHandle_ResumeMovement, this, MethodPointer.fromMethod(ResumeMovement), Duration - 0.1, false, -1);
  }

  /** restore pawn's movement state */
  // we need uexpose here because we are using MethodPointer.fromMethod of this
  @:uexpose private function ResumeMovement():Void {
    SetActorEnableCollision(true);

    // restore movement state and saved speed
    var MyMovement = GetCharacterMovement().as(PlayerMovementComp);
    MyMovement.RestoreMovement();

    ClimbToMarker = null;
  }

  /** play end of round animation */
  private function PlayRoundFinished():Void {
    var MyGame = GetWorld().GetAuthGameMode().as(GameMode);
    var bWon = MyGame != null && MyGame.IsRoundWon();

    PlayAnimMontage(bWon ? WonMontage : LostMontage, 1, UnrealName.NAME_None);

    GetCharacterMovement().StopMovementImmediately();
    GetCharacterMovement().DisableMovement();
  }
}
