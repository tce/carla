// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "WheeledVehicleMovementComponentNW.h"
#include "Components/PrimitiveComponent.h"
#include "Logging/MessageLog.h"

UWheeledVehicleMovementComponentNW::UWheeledVehicleMovementComponentNW(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
#if 0 // @TODO
	// grab default values from physx
	PxVehicleEngineData DefEngineData;
	EngineData.MOI = DefEngineData.mMOI;
	EngineData.MaxRPM = OmegaToRPM(DefEngineData.mMaxOmega);
	EngineData.DampingRateFullThrottle = DefEngineData.mDampingRateFullThrottle;
	EngineData.DampingRateZeroThrottleClutchEngaged = DefEngineData.mDampingRateZeroThrottleClutchEngaged;
	EngineData.DampingRateZeroThrottleClutchDisengaged = DefEngineData.mDampingRateZeroThrottleClutchDisengaged;

	// Convert from PhysX curve to ours
	FRichCurve* TorqueCurveData = EngineData.TorqueCurve.GetRichCurve();
	for (PxU32 KeyIdx = 0; KeyIdx < DefEngineData.mTorqueCurve.getNbDataPairs(); ++KeyIdx)
	{
		float Input = DefEngineData.mTorqueCurve.getX(KeyIdx) * EngineData.MaxRPM;
		float Output = DefEngineData.mTorqueCurve.getY(KeyIdx) * DefEngineData.mPeakTorque;
		TorqueCurveData->AddKey(Input, Output);
	}

	PxVehicleClutchData DefClutchData;
	TransmissionData.ClutchStrength = DefClutchData.mStrength;

	PxVehicleGearsData DefGearSetup;
	TransmissionData.GearSwitchTime = DefGearSetup.mSwitchTime;
	TransmissionData.ReverseGearRatio = DefGearSetup.mRatios[PxVehicleGearsData::eREVERSE];
	TransmissionData.FinalRatio = DefGearSetup.mFinalRatio;

	PxVehicleAutoBoxData DefAutoBoxSetup;
	TransmissionData.NeutralGearUpRatio = DefAutoBoxSetup.mUpRatios[PxVehicleGearsData::eNEUTRAL];
	TransmissionData.GearAutoBoxLatency = DefAutoBoxSetup.getLatency();
	TransmissionData.bUseGearAutoBox = true;

	for (uint32 i = PxVehicleGearsData::eFIRST; i < DefGearSetup.mNbRatios; ++i)
	{
		FVehicleNWGearData GearData;
		GearData.DownRatio = DefAutoBoxSetup.mDownRatios[i];
		GearData.UpRatio = DefAutoBoxSetup.mUpRatios[i];
		GearData.Ratio = DefGearSetup.mRatios[i];
		TransmissionData.ForwardGears.Add(GearData);
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
	DifferentialData.SetNum(NbrWheels);

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
		for (int32 GearIdx = 0; GearIdx < TransmissionData.ForwardGears.Num(); ++GearIdx)
		{
			FVehicleNWGearData& GearData = TransmissionData.ForwardGears[GearIdx];
			GearData.DownRatio = FMath::Min(GearData.DownRatio, GearData.UpRatio);
		}
	}
	else if (PropertyName == TEXT("UpRatio"))
	{
		for (int32 GearIdx = 0; GearIdx < TransmissionData.ForwardGears.Num(); ++GearIdx)
		{
			FVehicleNWGearData& GearData = TransmissionData.ForwardGears[GearIdx];
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

int32 UWheeledVehicleMovementComponentNW::GetCustomGearBoxNumForwardGears() const
{
	return TransmissionData.ForwardGears.Num();
}

void UWheeledVehicleMovementComponentNW::SetupVehicle(TUniquePtr<Chaos::FSimpleWheeledVehicle> & PVehicle)
{
#if 0 // @TODO
	if (!UpdatedPrimitive)
	{
		return;
	}

	if (WheelSetups.Num() < 2)
	{
		PVehicle = nullptr;
		PVehicleDrive = nullptr;
		return;
	}

	for (int32 WheelIdx = 0; WheelIdx < WheelSetups.Num(); ++WheelIdx)
	{
		const FWheelSetup& WheelSetup = WheelSetups[WheelIdx];
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

	SetUseAutomaticGears(TransmissionData.bUseGearAutoBox);
#endif
}

void UWheeledVehicleMovementComponentNW::Update(float DeltaTime)
{
#if 0 // @TODO
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

void UWheeledVehicleMovementComponentNW::UpdateEngineSetup(const FVehicleNWEngineData& NewEngineSetup)
{
#if 0 // @TODO
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
#if 0 // @TODO
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
#if 0 // @TODO
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

void UWheeledVehicleMovementComponentNW::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);
#if 0 // @TODO
	if (Ar.IsLoading() && Ar.UE4Ver() < VER_UE4_VEHICLES_UNIT_CHANGE)
	{
		PxVehicleEngineData DefEngineData;
		const float DefaultRPM = OmegaToRPM(DefEngineData.mMaxOmega);

		// We need to convert from old units to new. This backwards compatible code fails in the rare case that they were using very strange values that are the new defaults in the correct units.
		EngineData.MaxRPM = EngineData.MaxRPM != DefaultRPM ? OmegaToRPM(EngineData.MaxRPM) : DefaultRPM;	//need to convert from rad/s to RPM
	}

	if (Ar.IsLoading() && Ar.UE4Ver() < VER_UE4_VEHICLES_UNIT_CHANGE2)
	{
		PxVehicleEngineData DefEngineData;
		PxVehicleClutchData DefClutchData;

		// We need to convert from old units to new. This backwards compatable code fails in the rare case that they were using very strange values that are the new defaults in the correct units.
		BackwardsConvertCm2ToM2NW(EngineData.DampingRateFullThrottle, DefEngineData.mDampingRateFullThrottle);
		BackwardsConvertCm2ToM2NW(EngineData.DampingRateZeroThrottleClutchDisengaged, DefEngineData.mDampingRateZeroThrottleClutchDisengaged);
		BackwardsConvertCm2ToM2NW(EngineData.DampingRateZeroThrottleClutchEngaged, DefEngineData.mDampingRateZeroThrottleClutchEngaged);
		BackwardsConvertCm2ToM2NW(EngineData.MOI, DefEngineData.mMOI);
		BackwardsConvertCm2ToM2NW(TransmissionData.ClutchStrength, DefClutchData.mStrength);
	}
#endif
}

void UWheeledVehicleMovementComponentNW::ComputeConstants()
{
#if 0 // @TODO
	Super::ComputeConstants();
	MaxEngineRPM = EngineData.MaxRPM;
#endif
}
