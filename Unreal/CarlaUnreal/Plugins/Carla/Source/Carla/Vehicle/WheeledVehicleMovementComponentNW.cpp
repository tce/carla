// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "WheeledVehicleMovementComponentNW.h"
#include "PhysicsPublic.h"
#include "Components/PrimitiveComponent.h"
#include "Logging/MessageLog.h"

FVehicleNWEngineData FVehicleNWEngineData::FromChaosConfig(FVehicleEngineConfig VehicleEngineConfig)
{
	FVehicleNWEngineData Out;
	Out.TorqueCurve = VehicleEngineConfig.TorqueCurve;
	Out.MaxRPM = VehicleEngineConfig.MaxRPM;
	Out.MOI = VehicleEngineConfig.EngineRevUpMOI;
	Out.DampingRateFullThrottle = 0.0F; // @TODO
	Out.DampingRateZeroThrottleClutchEngaged = 0.0F; // @TODO
	Out.DampingRateZeroThrottleClutchDisengaged = 0.0F; // @TODO
	return Out;
}

FVehicleEngineConfig FVehicleNWEngineData::ToChaosConfig() const
{
	FVehicleEngineConfig Out;
	Out.TorqueCurve = TorqueCurve;
	Out.MaxTorque = FindPeakTorque();
	Out.MaxRPM = MaxRPM;
	Out.EngineIdleRPM = MOI;
	Out.EngineBrakeEffect = 0.0F; // @TODO
	Out.EngineRevUpMOI = 0.0F; // @TODO
	Out.EngineRevDownRate = 0.0F; // @TODO
	return Out;
}

FVehicleNWTransmissionData FVehicleNWTransmissionData::FromChaosConfig(FVehicleTransmissionConfig VehicleEngineConfig)
{
	const auto& PhysicsConfig = VehicleEngineConfig.GetPhysicsTransmissionConfig();

	FVehicleNWTransmissionData Out;
	Out.bUseGearAutoBox = VehicleEngineConfig.bUseAutomaticGears;
	Out.GearSwitchTime = VehicleEngineConfig.GearChangeTime;
	Out.GearAutoBoxLatency = 0.0F; // @TODO
	Out.FinalRatio = PhysicsConfig.FinalDriveRatio;
	Out.ForwardGears.SetNum(PhysicsConfig.ForwardRatios.Num());
	for (int32 i = 0; i != PhysicsConfig.ForwardRatios.Num(); ++i)
	{
		auto& OutGear = Out.ForwardGears[i];
		OutGear.UpRatio = PhysicsConfig.ForwardRatios[i];
		OutGear.DownRatio = PhysicsConfig.ReverseRatios[i];
		OutGear.Ratio = VehicleEngineConfig.GetGearRatio(i);
	}
	Out.ReverseGearRatio = 0.0F; // @TODO
	Out.NeutralGearUpRatio = 0.0F; // @TODO
	Out.ClutchStrength = 0.0F; // @TODO
	return Out;
}

FVehicleTransmissionConfig FVehicleNWTransmissionData::ToChaosConfig() const
{
	FVehicleTransmissionConfig Out;
	return Out;
}

UWheeledVehicleMovementComponentNW::UWheeledVehicleMovementComponentNW(
	const FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer)
{
}

#if WITH_EDITOR
void UWheeledVehicleMovementComponentNW::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	const FName PropertyName =
		PropertyChangedEvent.Property ?
		PropertyChangedEvent.Property->GetFName() :
		NAME_None;

	if (PropertyName == TEXT("DownRatio"))
	{
		for (int32 GearIdx = 0; GearIdx < TransmissionSetup.ReverseGearRatios.Num(); ++GearIdx)
		{
			auto GearData = GetGearData(GearIdx);
			TransmissionSetup.ReverseGearRatios[GearIdx] = FMath::Min(GearData.DownRatio, GearData.UpRatio);
		}
	}
	else if (PropertyName == TEXT("UpRatio"))
	{
		for (int32 GearIdx = 0; GearIdx < TransmissionSetup.ForwardGearRatios.Num(); ++GearIdx)
		{
			auto GearData = GetGearData(GearIdx);
			TransmissionSetup.ForwardGearRatios[GearIdx] = FMath::Max(GearData.DownRatio, GearData.UpRatio);
		}
	}
	else if (PropertyName == TEXT("SteeringCurve"))
	{
		//make sure values are capped between 0 and 1
		TArray<FRichCurveKey> SteerKeys = SteeringCurve.GetRichCurve()->GetCopyOfKeys();
		for (int32 KeyIdx = 0; KeyIdx < SteerKeys.Num(); ++KeyIdx)
		{
			float NewValue = FMath::Clamp(SteerKeys[KeyIdx].Value, 0.0f, 1.0f);
			SteeringCurve.GetRichCurve()->UpdateOrAddKey(SteerKeys[KeyIdx].Time, NewValue);
		}
	}
}
#endif

FVehicleNWGearData UWheeledVehicleMovementComponentNW::GetGearData(int32 Index) const
{
	FVehicleNWGearData Out;
	Out.UpRatio = TransmissionSetup.ForwardGearRatios[Index];
	Out.DownRatio = TransmissionSetup.ReverseGearRatios[Index];
	Out.DownRatio = TransmissionSetup.GetGearRatio(Index);
	return Out;
}

float FVehicleNWEngineData::FindPeakTorque() const
{
	float PeakTorque = 0.0f;

	for (auto& Key : TorqueCurve.GetRichCurveConst()->GetConstRefOfKeys())
		PeakTorque = FMath::Max(PeakTorque, Key.Value);

	return PeakTorque;
}

int32 UWheeledVehicleMovementComponentNW::GetCustomGearBoxNumForwardGears() const
{
	return TransmissionSetup.ForwardGearRatios.Num();
}

void UWheeledVehicleMovementComponentNW::SetupVehicle(
	TUniquePtr<Chaos::FSimpleWheeledVehicle>& PVehicle)
{
	if (!UpdatedPrimitive)
		return;

	if (WheelSetups.Num() < 2)
	{
		PVehicle = nullptr;
		return;
	}

	for (const auto& WheelSetup : WheelSetups)
		if (WheelSetup.BoneName == NAME_None)
			return;

	SetupVehicleShapes();
	SetupVehicleMass();

	auto TargetInstance = UpdatedPrimitive->GetBodyInstance();
	auto ActorHandle = TargetInstance->GetPhysicsActorHandle();

	FPhysicsCommand::ExecuteWrite(ActorHandle, [](const FPhysicsActorHandle& ActorHandle)
		{
			// @TODO
		});

	SetUseAutomaticGears(TransmissionSetup.bUseAutomaticGears);
}

void UWheeledVehicleMovementComponentNW::UpdateSimulation(float DeltaTime)
{
	auto TargetInstance = UpdatedPrimitive->GetBodyInstance();
	auto ActorHandle = TargetInstance->GetPhysicsActorHandle();

	FPhysicsCommand::ExecuteWrite(ActorHandle, [](const FPhysicsActorHandle& ActorHandle)
	{
			// @TODO
	});
}

void UWheeledVehicleMovementComponentNW::UpdateEngineSetup(const FVehicleNWEngineData& NewEngineSetup)
{
	// @TODO
}

void UWheeledVehicleMovementComponentNW::UpdateDifferentialSetup(const TArray<FVehicleNWWheelDifferentialData>& NewDifferentialSetup)
{
	// @TODO
}

void UWheeledVehicleMovementComponentNW::UpdateTransmissionSetup(const FVehicleNWTransmissionData& NewTransmissionSetup)
{
	// @TODO
}

void UWheeledVehicleMovementComponentNW::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);
	// @TODO
}

void UWheeledVehicleMovementComponentNW::ComputeConstants()
{
	Super::ComputeConstants();
	// @TODO
}