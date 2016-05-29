package platformer;
import unreal.*;

using unreal.CoreAPI;

@:uclass
@:uname("UPlatformerPlayerMovementComp")
class PlayerMovementComp extends UCharacterMovementComponent {
  public function new(wrapped) {
    super(wrapped);

    MaxAcceleration = 200.0;
    BrakingDecelerationWalking = MaxAcceleration;
    MaxWalkSpeed = 900.0;

    SlideVelocityReduction = 30.0;
    SlideHeight = 60.0;
    SlideMeshRelativeLocationOffset = new FVector(0.0, 0.0, 34.0);
    bWantsSlideMeshRelativeLocationOffset = true;
    MinSlideSpeed = 200.0;
    MaxSlideSpeed = MaxWalkSpeed + 200.0;

    ModSpeedObstacleHit = 0.0;
    ModSpeedLedgeGrab = 0.8;
  }

  /** stop slide when falling */
  override public function StartFalling(Iterations:Int, remainingTime:Float32, timeTick:Float32, Delta:Const<PRef<FVector>>, subLoc:Const<PRef<FVector>>) {
    super.StartFalling(Iterations, remainingTime, timeTick, Delta, subLoc);

    if (MovementMode == MOVE_Falling && IsSliding())
    {
      TryToEndSlide();
    }
  }

  /** returns true when pawn is sliding */
  public function IsSliding():Bool {
    return bInSlide;
  }

  /** attempts to end slide move - fails if collisions above pawn don't allow it */
  public function TryToEndSlide() {
    // end slide if collisions allow
    if (bInSlide)
    {
      if (RestoreCollisionHeightAfterSlide())
      {
        bInSlide = false;

        var MyOwner = PawnOwner.as(Character);
        if (MyOwner != null)
        {
          MyOwner.PlaySlideFinished();
        }
      }
    }
  }

  /** stop movement and save current speed with obstacle modifier */
  public function PauseMovementForObstacleHit() {
    SavedSpeed = Velocity.Size() * ModSpeedObstacleHit;

    StopMovementImmediately();
    DisableMovement();
    TryToEndSlide();
  }

  /** stop movement and save current speed with ledge grab modifier */
  public function PauseMovementForLedgeGrab() {
    SavedSpeed = Velocity.Size() * ModSpeedLedgeGrab;

    StopMovementImmediately();
    DisableMovement();
    TryToEndSlide();
  }

  /** restore movement and saved speed */
  public function RestoreMovement() {
    SetMovementMode(MOVE_Walking, 0);

    if (SavedSpeed > 0)
    {
      Velocity = PawnOwner.GetActorForwardVector() * SavedSpeed;
    }
  }

  /** update slide */
  override function PhysWalking(deltaTime:Float32, Iterations:Int32) {
    var MyPawn = PawnOwner.as(Character);
    if (MyPawn != null)
    {
      var bWantsToSlide = MyPawn.WantsToSlide();
      if (IsSliding())
      {
        CalcCurrentSlideVelocityReduction(deltaTime);
        CalcSlideVelocity(Velocity);

        var CurrentSpeedSq = Velocity.SizeSquared();
        if (CurrentSpeedSq <= Math.pow(MinSlideSpeed, 2))
        {
          // slide has min speed - try to end it
          TryToEndSlide();
        }
      }
      else if (bWantsToSlide)
      {
        if (!IsFlying() &&
            Velocity.SizeSquared() > Math.pow(MinSlideSpeed * 2, 2)) // make sure pawn has some velocity
        {
          StartSlide();
        }
      }
    }

    super.PhysWalking(deltaTime, Iterations);
  }

  /** force movement */
  override function ScaleInputAcceleration(InputAcceleration:Const<PRef<FVector>>):FVector {
    var NewAccel = InputAcceleration.copy();

    var PlatGameMode = GetWorld().GetAuthGameMode().as(GameMode);
    if (PlatGameMode != null && PlatGameMode.IsRoundInProgress())
    {
      NewAccel.X = 1.0;
    }

    return super.ScaleInputAcceleration(NewAccel);
  }

  /** calculates OutVelocity which is new velocity for pawn during slide */
  function CalcSlideVelocity(OutVelocity:PRef<FVector>) {
    var VelocityDir:FVector = Velocity.GetSafeNormal();
    var NewVelocity = Velocity + (VelocityDir * CurrentSlideVelocityReduction);

    var NewSpeedSq = NewVelocity.SizeSquared();
    if (NewSpeedSq > Math.pow(MaxSlideSpeed, 2))
    {
      NewVelocity = VelocityDir * MaxSlideSpeed;
    }
    else if (NewSpeedSq < Math.pow(MinSlideSpeed, 2))
    {
      NewVelocity = VelocityDir * MinSlideSpeed;
    }

    OutVelocity.X = NewVelocity.X;
    OutVelocity.Y = NewVelocity.Y;
    OutVelocity.Z = NewVelocity.Z;
  }

  /** while pawn is sliding calculates new value of CurrentSlideVelocityReduction */
  function CalcCurrentSlideVelocityReduction(DeltaTime:Float32) {
    var ReductionCoef = 0.0;

    var FloorDotVelocity = FVector.DotProduct(CurrentFloor.HitResult.ImpactNormal, Velocity.GetSafeNormal());
    var bNeedsSlopeAdjustment = (FloorDotVelocity != 0.0);

    if (bNeedsSlopeAdjustment)
    {
      var Multiplier = 1.0 + Math.abs(FloorDotVelocity);
      if (FloorDotVelocity > 0.0)
      {
        ReductionCoef += SlideVelocityReduction * Multiplier; // increasing speed when sliding down a slope
      }
      else
      {
        ReductionCoef -= SlideVelocityReduction * Multiplier; // reducing speed when sliding up a slope
      }
    }
    else
    {
      ReductionCoef -= SlideVelocityReduction; // reducing speed on flat ground
    }

    var TimeDilation = GetWorld().GetWorldSettings(false,false).GetEffectiveTimeDilation();
    CurrentSlideVelocityReduction += ReductionCoef * TimeDilation * DeltaTime;
  }

  // /** handles pawn slide move */
  // function HandleSlide(DeltaTime:Float32) {
  // }

  /** forces pawn to start sliding */
  function StartSlide() {
    if (!bInSlide)
    {
      bInSlide = true;
      CurrentSlideVelocityReduction = 0.0;
      SetSlideCollisionHeight();

      var MyOwner = PawnOwner.as(Character);
      if (MyOwner != null)
      {
        MyOwner.PlaySlideStarted();
      }
    }
  }

  /** changes pawn height to SlideHeight and adjusts pawn collisions */
  function SetSlideCollisionHeight() {
    if (CharacterOwner == null || SlideHeight <= 0.0)
    {
      return;
    }

    // Do not perform if collision is already at desired size.
    if (CharacterOwner.GetCapsuleComponent().GetUnscaledCapsuleHalfHeight() == SlideHeight)
    {
      return;
    }

    // Change collision size to new value
    CharacterOwner.GetCapsuleComponent().SetCapsuleSize(CharacterOwner.GetCapsuleComponent().GetUnscaledCapsuleRadius(), SlideHeight, true);

    // applying correction to PawnOwner mesh relative location
    if (bWantsSlideMeshRelativeLocationOffset)
    {
      var DefCharacter = CharacterOwner.GetClass().GetDefaultObject(new TypeParam<ACharacter>());
      var Correction = DefCharacter.GetMesh().RelativeLocation + SlideMeshRelativeLocationOffset;
      CharacterOwner.GetMesh().SetRelativeLocation(Correction, false, null, None);
    }
  }

  /**
   * restores pawn height to default after slide, if collisions above pawn allow that
   * returns true if height change succeeded, false otherwise
   */
  function RestoreCollisionHeightAfterSlide():Bool {
    var CharacterOwner = PawnOwner.as(ACharacter);
    if (CharacterOwner == null || UpdatedPrimitive == null)
    {
      return false;
    }

    var DefCharacter = CharacterOwner.GetClass().GetDefaultObject(new TypeParam<ACharacter>());
    var DefHalfHeight = DefCharacter.GetCapsuleComponent().GetUnscaledCapsuleHalfHeight();
    var DefRadius = DefCharacter.GetCapsuleComponent().GetUnscaledCapsuleRadius();

    // Do not perform if collision is already at desired size.
    if (CharacterOwner.GetCapsuleComponent().GetUnscaledCapsuleHalfHeight() == DefHalfHeight)
    {
      return true;
    }

    var HeightAdjust = DefHalfHeight - CharacterOwner.GetCapsuleComponent().GetUnscaledCapsuleHalfHeight();
    var NewLocation = CharacterOwner.GetActorLocation() + new FVector(0.0, 0.0, HeightAdjust);

    // check if there is enough space for default capsule size
    var TraceParams = FCollisionQueryParams.createWithParams("FinishSlide", false, CharacterOwner);
    var ResponseParam = FCollisionResponseParams.create();
    InitCollisionParams(TraceParams, ResponseParam);
    // var bBlocked = GetWorld().OverlapBlockingTestByChannel(NewLocation, FQuat.Identity, UpdatedPrimitive.GetCollisionObjectType(), FCollisionShape.MakeCapsule(DefRadius, DefHalfHeight), TraceParams, ResponseParam);
    var bBlocked = GetWorld().OverlapBlockingTestByChannel(NewLocation, FQuat.Identity, UpdatedPrimitive.GetCollisionObjectType(), FCollisionShape.MakeCapsule(DefRadius, DefHalfHeight), TraceParams, FCollisionResponseParams.DefaultResponseParam);
    if (bBlocked)
    {
      return false;
    }

    // restore capsule size and move up to adjusted location
    CharacterOwner.TeleportTo(NewLocation, CharacterOwner.GetActorRotation(), false, true);
    CharacterOwner.GetCapsuleComponent().SetCapsuleSize(DefRadius, DefHalfHeight, true);

    // restoring original PawnOwner mesh relative location
    if (bWantsSlideMeshRelativeLocationOffset)
    {
      CharacterOwner.GetMesh().SetRelativeLocation(DefCharacter.GetMesh().RelativeLocation, false, null, None);
    }

    return true;
  }

  /** speed multiplier after hiting an obstacle */
  @:uproperty(EditDefaultsOnly, Category=Config)
  var ModSpeedObstacleHit:Float32;

  /** speed multiplier after ledge grab */
  @:uproperty(EditDefaultsOnly, Category=Config)
  var ModSpeedLedgeGrab:Float32;

  /** value by which speed will be reduced during slide */
  @:uproperty(EditDefaultsOnly, Category=Config)
  var SlideVelocityReduction:Float32;

  /** minimal speed pawn can slide with */
  @:uproperty(EditDefaultsOnly, Category=Config)
  var MinSlideSpeed:Float32;

  /** maximal speed pawn can slide with */
  @:uproperty(EditDefaultsOnly, Category=Config)
  var MaxSlideSpeed:Float32;

  /** height of pawn while sliding */
  @:uproperty(EditDefaultsOnly, Category=Config)
  var SlideHeight:Float32;

  /** offset value, by which relative location of pawn mesh needs to be changed, when pawn is sliding */
  var SlideMeshRelativeLocationOffset:FVector;

  /** value by which sliding pawn speed is currently being reduced */
  var CurrentSlideVelocityReduction:Float32;

  /** saved modified value of speed to restore after animation finish */
  var SavedSpeed:Float32;

  /** true when pawn is sliding */
  var bInSlide:Bool;

  /** true if pawn needs to use SlideMeshRelativeLocationOffset while sliding */
  var bWantsSlideMeshRelativeLocationOffset:Bool;
}
