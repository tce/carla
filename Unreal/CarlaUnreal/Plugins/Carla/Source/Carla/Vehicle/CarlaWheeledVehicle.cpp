// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
// Copyright (c) 2019 Intel Corporation
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Components/BoxComponent.h"
#include "Engine/CollisionProfile.h"
#include "MovementComponents/DefaultMovementComponent.h"
#include "Rendering/SkeletalMeshRenderData.h"
#include "UObject/UObjectGlobals.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetSystemLibrary.h"

#include "ChaosVehicleMovementComponent.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "VehicleAnimationInstance.h"

// #include "TireConfig.h"
// #include "VehicleWheel.h"

#include "Carla.h"
#include "Carla/Game/CarlaHUD.h"
#include "Carla/Game/CarlaStatics.h"
#include "Carla/Trigger/FrictionTrigger.h"
#include "Carla/Util/ActorAttacher.h"
#include "Carla/Util/EmptyActor.h"
#include "Carla/Util/BoundingBoxCalculator.h"
#include "Carla/Vegetation/VegetationManager.h"
#include "Carla/Vehicle/CarlaWheeledVehicle.h"

// =============================================================================
// -- Constructor and destructor -----------------------------------------------
// =============================================================================

ACarlaWheeledVehicle::ACarlaWheeledVehicle(const FObjectInitializer& ObjectInitializer) :
  Super(ObjectInitializer)
{
  VehicleBounds = CreateDefaultSubobject<UBoxComponent>(TEXT("VehicleBounds"));
  VehicleBounds->SetupAttachment(RootComponent);
  VehicleBounds->SetHiddenInGame(true);
  VehicleBounds->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);

  VelocityControl = CreateDefaultSubobject<UVehicleVelocityControl>(TEXT("VelocityControl"));
  VelocityControl->Deactivate();

  GetVehicleMovementComponent()->bReverseAsBrake = false; // @CARLA_UE5
  BaseMovementComponent = CreateDefaultSubobject<UBaseCarlaMovementComponent>(TEXT("BaseMovementComponent"));
}

ACarlaWheeledVehicle::~ACarlaWheeledVehicle() {}

void ACarlaWheeledVehicle::SetWheelCollision(
    UChaosWheeledVehicleMovementComponent* Vehicle4W,
    const FVehiclePhysicsControl &PhysicsControl ) {

  #ifdef WHEEL_SWEEP_ENABLED
    const bool IsBike = IsTwoWheeledVehicle();

    if (IsBike)
      return;

    const bool IsEqual = Vehicle4W->UseSweepWheelCollision == PhysicsControl.UseSweepWheelCollision;

    if (IsEqual)
      return;

    Vehicle4W->UseSweepWheelCollision = PhysicsControl.UseSweepWheelCollision;

  #else

    if (PhysicsControl.UseSweepWheelCollision)
      UE_LOG(LogCarla, Warning, TEXT("Error: Sweep for wheel collision is not available. \
      Make sure you have installed the required patch.") );

  #endif

}

void ACarlaWheeledVehicle::SetWheelCollisionNW(
    UChaosWheeledVehicleMovementComponent* VehicleNW,
    const FVehiclePhysicsControl &PhysicsControl ) {

  #ifdef WHEEL_SWEEP_ENABLED
    const bool IsEqual = VehicleNW->UseSweepWheelCollision == PhysicsControl.UseSweepWheelCollision;

    if (IsEqual)
      return;

    VehicleNW->UseSweepWheelCollision = PhysicsControl.UseSweepWheelCollision;

  #else

    if (PhysicsControl.UseSweepWheelCollision)
      UE_LOG(LogCarla, Warning, TEXT("Error: Sweep for wheel collision is not available. \
      Make sure you have installed the required patch.") );

  #endif
}

void ACarlaWheeledVehicle::BeginPlay()
{
  Super::BeginPlay();

  UDefaultMovementComponent::CreateDefaultMovementComponent(this);

  // Get constraint components and their initial transforms
  FTransform ActorInverseTransform = GetActorTransform().Inverse();
  ConstraintsComponents.Empty();
  DoorComponentsTransform.Empty();
  ConstraintDoor.Empty();
  for (FName& ComponentName : ConstraintComponentNames)
  {
    UPhysicsConstraintComponent* ConstraintComponent =
        Cast<UPhysicsConstraintComponent>(GetDefaultSubobjectByName(ComponentName));
    if (ConstraintComponent)
    {
      UPrimitiveComponent* DoorComponent = Cast<UPrimitiveComponent>(
          GetDefaultSubobjectByName(ConstraintComponent->ComponentName1.ComponentName));
      if(DoorComponent)
      {
        UE_LOG(LogCarla, Warning, TEXT("Door name: %s"), *(DoorComponent->GetName()));
        FTransform ComponentWorldTransform = DoorComponent->GetComponentTransform();
        FTransform RelativeTransform = ComponentWorldTransform * ActorInverseTransform;
        DoorComponentsTransform.Add(DoorComponent, RelativeTransform);
        ConstraintDoor.Add(ConstraintComponent, DoorComponent);
        ConstraintsComponents.Add(ConstraintComponent);
        ConstraintComponent->TermComponentConstraint();
      }
      else
      {
        UE_LOG(LogCarla, Error, TEXT("Missing component for constraint: %s"), *(ConstraintComponent->GetName()));
      }
    }
  }
  ResetConstraints();

  // get collision disable constraints (used to prevent doors from colliding with each other)
  CollisionDisableConstraints.Empty();
  TArray<UPhysicsConstraintComponent*> Constraints;
  GetComponents(Constraints);
  for (UPhysicsConstraintComponent* Constraint : Constraints)
  {
    if (!ConstraintsComponents.Contains(Constraint))
    {
      UPrimitiveComponent* CollisionDisabledComponent1 = Cast<UPrimitiveComponent>(
          GetDefaultSubobjectByName(Constraint->ComponentName1.ComponentName));
      UPrimitiveComponent* CollisionDisabledComponent2 = Cast<UPrimitiveComponent>(
          GetDefaultSubobjectByName(Constraint->ComponentName2.ComponentName));
      if (CollisionDisabledComponent1)
      {
        CollisionDisableConstraints.Add(CollisionDisabledComponent1, Constraint);
      }
      if (CollisionDisabledComponent2)
      {
        CollisionDisableConstraints.Add(CollisionDisabledComponent2, Constraint);
      }
    }
  }

  float FrictionScale = 3.5f;

#if 1 // @CARLA_UE5
  auto MovementComponent = Cast<UChaosWheeledVehicleMovementComponent>(GetVehicleMovementComponent());

  if (MovementComponent)
  {
    check(MovementComponent != nullptr);

    // Setup Tire Configs with default value. This is needed to avoid getting
    // friction values of previously created TireConfigs for the same vehicle
    // blueprint.
    TArray<float> OriginalFrictions;
    OriginalFrictions.Init(FrictionScale, MovementComponent->Wheels.Num());
    SetWheelsFrictionScale(OriginalFrictions);

    // Check if it overlaps with a Friction trigger, if so, update the friction
    // scale.
    TArray<AActor *> OverlapActors;
    GetOverlappingActors(OverlapActors, AFrictionTrigger::StaticClass());
    for (const auto &Actor : OverlapActors)
    {
      AFrictionTrigger *FrictionTrigger = Cast<AFrictionTrigger>(Actor);
      if (FrictionTrigger)
      {
        FrictionScale = FrictionTrigger->Friction;
      }
    }

    // Set the friction scale to Wheel CDO and update wheel setups
    TArray<FChaosWheelSetup> NewWheelSetups = MovementComponent->WheelSetups;
    for (const auto &WheelSetup : NewWheelSetups)
    {
      UChaosVehicleWheel *Wheel = WheelSetup.WheelClass.GetDefaultObject();
      check(Wheel != nullptr);
    }

    MovementComponent->WheelSetups = NewWheelSetups;

    LastPhysicsControl = GetVehiclePhysicsControl();

    // Update physics in the Ackermann Controller
    AckermannController.UpdateVehiclePhysics(this);
  }
#endif
  AddReferenceToManager();
}

bool ACarlaWheeledVehicle::IsInVehicleRange(const FVector& Location) const
{
  TRACE_CPUPROFILER_EVENT_SCOPE(ACarlaWheeledVehicle::IsInVehicleRange);

  return FoliageBoundingBox.IsInside(Location);
}

void ACarlaWheeledVehicle::UpdateDetectionBox()
{
  const FTransform GlobalTransform = GetActorTransform();
  const FVector Vec { DetectionSize, DetectionSize, DetectionSize };
  FBox Box = FBox(-Vec, Vec);
  const FTransform NonScaledTransform(GlobalTransform.GetRotation(), GlobalTransform.GetLocation(), {1.0f, 1.0f, 1.0f});
  FoliageBoundingBox = Box.TransformBy(NonScaledTransform);
}

const TArray<int32> ACarlaWheeledVehicle::GetFoliageInstancesCloseToVehicle(const UInstancedStaticMeshComponent* Component) const
{  
  TRACE_CPUPROFILER_EVENT_SCOPE(ACarlaWheeledVehicle::GetFoliageInstancesCloseToVehicle);
  return Component->GetInstancesOverlappingBox(FoliageBoundingBox);
}

FBox ACarlaWheeledVehicle::GetDetectionBox() const
{  
  TRACE_CPUPROFILER_EVENT_SCOPE(ACarlaWheeledVehicle::GetDetectionBox);
  return FoliageBoundingBox;
}

float ACarlaWheeledVehicle::GetDetectionSize() const
{
  return DetectionSize;
}

void ACarlaWheeledVehicle::DrawFoliageBoundingBox() const
{
  const FVector& Center = FoliageBoundingBox.GetCenter();
  const FVector& Extent = FoliageBoundingBox.GetExtent();
  const FQuat& Rotation = GetActorQuat();
  DrawDebugBox(GetWorld(), Center, Extent, Rotation, FColor::Magenta, false, 0.0f, 0, 5.0f);
}

FBoxSphereBounds ACarlaWheeledVehicle::GetBoxSphereBounds() const
{
  ALargeMapManager* LargeMap = UCarlaStatics::GetLargeMapManager(GetWorld());
  if (LargeMap)
  {
    FTransform GlobalTransform = LargeMap->LocalToGlobalTransform(GetActorTransform());
    return VehicleBounds->CalcBounds(GlobalTransform);
  }
  return VehicleBounds->CalcBounds(GetActorTransform());
}

void ACarlaWheeledVehicle::AdjustVehicleBounds()
{
  FBoundingBox BoundingBox = UBoundingBoxCalculator::GetVehicleBoundingBox(this);

  const FTransform& CompToWorldTransform = RootComponent->GetComponentTransform();
  const FRotator Rotation = CompToWorldTransform.GetRotation().Rotator();
  const FVector Translation = CompToWorldTransform.GetLocation();
  const FVector Scale = CompToWorldTransform.GetScale3D();

  // Invert BB origin to local space
  BoundingBox.Origin -= Translation;
  BoundingBox.Origin = Rotation.UnrotateVector(BoundingBox.Origin);
  BoundingBox.Origin /= Scale;

  // Prepare Box Collisions
  FTransform Transform;
  Transform.SetTranslation(BoundingBox.Origin);
  VehicleBounds->SetRelativeTransform(Transform);
  VehicleBounds->SetBoxExtent(BoundingBox.Extent);
}

// =============================================================================
// -- Get functions ------------------------------------------------------------
// =============================================================================

float ACarlaWheeledVehicle::GetVehicleForwardSpeed() const
{
  return BaseMovementComponent->GetVehicleForwardSpeed();
}

FVector ACarlaWheeledVehicle::GetVehicleOrientation() const
{
  return GetVehicleTransform().GetRotation().GetForwardVector();
}

int32 ACarlaWheeledVehicle::GetVehicleCurrentGear() const
{
    return BaseMovementComponent->GetVehicleCurrentGear();
}

FTransform ACarlaWheeledVehicle::GetVehicleBoundingBoxTransform() const
{
  return VehicleBounds->GetRelativeTransform();
}

FVector ACarlaWheeledVehicle::GetVehicleBoundingBoxExtent() const
{
  return VehicleBounds->GetScaledBoxExtent();
}

float ACarlaWheeledVehicle::GetMaximumSteerAngle() const
{
#if 1 // @CARLA_UE5
  const auto &Wheels = Cast<UChaosWheeledVehicleMovementComponent>(GetVehicleMovementComponent())->Wheels;
  check(Wheels.Num() > 0);
  auto FrontWheel = Wheels[0];
  check(FrontWheel != nullptr);
  return FrontWheel->GetSteerAngle();
#else
    return 0.0f;
#endif
}

// =============================================================================
// -- Set functions ------------------------------------------------------------
// =============================================================================

void ACarlaWheeledVehicle::FlushVehicleControl()
{
  if (bAckermannControlActive) {
    AckermannController.UpdateVehicleState(this);
    AckermannController.RunLoop(InputControl.Control);
  }

  BaseMovementComponent->ProcessControl(InputControl.Control);
  InputControl.Control.bReverse = InputControl.Control.Gear < 0;
  LastAppliedControl = InputControl.Control;
  InputControl.Priority = EVehicleInputPriority::INVALID;
}

void ACarlaWheeledVehicle::SetThrottleInput(const float Value)
{
  FVehicleControl Control = InputControl.Control;
  Control.Throttle = Value;
  ApplyVehicleControl(Control, EVehicleInputPriority::User);
}

void ACarlaWheeledVehicle::SetSteeringInput(const float Value)
{
  FVehicleControl Control = InputControl.Control;
  Control.Steer = Value;
  ApplyVehicleControl(Control, EVehicleInputPriority::User);
}

void ACarlaWheeledVehicle::SetBrakeInput(const float Value)
{
  FVehicleControl Control = InputControl.Control;
  Control.Brake = Value;
  ApplyVehicleControl(Control, EVehicleInputPriority::User);
}

void ACarlaWheeledVehicle::SetReverse(const bool Value)
{
  FVehicleControl Control = InputControl.Control;
  Control.bReverse = Value;
  ApplyVehicleControl(Control, EVehicleInputPriority::User);
}

void ACarlaWheeledVehicle::SetHandbrakeInput(const bool Value)
{
  FVehicleControl Control = InputControl.Control;
  Control.bHandBrake = Value;
  ApplyVehicleControl(Control, EVehicleInputPriority::User);
}

TArray<float> ACarlaWheeledVehicle::GetWheelsFrictionScale()
{
    TArray<float> WheelsFrictionScale;

    auto Movement = Cast<UChaosWheeledVehicleMovementComponent>(GetVehicleMovement());

    if (Movement)
    {
        for (auto& Wheel : Movement->Wheels)
        {
            auto Friction = Wheel->FrictionForceMultiplier;
            // auto Friction = Wheel->TireConfig->GetFrictionScale()
            WheelsFrictionScale.Add(Friction);
        }
    }

    return WheelsFrictionScale;
}

void ACarlaWheeledVehicle::SetWheelsFrictionScale(TArray<float>& WheelsFrictionScale)
{
    auto Movement = Cast<UChaosWheeledVehicleMovementComponent>(GetVehicleMovement());

    if (Movement)
    {
        check(Movement->Wheels.Num() == WheelsFrictionScale.Num());

        for (int32 i = 0; i < Movement->Wheels.Num(); ++i)
        {
            Movement->Wheels[i]->FrictionForceMultiplier = WheelsFrictionScale[i];
        }
    }
}

FVehiclePhysicsControl ACarlaWheeledVehicle::GetVehiclePhysicsControl() const
{
    FVehiclePhysicsControl PhysicsControl;

    if (!bIsNWVehicle)
    {
        auto Vehicle4W = Cast<UChaosWheeledVehicleMovementComponent>(GetVehicleMovement());
        check(Vehicle4W != nullptr);

        // Engine Setup
        PhysicsControl.TorqueCurve = Vehicle4W->EngineSetup.TorqueCurve.EditorCurveData;
        PhysicsControl.MaxRPM = Vehicle4W->EngineSetup.MaxRPM;
        PhysicsControl.MOI = Vehicle4W->EngineSetup.EngineRevUpMOI;
        // @TODO PhysicsControl.DampingRateFullThrottle = Vehicle4W->EngineSetup.DampingRateFullThrottle;
        // @TODO PhysicsControl.DampingRateZeroThrottleClutchEngaged = Vehicle4W->EngineSetup.DampingRateZeroThrottleClutchEngaged;
        // @TODO PhysicsControl.DampingRateZeroThrottleClutchDisengaged = Vehicle4W->EngineSetup.DampingRateZeroThrottleClutchDisengaged;

        // Transmission Setup
        PhysicsControl.bUseGearAutoBox = Vehicle4W->TransmissionSetup.bUseAutomaticGears;
        PhysicsControl.GearSwitchTime = Vehicle4W->TransmissionSetup.GearChangeTime;
        // @TODO PhysicsControl.ClutchStrength = Vehicle4W->TransmissionSetup.ClutchStrength;
        PhysicsControl.FinalRatio = Vehicle4W->TransmissionSetup.FinalRatio;

        TArray<FGearPhysicsControl> ForwardGears;

        for (int32 i = 0; i != Vehicle4W->TransmissionSetup.ForwardGearRatios.Num(); ++i)
        {
            FGearPhysicsControl GearPhysicsControl;

            GearPhysicsControl.Ratio = Vehicle4W->TransmissionSetup.GetGearRatio(i);
            GearPhysicsControl.UpRatio = Vehicle4W->TransmissionSetup.ForwardGearRatios[i];
            GearPhysicsControl.DownRatio = Vehicle4W->TransmissionSetup.ReverseGearRatios[i];

            ForwardGears.Add(GearPhysicsControl);
        }

        PhysicsControl.ForwardGears = ForwardGears;

        // Vehicle Setup
        PhysicsControl.Mass = Vehicle4W->Mass;
        PhysicsControl.DragCoefficient = Vehicle4W->DragCoefficient;

        // Center of mass offset (Center of mass is always zero vector in local
        // position)
        UPrimitiveComponent* UpdatedPrimitive = Cast<UPrimitiveComponent>(Vehicle4W->UpdatedComponent);
        check(UpdatedPrimitive != nullptr);

        PhysicsControl.CenterOfMass = UpdatedPrimitive->BodyInstance.COMNudge;

        // Transmission Setup
        PhysicsControl.SteeringCurve = Vehicle4W->SteeringSetup.SteeringCurve.EditorCurveData;

        // Wheels Setup
        TArray<FWheelPhysicsControl> Wheels;

        for (int32 i = 0; i < Vehicle4W->WheelSetups.Num(); ++i)
        {
            FWheelPhysicsControl PhysicsWheel;
            // @TODO PxVehicleWheelData PWheelData = Vehicle4W->PVehicle->mWheelsSimData.getWheelData(i);
            // @TODO PhysicsWheel.DampingRate = Cm2ToM2(PWheelData.mDampingRate);
            // @TODO PhysicsWheel.MaxSteerAngle = FMath::RadiansToDegrees(PWheelData.mMaxSteer);
            // @TODO PhysicsWheel.Radius = PWheelData.mRadius;
            // @TODO PhysicsWheel.MaxBrakeTorque = Cm2ToM2(PWheelData.mMaxBrakeTorque);
            // @TODO PhysicsWheel.MaxHandBrakeTorque = Cm2ToM2(PWheelData.mMaxHandBrakeTorque);
            // @TODO PxVehicleTireData PTireData = Vehicle4W->PVehicle->mWheelsSimData.getTireData(i);
            // @TODO PhysicsWheel.LatStiffMaxLoad = PTireData.mLatStiffX;
            // @TODO PhysicsWheel.LatStiffValue = PTireData.mLatStiffY;
            // @TODO PhysicsWheel.LongStiffValue = PTireData.mLongitudinalStiffnessPerUnitGravity;
            // @TODO PhysicsWheel.TireFriction = Vehicle4W->Wheels[i]->TireConfig->GetFrictionScale();
            // @TODO PhysicsWheel.Position = Vehicle4W->Wheels[i]->Location;
            Wheels.Add(PhysicsWheel);
        }

        PhysicsControl.Wheels = Wheels;

    }
    else {
        auto VehicleNW = Cast<UChaosWheeledVehicleMovementComponent>(
            GetVehicleMovement());

        check(VehicleNW != nullptr);

        // Engine Setup
        PhysicsControl.TorqueCurve = VehicleNW->EngineSetup.TorqueCurve.EditorCurveData;
        PhysicsControl.MaxRPM = VehicleNW->EngineSetup.MaxRPM;
        PhysicsControl.MOI = VehicleNW->EngineSetup.EngineRevUpMOI;
        // @TODO PhysicsControl.DampingRateFullThrottle = VehicleNW->EngineSetup.DampingRateFullThrottle;
        // @TODO PhysicsControl.DampingRateZeroThrottleClutchEngaged = VehicleNW->EngineSetup.DampingRateZeroThrottleClutchEngaged;
        // @TODO PhysicsControl.DampingRateZeroThrottleClutchDisengaged = VehicleNW->EngineSetup.DampingRateZeroThrottleClutchDisengaged;

        // Transmission Setup
        PhysicsControl.bUseGearAutoBox = VehicleNW->TransmissionSetup.bUseAutomaticGears;
        PhysicsControl.GearSwitchTime = VehicleNW->TransmissionSetup.GearChangeTime;
        // @TODO PhysicsControl.ClutchStrength = VehicleNW->TransmissionSetup.ClutchStrength;
        PhysicsControl.FinalRatio = VehicleNW->TransmissionSetup.FinalRatio;

        TArray<FGearPhysicsControl> ForwardGears;

        for (int32 i = 0; i != VehicleNW->TransmissionSetup.ForwardGearRatios.Num(); ++i)
        {
            FGearPhysicsControl GearPhysicsControl;
            GearPhysicsControl.Ratio = VehicleNW->TransmissionSetup.GetGearRatio(i);
            GearPhysicsControl.UpRatio = VehicleNW->TransmissionSetup.ForwardGearRatios[i];
            GearPhysicsControl.DownRatio = VehicleNW->TransmissionSetup.ReverseGearRatios[i];
            ForwardGears.Add(GearPhysicsControl);
        }

        PhysicsControl.ForwardGears = ForwardGears;

        // VehicleNW Setup
        PhysicsControl.Mass = VehicleNW->Mass;
        PhysicsControl.DragCoefficient = VehicleNW->DragCoefficient;

        // Center of mass offset (Center of mass is always zero vector in local
        // position)
        UPrimitiveComponent* UpdatedPrimitive = Cast<UPrimitiveComponent>(VehicleNW->UpdatedComponent);
        check(UpdatedPrimitive != nullptr);

        PhysicsControl.CenterOfMass = UpdatedPrimitive->BodyInstance.COMNudge;

        // Transmission Setup
        PhysicsControl.SteeringCurve = VehicleNW->SteeringSetup.SteeringCurve.EditorCurveData;

        // Wheels Setup
        TArray<FWheelPhysicsControl> Wheels;

        for (int32 i = 0; i < VehicleNW->WheelSetups.Num(); ++i)
        {
            FWheelPhysicsControl PhysicsWheel;
            // @TODO PxVehicleWheelData PWheelData = VehicleNW->PVehicle->mWheelsSimData.getWheelData(i);
            // @TODO PhysicsWheel.DampingRate = Cm2ToM2(PWheelData.mDampingRate);
            // @TODO PhysicsWheel.MaxSteerAngle = FMath::RadiansToDegrees(PWheelData.mMaxSteer);
            // @TODO PhysicsWheel.Radius = PWheelData.mRadius;
            // @TODO PhysicsWheel.MaxBrakeTorque = Cm2ToM2(PWheelData.mMaxBrakeTorque);
            // @TODO PhysicsWheel.MaxHandBrakeTorque = Cm2ToM2(PWheelData.mMaxHandBrakeTorque);
            // @TODO PxVehicleTireData PTireData = VehicleNW->PVehicle->mWheelsSimData.getTireData(i);
            // @TODO PhysicsWheel.LatStiffMaxLoad = PTireData.mLatStiffX;
            // @TODO PhysicsWheel.LatStiffValue = PTireData.mLatStiffY;
            // @TODO PhysicsWheel.LongStiffValue = PTireData.mLongitudinalStiffnessPerUnitGravity;
            // @TODO PhysicsWheel.TireFriction = VehicleNW->Wheels[i]->TireConfig->GetFrictionScale();
            // @TODO PhysicsWheel.Position = VehicleNW->Wheels[i]->Location;
            Wheels.Add(PhysicsWheel);
        }

        PhysicsControl.Wheels = Wheels;

    }

    return PhysicsControl;
}

FVehicleLightState ACarlaWheeledVehicle::GetVehicleLightState() const
{
    return InputControl.LightState;
}

void ACarlaWheeledVehicle::RestoreVehiclePhysicsControl()
{
    ApplyVehiclePhysicsControl(LastPhysicsControl);
}

void ACarlaWheeledVehicle::ApplyVehiclePhysicsControl(const FVehiclePhysicsControl& PhysicsControl)
{
    LastPhysicsControl = PhysicsControl;

    if (!bIsNWVehicle)
    {
        auto Vehicle4W = Cast<UChaosWheeledVehicleMovementComponent>(GetVehicleMovement());
        check(Vehicle4W != nullptr);

        // Engine Setup
        Vehicle4W->EngineSetup.TorqueCurve.EditorCurveData = PhysicsControl.TorqueCurve;
        Vehicle4W->EngineSetup.MaxRPM = PhysicsControl.MaxRPM;
        Vehicle4W->EngineSetup.EngineRevUpMOI = PhysicsControl.MOI;

        // @TODO Vehicle4W->EngineSetup.DampingRateFullThrottle = PhysicsControl.DampingRateFullThrottle;
        // @TODO Vehicle4W->EngineSetup.DampingRateZeroThrottleClutchEngaged = PhysicsControl.DampingRateZeroThrottleClutchEngaged;
        // @TODO Vehicle4W->EngineSetup.DampingRateZeroThrottleClutchDisengaged = PhysicsControl.DampingRateZeroThrottleClutchDisengaged;

        // Transmission Setup
        Vehicle4W->TransmissionSetup.bUseAutomaticGears = PhysicsControl.bUseGearAutoBox;
        Vehicle4W->TransmissionSetup.GearChangeTime = PhysicsControl.GearSwitchTime;
        // @TODO Vehicle4W->TransmissionSetup.ClutchStrength = PhysicsControl.ClutchStrength;
        Vehicle4W->TransmissionSetup.FinalRatio = PhysicsControl.FinalRatio;

        Vehicle4W->TransmissionSetup.ForwardGearRatios.SetNum(std::max(
            Vehicle4W->TransmissionSetup.ForwardGearRatios.Num(),
            PhysicsControl.ForwardGears.Num()));

        Vehicle4W->TransmissionSetup.ReverseGearRatios.SetNum(std::max(
            Vehicle4W->TransmissionSetup.ReverseGearRatios.Num(),
            PhysicsControl.ForwardGears.Num()));

        for (int32 i = 0; i != PhysicsControl.ForwardGears.Num(); ++i)
        {
            auto& Gear = PhysicsControl.ForwardGears[i];
            Vehicle4W->TransmissionSetup.ForwardGearRatios[i] = Gear.UpRatio;
            Vehicle4W->TransmissionSetup.ReverseGearRatios[i] = Gear.DownRatio;
        }

        // Vehicle Setup
        Vehicle4W->Mass = PhysicsControl.Mass;
        Vehicle4W->DragCoefficient = PhysicsControl.DragCoefficient;

        // Center of mass
        UPrimitiveComponent* UpdatedPrimitive = Cast<UPrimitiveComponent>(Vehicle4W->UpdatedComponent);
        check(UpdatedPrimitive != nullptr);

        UpdatedPrimitive->BodyInstance.COMNudge = PhysicsControl.CenterOfMass;

        // Transmission Setup
        Vehicle4W->SteeringSetup.SteeringCurve.EditorCurveData = PhysicsControl.SteeringCurve;

        // Wheels Setup
        const int PhysicsWheelsNum = PhysicsControl.Wheels.Num();
        if (PhysicsWheelsNum != 4)
        {
            UE_LOG(LogCarla, Error, TEXT("Number of WheelPhysicsControl is not 4."));
            return;
        }

        // Change, if required, the collision mode for wheels
        SetWheelCollision(Vehicle4W, PhysicsControl);

        TArray<FChaosWheelSetup> NewWheelSetups = Vehicle4W->WheelSetups;

        for (int32 i = 0; i < PhysicsWheelsNum; ++i)
        {
            UChaosVehicleWheel* Wheel = NewWheelSetups[i].WheelClass.GetDefaultObject();
            check(Wheel != nullptr);

            // @TODO Wheel->TireConfig = DuplicateObject<UTireConfig>(Wheel->TireConfig, nullptr);
            // @TODO Wheel->TireConfig->SetFrictionScale(PhysicsControl.Wheels[i].TireFriction);
        }

        Vehicle4W->WheelSetups = NewWheelSetups;

        // Recreate Physics State for vehicle setup
        // GetWorld()->GetPhysicsScene()->GetPxScene()->lockWrite();
        Vehicle4W->RecreatePhysicsState();
        // GetWorld()->GetPhysicsScene()->GetPxScene()->unlockWrite();

        for (int32 i = 0; i < PhysicsWheelsNum; ++i)
        {
            // @TODO PxVehicleWheelData PWheelData = Vehicle4W->PVehicle->mWheelsSimData.getWheelData(i);
            // @TODO PWheelData.mRadius = PhysicsControl.Wheels[i].Radius;
            // @TODO PWheelData.mMaxSteer = FMath::DegreesToRadians(PhysicsControl.Wheels[i].MaxSteerAngle);
            // @TODO PWheelData.mDampingRate = M2ToCm2(PhysicsControl.Wheels[i].DampingRate);
            // @TODO PWheelData.mMaxBrakeTorque = M2ToCm2(PhysicsControl.Wheels[i].MaxBrakeTorque);
            // @TODO PWheelData.mMaxHandBrakeTorque = M2ToCm2(PhysicsControl.Wheels[i].MaxHandBrakeTorque);
            // @TODO Vehicle4W->PVehicle->mWheelsSimData.setWheelData(i, PWheelData);
            // @TODO PxVehicleTireData PTireData = Vehicle4W->PVehicle->mWheelsSimData.getTireData(i);
            // @TODO PTireData.mLatStiffX = PhysicsControl.Wheels[i].LatStiffMaxLoad;
            // @TODO PTireData.mLatStiffY = PhysicsControl.Wheels[i].LatStiffValue;
            // @TODO PTireData.mLongitudinalStiffnessPerUnitGravity = PhysicsControl.Wheels[i].LongStiffValue;
            // @TODO Vehicle4W->PVehicle->mWheelsSimData.setTireData(i, PTireData);
        }

        ResetConstraints();
    }
    else {
        auto VehicleNW = Cast<UChaosWheeledVehicleMovementComponent>(
            GetVehicleMovement());

        check(VehicleNW != nullptr);

        // Engine Setup
        VehicleNW->EngineSetup.TorqueCurve.EditorCurveData = PhysicsControl.TorqueCurve;
        VehicleNW->EngineSetup.MaxRPM = PhysicsControl.MaxRPM;

        VehicleNW->EngineSetup.EngineRevUpMOI = PhysicsControl.MOI;

        // @TODO VehicleNW->EngineSetup.DampingRateFullThrottle = PhysicsControl.DampingRateFullThrottle;
        // @TODO VehicleNW->EngineSetup.DampingRateZeroThrottleClutchEngaged = PhysicsControl.DampingRateZeroThrottleClutchEngaged;
        // @TODO VehicleNW->EngineSetup.DampingRateZeroThrottleClutchDisengaged = PhysicsControl.DampingRateZeroThrottleClutchDisengaged;

        // Transmission Setup
        VehicleNW->TransmissionSetup.bUseAutomaticGears = PhysicsControl.bUseGearAutoBox;
        VehicleNW->TransmissionSetup.GearChangeTime = PhysicsControl.GearSwitchTime;
        // @TODO VehicleNW->TransmissionSetup.ClutchStrength = PhysicsControl.ClutchStrength;
        VehicleNW->TransmissionSetup.FinalRatio = PhysicsControl.FinalRatio;

        for (int32 i = 0; i != PhysicsControl.ForwardGears.Num(); ++i)
        {
            auto& Gear = PhysicsControl.ForwardGears[i];
            VehicleNW->TransmissionSetup.ForwardGearRatios[i] = Gear.UpRatio;
            VehicleNW->TransmissionSetup.ReverseGearRatios[i] = Gear.DownRatio;
        }

        // VehicleNW Setup
        VehicleNW->Mass = PhysicsControl.Mass;
        VehicleNW->DragCoefficient = PhysicsControl.DragCoefficient;

        // Center of mass
        UPrimitiveComponent* UpdatedPrimitive = Cast<UPrimitiveComponent>(VehicleNW->UpdatedComponent);
        check(UpdatedPrimitive != nullptr);

        UpdatedPrimitive->BodyInstance.COMNudge = PhysicsControl.CenterOfMass;

        // Transmission Setup
        VehicleNW->SteeringSetup.SteeringCurve.EditorCurveData = PhysicsControl.SteeringCurve;

        // Wheels Setup
        const int PhysicsWheelsNum = PhysicsControl.Wheels.Num();

        // Change, if required, the collision mode for wheels
        SetWheelCollisionNW(VehicleNW, PhysicsControl);

        TArray<FChaosWheelSetup> NewWheelSetups = VehicleNW->WheelSetups;

        for (int32 i = 0; i < PhysicsWheelsNum; ++i)
        {
            UChaosVehicleWheel* Wheel = NewWheelSetups[i].WheelClass.GetDefaultObject();
            check(Wheel != nullptr);

            // @TODO Wheel->TireConfig = DuplicateObject<UTireConfig>(Wheel->TireConfig, nullptr);
            // @TODO Wheel->TireConfig->SetFrictionScale(PhysicsControl.Wheels[i].TireFriction);
        }

        VehicleNW->WheelSetups = NewWheelSetups;

        // Recreate Physics State for vehicle setup
        // GetWorld()->GetPhysicsScene()->GetPxScene()->lockWrite();
        VehicleNW->RecreatePhysicsState();
        // GetWorld()->GetPhysicsScene()->GetPxScene()->unlockWrite();

        for (int32 i = 0; i < PhysicsWheelsNum; ++i)
        {
            // @TODO PxVehicleWheelData PWheelData = VehicleNW->PVehicle->mWheelsSimData.getWheelData(i);
            // @TODO PWheelData.mRadius = PhysicsControl.Wheels[i].Radius;
            // @TODO PWheelData.mMaxSteer = FMath::DegreesToRadians(PhysicsControl.Wheels[i].MaxSteerAngle);
            // @TODO PWheelData.mDampingRate = M2ToCm2(PhysicsControl.Wheels[i].DampingRate);
            // @TODO PWheelData.mMaxBrakeTorque = M2ToCm2(PhysicsControl.Wheels[i].MaxBrakeTorque);
            // @TODO PWheelData.mMaxHandBrakeTorque = M2ToCm2(PhysicsControl.Wheels[i].MaxHandBrakeTorque);
            // @TODO VehicleNW->PVehicle->mWheelsSimData.setWheelData(i, PWheelData);
            // @TODO PxVehicleTireData PTireData = VehicleNW->PVehicle->mWheelsSimData.getTireData(i);
            // @TODO PTireData.mLatStiffX = PhysicsControl.Wheels[i].LatStiffMaxLoad;
            // @TODO PTireData.mLatStiffY = PhysicsControl.Wheels[i].LatStiffValue;
            // @TODO PTireData.mLongitudinalStiffnessPerUnitGravity = PhysicsControl.Wheels[i].LongStiffValue;
            // @TODO VehicleNW->PVehicle->mWheelsSimData.setTireData(i, PTireData);
        }

        ResetConstraints();

    }

    auto* Recorder = UCarlaStatics::GetRecorder(GetWorld());
    if (Recorder && Recorder->IsEnabled())
    {
        Recorder->AddPhysicsControl(*this);
    }

    // Update physics in the Ackermann Controller
    AckermannController.UpdateVehiclePhysics(this);
}

void ACarlaWheeledVehicle::ActivateVelocityControl(const FVector& Velocity)
{
    VelocityControl->Activate(Velocity);
}

void ACarlaWheeledVehicle::DeactivateVelocityControl()
{
    VelocityControl->Deactivate();
}

void ACarlaWheeledVehicle::ShowDebugTelemetry(bool Enabled)
{
    if (GetWorld()->GetFirstPlayerController())
    {
        ACarlaHUD* hud = Cast<ACarlaHUD>(GetWorld()->GetFirstPlayerController()->GetHUD());
        if (hud) {

            // Set/Unset the car movement component in HUD to show the temetry
            auto MovementComponent = Cast<UChaosWheeledVehicleMovementComponent>(GetVehicleMovementComponent());

            if (Enabled)
            {
                hud->AddDebugVehicleForTelemetry(MovementComponent);
            }
            else
            {
                if (hud->DebugVehicle == MovementComponent)
                {
                    hud->AddDebugVehicleForTelemetry(nullptr);
                    // MovementComponent->StopTelemetry();
                }
            }

        }
        else {
            UE_LOG(LogCarla, Warning, TEXT("ACarlaWheeledVehicle::ShowDebugTelemetry:: Cannot find HUD for debug info"));
        }
    }
}

void ACarlaWheeledVehicle::SetVehicleLightState(const FVehicleLightState& LightState)
{
    InputControl.LightState = LightState;
    RefreshLightState(LightState);
}

void ACarlaWheeledVehicle::SetFailureState(const carla::rpc::VehicleFailureState& InFailureState)
{
    FailureState = InFailureState;
}

void ACarlaWheeledVehicle::SetCarlaMovementComponent(UBaseCarlaMovementComponent* MovementComponent)
{
    if (BaseMovementComponent)
    {
        BaseMovementComponent->DestroyComponent();
    }
    BaseMovementComponent = MovementComponent;
}

void ACarlaWheeledVehicle::SetWheelSteerDirection(EVehicleWheelLocation WheelLocation, float AngleInDeg) {

    if (bPhysicsEnabled == false)
    {
        check((uint8)WheelLocation >= 0);
        UVehicleAnimationInstance* VehicleAnim = Cast<UVehicleAnimationInstance>(GetMesh()->GetAnimInstance());
        check(VehicleAnim != nullptr);
        // @TODO VehicleAnim->SetWheelRotYaw((uint8)WheelLocation, AngleInDeg);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot set wheel steer direction. Physics are enabled."))
    }
}

float ACarlaWheeledVehicle::GetWheelSteerAngle(EVehicleWheelLocation WheelLocation)
{
    check((uint8)WheelLocation >= 0);
    UVehicleAnimationInstance* VehicleAnim = Cast<UVehicleAnimationInstance>(GetMesh()->GetAnimInstance());
    check(VehicleAnim != nullptr);
    check(VehicleAnim->GetWheeledVehicleComponent() != nullptr);

    if (bPhysicsEnabled == true)
    {
        return VehicleAnim->GetWheeledVehicleComponent()->Wheels[(uint8)WheelLocation]->GetSteerAngle();
    }
    else
    {
        // @TODO return VehicleAnim->GetWheelRotAngle((uint8)WheelLocation);
        return 0.0F;
    }
}

void ACarlaWheeledVehicle::SetSimulatePhysics(bool enabled)
{
    if (!GetCarlaMovementComponent<UDefaultMovementComponent>())
    {
        return;
    }

    auto Movement = GetVehicleMovement();
    if (Movement)
    {
        check(Movement != nullptr);

        if (bPhysicsEnabled == enabled)
            return;

        SetActorEnableCollision(true);
        auto RootComponent = Cast<UPrimitiveComponent>(GetRootComponent());
        RootComponent->SetSimulatePhysics(enabled);
        RootComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

        auto VehicleAnim = Cast<UVehicleAnimationInstance>(GetMesh()->GetAnimInstance());
        check(VehicleAnim != nullptr);

        // GetWorld()->GetPhysicsScene()->GetPxScene()->lockWrite();
        if (enabled)
        {
            Movement->RecreatePhysicsState();
            // @TODO VehicleAnim->ResetWheelCustomRotations();
        }
        else
        {
            Movement->DestroyPhysicsState();
        }

        // GetWorld()->GetPhysicsScene()->GetPxScene()->unlockWrite();

        bPhysicsEnabled = enabled;

        ResetConstraints();
    }
}

void ACarlaWheeledVehicle::ResetConstraints()
{
    for (int i = 0; i < ConstraintsComponents.Num(); i++)
    {
        OpenDoorPhys(EVehicleDoor(i));
    }
    for (int i = 0; i < ConstraintsComponents.Num(); i++)
    {
        CloseDoorPhys(EVehicleDoor(i));
    }
}

FVector ACarlaWheeledVehicle::GetVelocity() const
{
    return BaseMovementComponent->GetVelocity();
}

void ACarlaWheeledVehicle::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    ShowDebugTelemetry(false);
    Super::EndPlay(EndPlayReason);
    RemoveReferenceToManager();
}

void ACarlaWheeledVehicle::OpenDoor(const EVehicleDoor DoorIdx) {
    if (int(DoorIdx) >= ConstraintsComponents.Num() && DoorIdx != EVehicleDoor::All) {
        UE_LOG(LogTemp, Warning, TEXT("This door is not configured for this car."));
        return;
    }

    if (DoorIdx == EVehicleDoor::All) {
        for (int i = 0; i < ConstraintsComponents.Num(); i++)
        {
            OpenDoorPhys(EVehicleDoor(i));
        }
        return;
    }

    OpenDoorPhys(DoorIdx);
}

void ACarlaWheeledVehicle::CloseDoor(const EVehicleDoor DoorIdx) {
    if (int(DoorIdx) >= ConstraintsComponents.Num() && DoorIdx != EVehicleDoor::All) {
        UE_LOG(LogTemp, Warning, TEXT("This door is not configured for this car."));
        return;
    }

    if (DoorIdx == EVehicleDoor::All) {
        for (int i = 0; i < ConstraintsComponents.Num(); i++)
        {
            CloseDoorPhys(EVehicleDoor(i));
        }
        return;
    }

    CloseDoorPhys(DoorIdx);
}

void ACarlaWheeledVehicle::OpenDoorPhys(const EVehicleDoor DoorIdx)
{
    UPhysicsConstraintComponent* Constraint = ConstraintsComponents[static_cast<int>(DoorIdx)];
    UPrimitiveComponent* DoorComponent = ConstraintDoor[Constraint];
    DoorComponent->DetachFromComponent(
        FDetachmentTransformRules(EDetachmentRule::KeepWorld, false));
    FTransform DoorInitialTransform =
        DoorComponentsTransform[DoorComponent] * GetActorTransform();
    DoorComponent->SetWorldTransform(DoorInitialTransform);
    DoorComponent->SetSimulatePhysics(true);
    DoorComponent->SetCollisionProfileName(TEXT("BlockAll"));
    float AngleLimit = Constraint->ConstraintInstance.GetAngularSwing1Limit();
    FRotator AngularRotationOffset = Constraint->ConstraintInstance.AngularRotationOffset;

    if (Constraint->ConstraintInstance.AngularRotationOffset.Yaw < 0.0f)
    {
        AngleLimit = -AngleLimit;
    }
    Constraint->SetAngularOrientationTarget(FRotator(0, AngleLimit, 0));
    Constraint->SetAngularDriveParams(DoorOpenStrength, 1.0, 0.0);

    Constraint->InitComponentConstraint();

    UPhysicsConstraintComponent** CollisionDisable =
        CollisionDisableConstraints.Find(DoorComponent);
    if (CollisionDisable)
    {
        (*CollisionDisable)->InitComponentConstraint();
    }
}

void ACarlaWheeledVehicle::CloseDoorPhys(const EVehicleDoor DoorIdx)
{
    UPhysicsConstraintComponent* Constraint = ConstraintsComponents[static_cast<int>(DoorIdx)];
    UPrimitiveComponent* DoorComponent = ConstraintDoor[Constraint];
    FTransform DoorInitialTransform =
        DoorComponentsTransform[DoorComponent] * GetActorTransform();
    DoorComponent->SetSimulatePhysics(false);
    DoorComponent->SetCollisionProfileName(TEXT("NoCollision"));
    DoorComponent->SetWorldTransform(DoorInitialTransform);
    DoorComponent->AttachToComponent(
        GetMesh(), FAttachmentTransformRules(EAttachmentRule::KeepWorld, true));
}

void ACarlaWheeledVehicle::ApplyRolloverBehavior()
{
    auto roll = GetVehicleTransform().Rotator().Roll;

    // The angular velocity reduction is applied in 4 stages, to improve its smoothness.
    // Case 4 starts the timer to set the rollover flag, so users are notified.
    switch (RolloverBehaviorTracker) {
    case 0: CheckRollover(roll, std::make_pair(130.0, 230.0));      break;
    case 1: CheckRollover(roll, std::make_pair(140.0, 220.0));      break;
    case 2: CheckRollover(roll, std::make_pair(150.0, 210.0));      break;
    case 3: CheckRollover(roll, std::make_pair(160.0, 200.0));      break;
    case 4:
        GetWorld()->GetTimerManager().SetTimer(TimerHandler, this, &ACarlaWheeledVehicle::SetRolloverFlag, RolloverFlagTime);
        RolloverBehaviorTracker += 1;
        break;
    case 5: break;
    default:
        RolloverBehaviorTracker = 5;
    }

    // In case the vehicle recovers, reset the rollover tracker
    if (RolloverBehaviorTracker > 0 && -30 < roll && roll < 30) {
        RolloverBehaviorTracker = 0;
        FailureState = carla::rpc::VehicleFailureState::None;
    }
}

void ACarlaWheeledVehicle::CheckRollover(const float roll, const std::pair<float, float> threshold_roll) {
    if (threshold_roll.first < roll && roll < threshold_roll.second) {
        auto RootComponent = Cast<UPrimitiveComponent>(GetRootComponent());
        auto AngularVelocity = RootComponent->GetPhysicsAngularVelocityInDegrees();
        RootComponent->SetPhysicsAngularVelocityInDegrees((1 - RolloverBehaviorForce) * AngularVelocity);
        RolloverBehaviorTracker += 1;
    }
}

void ACarlaWheeledVehicle::SetRolloverFlag() {
    // Make sure the vehicle hasn't recovered since the timer started
    if (RolloverBehaviorTracker >= 4) {
        FailureState = carla::rpc::VehicleFailureState::Rollover;
    }
}

carla::rpc::VehicleFailureState ACarlaWheeledVehicle::GetFailureState() const {
    return FailureState;
}

void ACarlaWheeledVehicle::AddReferenceToManager()
{
    const UObject* World = GetWorld();
    TArray<AActor*> ActorsInLevel;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), ActorsInLevel);
    for (AActor* Actor : ActorsInLevel)
    {
        AVegetationManager* Manager = Cast<AVegetationManager>(Actor);
        if (!IsValid(Manager))
            continue;
        Manager->AddVehicle(this);
        return;
    }
}

void ACarlaWheeledVehicle::RemoveReferenceToManager()
{
    const UObject* World = GetWorld();
    TArray<AActor*> ActorsInLevel;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), ActorsInLevel);
    for (AActor* Actor : ActorsInLevel)
    {
        AVegetationManager* Manager = Cast<AVegetationManager>(Actor);
        if (!IsValid(Manager))
            continue;
        Manager->RemoveVehicle(this);
        return;
    }
}
