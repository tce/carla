// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "WheeledVehicleMovementComponentNW.h"
#include "PhysicsPublic.h"
#include "Components/PrimitiveComponent.h"
#include "Logging/MessageLog.h"

UWheeledVehicleMovementComponentNW::UWheeledVehicleMovementComponentNW(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	FVehicleEngineConfig EngineDefaults;
	const auto& PhysicsEngineConfig = EngineDefaults.GetPhysicsEngineConfig();
	CarlaEngineSetup.MOI = EngineDefaults.EngineRevUpMOI;
	CarlaEngineSetup.MaxRPM = EngineDefaults.MaxRPM;
	CarlaEngineSetup.DampingRateFullThrottle = 0; // @TODO
	CarlaEngineSetup.DampingRateZeroThrottleClutchEngaged = 0; // @TODO
	CarlaEngineSetup.DampingRateZeroThrottleClutchDisengaged = 0; // @TODO
	
	const auto OutTorqueCurve = CarlaEngineSetup.TorqueCurve.GetRichCurve();
	const auto InTorqueCurve = EngineDefaults.TorqueCurve.GetRichCurve();
	*OutTorqueCurve = *InTorqueCurve;

	CarlaTransmissionSetup.ClutchStrength = 0; // @TODO
	CarlaTransmissionSetup.GearSwitchTime = 0; // @TODO
	CarlaTransmissionSetup.ReverseGearRatio = 0; // @TODO
	CarlaTransmissionSetup.FinalRatio = 0; // @TODO
	CarlaTransmissionSetup.NeutralGearUpRatio = 0; // @TODO
	CarlaTransmissionSetup.GearAutoBoxLatency = 0; // @TODO
	CarlaTransmissionSetup.bUseGearAutoBox = 0; // @TODO

	const auto GearRatioCount = std::min(
		TransmissionSetup.ForwardGearRatios.Num(),
		TransmissionSetup.ReverseGearRatios.Num());

	CarlaTransmissionSetup.ForwardGears.SetNum(GearRatioCount);
	for (int32 i = 0; i != GearRatioCount; ++i)
	{
		auto& Out = CarlaTransmissionSetup.ForwardGears[i];
		Out.Ratio = TransmissionSetup.GetGearRatio(i);
		Out.UpRatio = TransmissionSetup.ForwardGearRatios[i];
		Out.DownRatio = TransmissionSetup.ReverseGearRatios[i];
	}

	auto& SCurve = *SteeringCurve.GetRichCurve();
	SCurve.AddKey(0.0f, 1.0f);
	SCurve.AddKey(20.0f, 0.9f);
	SCurve.AddKey(60.0f, 0.8f);
	SCurve.AddKey(120.0f, 0.7f);

	// Initialize WheelSetups array with 4 wheels, this can be modified via editor later
	const int32 NbrWheels = 4;
	WheelSetups.SetNum(NbrWheels);
	CarlaDifferentialSetup.SetNum(NbrWheels);

	IdleBrakeInput = 10;

#if 0 // @CARLA_UE5
	// grab default values from physx
	PxVehicleEngineData DefEngineData;
	CarlaEngineSetup.MOI = DefEngineData.mMOI;
	CarlaEngineSetup.MaxRPM = OmegaToRPM(DefEngineData.mMaxOmega);
	CarlaEngineSetup.DampingRateFullThrottle = DefEngineData.mDampingRateFullThrottle;
	CarlaEngineSetup.DampingRateZeroThrottleClutchEngaged = DefEngineData.mDampingRateZeroThrottleClutchEngaged;
	CarlaEngineSetup.DampingRateZeroThrottleClutchDisengaged = DefEngineData.mDampingRateZeroThrottleClutchDisengaged;

	// Convert from PhysX curve to ours
	FRichCurve* TorqueCurveData = CarlaEngineSetup.TorqueCurve.GetRichCurve();
	for (PxU32 KeyIdx = 0; KeyIdx < DefEngineData.mTorqueCurve.getNbDataPairs(); ++KeyIdx)
	{
		float Input = DefEngineData.mTorqueCurve.getX(KeyIdx) * CarlaEngineSetup.MaxRPM;
		float Output = DefEngineData.mTorqueCurve.getY(KeyIdx) * DefEngineData.mPeakTorque;
		TorqueCurveData->AddKey(Input, Output);
	}

	PxVehicleClutchData DefClutchData;
	CarlaTransmissionSetup.ClutchStrength = DefClutchData.mStrength;

	PxVehicleGearsData DefGearSetup;
	CarlaTransmissionSetup.GearSwitchTime = DefGearSetup.mSwitchTime;
	CarlaTransmissionSetup.ReverseGearRatio = DefGearSetup.mRatios[PxVehicleGearsData::eREVERSE];
	CarlaTransmissionSetup.FinalRatio = DefGearSetup.mFinalRatio;

	PxVehicleAutoBoxData DefAutoBoxSetup;
	CarlaTransmissionSetup.NeutralGearUpRatio = DefAutoBoxSetup.mUpRatios[PxVehicleGearsData::eNEUTRAL];
	CarlaTransmissionSetup.GearAutoBoxLatency = DefAutoBoxSetup.getLatency();
	CarlaTransmissionSetup.bUseGearAutoBox = true;

	for (uint32 i = PxVehicleGearsData::eFIRST; i < DefGearSetup.mNbRatios; ++i)
	{
		FVehicleNWGearData GearData;
		GearData.DownRatio = DefAutoBoxSetup.mDownRatios[i];
		GearData.UpRatio = DefAutoBoxSetup.mUpRatios[i];
		GearData.Ratio = DefGearSetup.mRatios[i];
		CarlaTransmissionSetup.ForwardGears.Add(GearData);
	}

	// Init steering speed curve
	FRichCurve* SteeringCurveData = SteeringCurve.GetRichCurve();
	SteeringCurveData->AddKey(0.0f, 1.0f);
	SteeringCurveData->AddKey(20.0f, 0.9f);
	SteeringCurveData->AddKey(60.0f, 0.8f);
	SteeringCurveData->AddKey(120.0f, 0.7f);

	// Initialize WheelSetups array with 4 wheels, this can be modified via editor later
	const int32 NbrWheels = 4;
	WheelSetups.SetNum(NbrWheels);
	CarlaDifferentialSetup.SetNum(NbrWheels);

	IdleBrakeInput = 10;
#endif
}

#if WITH_EDITOR
void UWheeledVehicleMovementComponentNW::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	const FName PropertyName = PropertyChangedEvent.Property ? PropertyChangedEvent.Property->GetFName() : NAME_None;

	if (PropertyName == TEXT("DownRatio"))
	{
		for (int32 GearIdx = 0; GearIdx < CarlaTransmissionSetup.ForwardGears.Num(); ++GearIdx)
		{
			FVehicleNWGearData& GearData = CarlaTransmissionSetup.ForwardGears[GearIdx];
			GearData.DownRatio = FMath::Min(GearData.DownRatio, GearData.UpRatio);
		}
	}
	else if (PropertyName == TEXT("UpRatio"))
	{
		for (int32 GearIdx = 0; GearIdx < CarlaTransmissionSetup.ForwardGears.Num(); ++GearIdx)
		{
			FVehicleNWGearData& GearData = CarlaTransmissionSetup.ForwardGears[GearIdx];
			GearData.UpRatio = FMath::Max(GearData.DownRatio, GearData.UpRatio);
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

#if 0 // @CARLA_UE5
static void GetVehicleDifferentialNWSetup(const TArray<FVehicleNWWheelDifferentialData>& Setup, PxVehicleDifferentialNWData& PxSetup)
{
	for (int32 i = 0; i < Setup.Num(); ++i)
	{
		PxSetup.setDrivenWheel(i, Setup[i].bDriven);
	}
}
#endif

float FVehicleNWEngineData::FindPeakTorque() const
{
	// Find max torque
	float PeakTorque = 0.0f;
	TArray<FRichCurveKey> TorqueKeys = TorqueCurve.GetRichCurveConst()->GetCopyOfKeys();
	for (int32 KeyIdx = 0; KeyIdx < TorqueKeys.Num(); ++KeyIdx)
	{
		FRichCurveKey& Key = TorqueKeys[KeyIdx];
		PeakTorque = FMath::Max(PeakTorque, Key.Value);
	}
	return PeakTorque;
}

#if 0 // @CARLA_UE5
static void GetVehicleEngineSetup(const FVehicleNWEngineData& Setup, PxVehicleEngineData& PxSetup)
{
	PxSetup.mMOI = M2ToCm2(Setup.MOI);
	PxSetup.mMaxOmega = RPMToOmega(Setup.MaxRPM);
	PxSetup.mDampingRateFullThrottle = M2ToCm2(Setup.DampingRateFullThrottle);
	PxSetup.mDampingRateZeroThrottleClutchEngaged = M2ToCm2(Setup.DampingRateZeroThrottleClutchEngaged);
	PxSetup.mDampingRateZeroThrottleClutchDisengaged = M2ToCm2(Setup.DampingRateZeroThrottleClutchDisengaged);

	float PeakTorque = Setup.FindPeakTorque(); // In Nm
	PxSetup.mPeakTorque = M2ToCm2(PeakTorque);	// convert Nm to (kg cm^2/s^2)

	// Convert from our curve to PhysX
	PxSetup.mTorqueCurve.clear();
	TArray<FRichCurveKey> TorqueKeys = Setup.TorqueCurve.GetRichCurveConst()->GetCopyOfKeys();
	int32 NumTorqueCurveKeys = FMath::Min<int32>(TorqueKeys.Num(), PxVehicleEngineData::eMAX_NB_ENGINE_TORQUE_CURVE_ENTRIES);
	for (int32 KeyIdx = 0; KeyIdx < NumTorqueCurveKeys; ++KeyIdx)
	{
		FRichCurveKey& Key = TorqueKeys[KeyIdx];
		PxSetup.mTorqueCurve.addPair(FMath::Clamp(Key.Time / Setup.MaxRPM, 0.0f, 1.0f), Key.Value / PeakTorque); // Normalize torque to 0-1 range
	}
}

static void GetVehicleGearSetup(const FVehicleNWTransmissionData& Setup, PxVehicleGearsData& PxSetup)
{
	PxSetup.mSwitchTime = Setup.GearSwitchTime;
	PxSetup.mRatios[PxVehicleGearsData::eREVERSE] = Setup.ReverseGearRatio;
	for (int32 i = 0; i < Setup.ForwardGears.Num(); i++)
	{
		PxSetup.mRatios[i + PxVehicleGearsData::eFIRST] = Setup.ForwardGears[i].Ratio;
	}
	PxSetup.mFinalRatio = Setup.FinalRatio;
	PxSetup.mNbRatios = Setup.ForwardGears.Num() + PxVehicleGearsData::eFIRST;
}

static void GetVehicleAutoBoxSetup(const FVehicleNWTransmissionData& Setup, PxVehicleAutoBoxData& PxSetup)
{
	for (int32 i = 0; i < Setup.ForwardGears.Num(); ++i)
	{
		const FVehicleNWGearData& GearData = Setup.ForwardGears[i];
		PxSetup.mUpRatios[i + PxVehicleGearsData::eFIRST] = GearData.UpRatio;
		PxSetup.mDownRatios[i + PxVehicleGearsData::eFIRST] = GearData.DownRatio;
	}
	PxSetup.mUpRatios[PxVehicleGearsData::eNEUTRAL] = Setup.NeutralGearUpRatio;
	PxSetup.setLatency(Setup.GearAutoBoxLatency);

}
#endif

int32 UWheeledVehicleMovementComponentNW::GetCustomGearBoxNumForwardGears() const
{
	return CarlaTransmissionSetup.ForwardGears.Num();
}

#if 0 // @CARLA_UE5
static void SetupDriveHelper(const UWheeledVehicleMovementComponentNW* VehicleData, const PxVehicleWheelsSimData* PWheelsSimData, PxVehicleDriveSimDataNW& DriveData)
{
	PxVehicleDifferentialNWData CarlaDifferentialSetup;
	GetVehicleDifferentialNWSetup(VehicleData->CarlaDifferentialSetup, CarlaDifferentialSetup);

	DriveData.setDiffData(CarlaDifferentialSetup);

	PxVehicleEngineData CarlaEngineSetup;
	GetVehicleEngineSetup(VehicleData->CarlaEngineSetup, CarlaEngineSetup);
	DriveData.setEngineData(CarlaEngineSetup);

	PxVehicleClutchData ClutchSetup;
	ClutchSetup.mStrength = M2ToCm2(VehicleData->CarlaTransmissionSetup.ClutchStrength);
	DriveData.setClutchData(ClutchSetup);

	PxVehicleGearsData GearSetup;
	GetVehicleGearSetup(VehicleData->CarlaTransmissionSetup, GearSetup);
	DriveData.setGearsData(GearSetup);

	PxVehicleAutoBoxData AutoBoxSetup;
	GetVehicleAutoBoxSetup(VehicleData->CarlaTransmissionSetup, AutoBoxSetup);
	DriveData.setAutoBoxData(AutoBoxSetup);
}
#endif

void UWheeledVehicleMovementComponentNW::SetupVehicle(
	TUniquePtr<Chaos::FSimpleWheeledVehicle>& PVehicle)
{
	if (!UpdatedPrimitive)
	{
		return;
	}

	if (WheelSetups.Num() < 2)
	{
		PVehicle = nullptr;
		return;
	}

	for (auto& Setup : WheelSetups)
		if (Setup.BoneName == NAME_None)
			return;

	SetupVehicleShapes();
	SetupVehicleMass();
	auto TargetInstance = UpdatedPrimitive->GetBodyInstance();
	FPhysicsCommand::ExecuteWrite(
		TargetInstance->GetPhysicsActorHandle(),
		[&](const FPhysicsActorHandle& InHandle)
		{
			// @TODO

		});

#if 0 // @CARLA_UE5

	if (WheelSetups.Num() < 2)
	{
		PVehicle = nullptr;
		PVehicleDrive = nullptr;
		return;
	}

	for (int32 WheelIdx = 0; WheelIdx < WheelSetups.Num(); ++WheelIdx)
	{
		const FChaosWheelSetup& WheelSetup = WheelSetups[WheelIdx];
		if (WheelSetup.BoneName == NAME_None)
		{
			return;
		}
	}

	// Setup the chassis and wheel shapes
	SetupVehicleShapes();

	// Setup mass properties
	SetupVehicleMass();

	// Setup the wheels
	PxVehicleWheelsSimData* PWheelsSimData = PxVehicleWheelsSimData::allocate(WheelSetups.Num());
	SetupWheels(PWheelsSimData);

	// Setup drive data
	PxVehicleDriveSimDataNW DriveData;
	SetupDriveHelper(this, PWheelsSimData, DriveData);

	// Create the vehicle
	PxVehicleDriveNW* PVehicleDriveNW = PxVehicleDriveNW::allocate(WheelSetups.Num());
	check(PVehicleDriveNW);

	FBodyInstance* TargetInstance = UpdatedPrimitive->GetBodyInstance();

	FPhysicsCommand::ExecuteWrite(TargetInstance->ActorHandle, [&](const FPhysicsActorHandle& Actor)
		{
			PxRigidActor* PActor = FPhysicsInterface::GetPxRigidActor_AssumesLocked(Actor);
			if (!PActor)
			{
				return;
			}

			if (PxRigidDynamic* PVehicleActor = PActor->is<PxRigidDynamic>())
			{
				PVehicleDriveNW->setup(GPhysXSDK, PVehicleActor, *PWheelsSimData, DriveData, 0);
				PVehicleDriveNW->setToRestState();

				// cleanup
				PWheelsSimData->free();
			}
		});

	PWheelsSimData = nullptr;

	// cache values
	PVehicle = PVehicleDriveNW;
	PVehicleDrive = PVehicleDriveNW;

	SetUseAutoGears(CarlaTransmissionSetup.bUseGearAutoBox);
#endif
}

void UWheeledVehicleMovementComponentNW::UpdateSimulation(float DeltaTime)
{
	auto TargetInstance = UpdatedPrimitive->GetBodyInstance();

	FPhysicsCommand::ExecuteWrite(
		TargetInstance->GetPhysicsActorHandle(),
		[&](const FPhysicsActorHandle& InHandle)
		{
		});

#if 0 // @CARLA_UE5
	if (PVehicleDrive == nullptr)
		return;

	FBodyInstance* TargetInstance = UpdatedPrimitive->GetBodyInstance();

	FPhysicsCommand::ExecuteWrite(TargetInstance->ActorHandle, [&](const FPhysicsActorHandle& Actor)
	{
		PxVehicleDriveNWRawInputData RawInputData;
		RawInputData.setAnalogAccel(ThrottleInput);
		RawInputData.setAnalogSteer(SteeringInput);
		RawInputData.setAnalogBrake(BrakeInput);
		RawInputData.setAnalogHandbrake(HandbrakeInput);

		if (!PVehicleDrive->mDriveDynData.getUseAutoGears())
		{
			RawInputData.setGearUp(bRawGearUpInput);
			RawInputData.setGearDown(bRawGearDownInput);
		}

		// Convert from our curve to PxFixedSizeLookupTable
		PxFixedSizeLookupTable<8> SpeedSteerLookup;
		TArray<FRichCurveKey> SteerKeys = SteeringCurve.GetRichCurve()->GetCopyOfKeys();
		const int32 MaxSteeringSamples = FMath::Min(8, SteerKeys.Num());
		for (int32 KeyIdx = 0; KeyIdx < MaxSteeringSamples; KeyIdx++)
		{
			FRichCurveKey& Key = SteerKeys[KeyIdx];
			SpeedSteerLookup.addPair(KmHToCmS(Key.Time), FMath::Clamp(Key.Value, 0.0f, 1.0f));
		}

		PxVehiclePadSmoothingData SmoothData = {
			{ ThrottleInputRate.RiseRate, BrakeInputRate.RiseRate, HandbrakeInputRate.RiseRate, SteeringInputRate.RiseRate, SteeringInputRate.RiseRate },
			{ ThrottleInputRate.FallRate, BrakeInputRate.FallRate, HandbrakeInputRate.FallRate, SteeringInputRate.FallRate, SteeringInputRate.FallRate }
		};

		PxVehicleDriveNW* PVehicleDriveNW = (PxVehicleDriveNW*)PVehicleDrive;
		PxVehicleDriveNWSmoothAnalogRawInputsAndSetAnalogInputs(SmoothData, SpeedSteerLookup, RawInputData, DeltaTime, false, *PVehicleDriveNW);
	});
#endif
}

static FVehicleEngineConfig ToChaosEngineConfig(const FVehicleNWEngineData& In)
{
	FVehicleEngineConfig Out = {};
	auto& OutPhysics = Out.GetPhysicsEngineConfig();
	Out.EngineRevUpMOI = In.MOI;
	Out.MaxRPM = In.MaxRPM;
	// PxSetup.mDampingRateFullThrottle = M2ToCm2(Setup.DampingRateFullThrottle); // @TODO
	// PxSetup.mDampingRateZeroThrottleClutchEngaged = M2ToCm2(Setup.DampingRateZeroThrottleClutchEngaged); // @TODO
	// PxSetup.mDampingRateZeroThrottleClutchDisengaged = M2ToCm2(Setup.DampingRateZeroThrottleClutchDisengaged); // @TODO
	Out.TorqueCurve = In.TorqueCurve;
	Out.MaxTorque = In.FindPeakTorque();
	return Out;
}

void UWheeledVehicleMovementComponentNW::UpdateEngineSetup(const FVehicleNWEngineData& NewEngineSetup)
{
	// @TODO ?
	if (VehicleSimulationPT != nullptr)
	{
		EngineSetup = ToChaosEngineConfig(NewEngineSetup);
	}

#if 0 // @CARLA_UE5
	if (PVehicleDrive)
	{
		PxVehicleEngineData EngineData;
		GetVehicleEngineSetup(NewEngineSetup, EngineData);

		PxVehicleDriveNW* PVehicleDriveNW = (PxVehicleDriveNW*)PVehicleDrive;
		PVehicleDriveNW->mDriveSimData.setEngineData(EngineData);
	}
#endif
}

void UWheeledVehicleMovementComponentNW::UpdateDifferentialSetup(const TArray<FVehicleNWWheelDifferentialData>& NewDifferentialSetup)
{
	if (VehicleSimulationPT != nullptr)
	{
		// @TODO
	}

#if 0 // @CARLA_UE5
	if (PVehicleDrive)
	{
		PxVehicleDifferentialNWData DifferentialData;
		GetVehicleDifferentialNWSetup(NewDifferentialSetup, DifferentialData);

		PxVehicleDriveNW* PVehicleDriveNW = (PxVehicleDriveNW*)PVehicleDrive;
		PVehicleDriveNW->mDriveSimData.setDiffData(DifferentialData);
	}
#endif
}

void UWheeledVehicleMovementComponentNW::UpdateTransmissionSetup(const FVehicleNWTransmissionData& NewTransmissionSetup)
{
#if 0 // @CARLA_UE5
	if (PVehicleDrive)
	{
		PxVehicleGearsData GearData;
		GetVehicleGearSetup(NewTransmissionSetup, GearData);

		PxVehicleAutoBoxData AutoBoxData;
		GetVehicleAutoBoxSetup(NewTransmissionSetup, AutoBoxData);

		PxVehicleDriveNW* PVehicleDriveNW = (PxVehicleDriveNW*)PVehicleDrive;
		PVehicleDriveNW->mDriveSimData.setGearsData(GearData);
		PVehicleDriveNW->mDriveSimData.setAutoBoxData(AutoBoxData);
	}
#endif
}

#if 0 // @CARLA_UE5
static void BackwardsConvertCm2ToM2NW(float& val, float defaultValue)
{
	if (val != defaultValue)
	{
		val = Cm2ToM2(val);
	}
}
#endif

void UWheeledVehicleMovementComponentNW::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);
#if 0 // @CARLA_UE5
	if (Ar.IsLoading() && Ar.UEVer() < VER_UE4_VEHICLES_UNIT_CHANGE)
	{
		PxVehicleEngineData DefEngineData;
		const float DefaultRPM = OmegaToRPM(DefEngineData.mMaxOmega);

		// We need to convert from old units to new. This backwards compatible code fails in the rare case that they were using very strange values that are the new defaults in the correct units.
		CarlaEngineSetup.MaxRPM = CarlaEngineSetup.MaxRPM != DefaultRPM ? OmegaToRPM(CarlaEngineSetup.MaxRPM) : DefaultRPM;	//need to convert from rad/s to RPM
	}

	if (Ar.IsLoading() && Ar.UEVer() < VER_UE4_VEHICLES_UNIT_CHANGE2)
	{
		PxVehicleEngineData DefEngineData;
		PxVehicleClutchData DefClutchData;

		// We need to convert from old units to new. This backwards compatable code fails in the rare case that they were using very strange values that are the new defaults in the correct units.
		BackwardsConvertCm2ToM2NW(CarlaEngineSetup.DampingRateFullThrottle, DefEngineData.mDampingRateFullThrottle);
		BackwardsConvertCm2ToM2NW(CarlaEngineSetup.DampingRateZeroThrottleClutchDisengaged, DefEngineData.mDampingRateZeroThrottleClutchDisengaged);
		BackwardsConvertCm2ToM2NW(CarlaEngineSetup.DampingRateZeroThrottleClutchEngaged, DefEngineData.mDampingRateZeroThrottleClutchEngaged);
		BackwardsConvertCm2ToM2NW(CarlaEngineSetup.MOI, DefEngineData.mMOI);
		BackwardsConvertCm2ToM2NW(CarlaTransmissionSetup.ClutchStrength, DefClutchData.mStrength);
	}
#endif
}

void UWheeledVehicleMovementComponentNW::ComputeConstants()
{
	Super::ComputeConstants();
	EngineSetup.MaxRPM = CarlaEngineSetup.MaxRPM;
}

#if 0 // @CARLA_UE5
const void* UWheeledVehicleMovementComponentNW::GetTireData(physx::PxVehicleWheels* InWheels, UChaosVehicleWheel* InWheel)
{
	const void* realShaderData = &InWheels->mWheelsSimData.getTireData((PxU32)InWheel->WheelIndex);
	return realShaderData;
}

const int32 UWheeledVehicleMovementComponentNW::GetWheelShapeMapping(physx::PxVehicleWheels* InWheels, uint32 InWheel)
{
	const physx::PxI32 ShapeIndex = InWheels->mWheelsSimData.getWheelShapeMapping((PxU32)InWheel);
	return ShapeIndex;
}

const physx::PxVehicleWheelData UWheeledVehicleMovementComponentNW::GetWheelData(physx::PxVehicleWheels* InWheels, uint32 InWheel)
{
	const physx::PxVehicleWheelData WheelData = InWheels->mWheelsSimData.getWheelData((physx::PxU32)InWheel);
	return WheelData;
}
#endif
