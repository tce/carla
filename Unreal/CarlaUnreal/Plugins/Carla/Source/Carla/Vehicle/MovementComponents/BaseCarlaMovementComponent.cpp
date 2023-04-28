// Copyright (c) 2021 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
// Copyright (c) 2019 Intel Corporation
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "BaseCarlaMovementComponent.h"
#include "Carla/Vehicle/CarlaWheeledVehicle.h"

#include <compiler/disable-ue4-macros.h>
#include <carla/Logging.h>
#include <compiler/enable-ue4-macros.h>

void UBaseCarlaMovementComponent::BeginPlay()
{
  Super::BeginPlay();
  ACarlaWheeledVehicle* Vehicle = Cast<ACarlaWheeledVehicle>(GetOwner());
  if (Vehicle)
  {
    CarlaVehicle = Vehicle;
  }
  else
  {
    UE_LOG(LogCarla, Warning, TEXT("Error: Owner is not properly set for UBaseCarlaMovementComponent") );
  }
}

void UBaseCarlaMovementComponent::ProcessControl(FVehicleControl &Control)
{

}

FVector UBaseCarlaMovementComponent::GetVelocity() const
{
#if 1 // @CARLA_UE5
  if (CarlaVehicle)
  {
    return CarlaVehicle->AWheeledVehiclePawn::GetVelocity();
  }
#endif
  return FVector();
}

int32 UBaseCarlaMovementComponent::GetVehicleCurrentGear() const
{
  return 0;
}

float UBaseCarlaMovementComponent::GetVehicleForwardSpeed() const
{
  return 0.f;
}

void UBaseCarlaMovementComponent::DisableUEVehiclePhysics()
{
#if 1 // @CARLA_UE5
  if(!CarlaVehicle)
  {
    UE_LOG(LogCarla, Warning, TEXT("Error: Owner is not properly set for UCarSimManagerComponent") );
    return;
  }
  CarlaVehicle->GetVehicleMovementComponent()->SetComponentTickEnabled(false);
  CarlaVehicle->GetVehicleMovementComponent()->Deactivate();
  CarlaVehicle->GetMesh()->PhysicsTransformUpdateMode = EPhysicsTransformUpdateMode::ComponentTransformIsKinematic;
  auto * Bone = CarlaVehicle->GetMesh()->GetBodyInstance(NAME_None);
  if (Bone)
  {
    Bone->SetInstanceSimulatePhysics(false);
  }
#endif
}

void UBaseCarlaMovementComponent::EnableUEVehiclePhysics(bool bResetVelocity)
{
#if 1 // @CARLA_UE5
  FVector CurrentVelocity(0, 0, 0);
  if (!bResetVelocity)
  {
    CurrentVelocity = GetVelocity();
  }
  CarlaVehicle->GetMesh()->SetPhysicsLinearVelocity(CurrentVelocity, false, "Vehicle_Base");
  CarlaVehicle->GetVehicleMovementComponent()->SetComponentTickEnabled(true);
  CarlaVehicle->GetVehicleMovementComponent()->Activate();
  CarlaVehicle->GetMesh()->PhysicsTransformUpdateMode = EPhysicsTransformUpdateMode::SimulationUpatesComponentTransform;
  auto * Bone = CarlaVehicle->GetMesh()->GetBodyInstance(NAME_None);
  if (Bone)
  {
    Bone->SetInstanceSimulatePhysics(true);
  }
  else
  {
    carla::log_warning("No bone with name");
  }
  CarlaVehicle->GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Block);
  CarlaVehicle->GetMesh()->SetCollisionProfileName("Vehicle");
  CarlaVehicle->RestoreVehiclePhysicsControl();
#endif
}
