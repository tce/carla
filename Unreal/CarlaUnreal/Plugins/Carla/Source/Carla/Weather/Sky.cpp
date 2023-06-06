#include "Sky.h"
#include "Carla.h"
#include "WeatherParameters.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/VolumetricCloudComponent.h"
#include "Components/PostProcessComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/LevelStreamingDynamic.h"
#include "Misc/DateTime.h"

void ASky::Setup(const FSkyParameters& SkyParameters)
{
	DirectionalLight = CreateDefaultSubobject<UDirectionalLightComponent>(TEXT("ASky::DirectionalLight"));
	SkyLight = CreateDefaultSubobject<USkyLightComponent>(TEXT("ASky::SkyLight"));
	SkyAtmosphere = CreateDefaultSubobject<USkyAtmosphereComponent>(TEXT("ASky::SkyAtmosphere"));
	VolumetricCloud = CreateDefaultSubobject<UVolumetricCloudComponent>(TEXT("ASky::VolumetricCloud"));
	PostProcess = CreateDefaultSubobject<UPostProcessComponent>(TEXT("ASky::PostProcess"));
	ExponentialHeightFog = CreateDefaultSubobject<UExponentialHeightFogComponent>(TEXT("ASky::ExponentialHeightFog"));
	PlanetMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ASky::PlanetMesh"));
}

ASky::ASky(const FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer)
{
	Setup({});
}

ASky::ASky(
	const FObjectInitializer& ObjectInitializer,
	const FSkyParameters& SkyParameters) :
	Super(ObjectInitializer)
{
	Setup(SkyParameters);
}

void ASky::SetDirectionalLight(UDirectionalLightComponent* InDirectionalLight)
{
#if WITH_EDITOR
	UE_LOG(LogCarla, Error, TEXT("ASky::SetDirectionalLight was passed an invalid UDirectionalLightComponent."));
#else
	check(InDirectionalLight);
#endif

	DirectionalLight = InDirectionalLight;
}

void ASky::SetSkyLight(USkyLightComponent* InSkyLight)
{
#if WITH_EDITOR
	UE_LOG(LogCarla, Error, TEXT("ASky::SetSkyLight was passed an invalid USkyLightComponent."));
#else
	check(InSkyLight);
#endif

	SkyLight = InSkyLight;
}

void ASky::SetSkyAtmosphere(USkyAtmosphereComponent* InSkyAtmosphere)
{
#if WITH_EDITOR
	UE_LOG(LogCarla, Error, TEXT("ASky::SetSkyAtmosphere was passed an invalid USkyAtmosphereComponent."));
#else
	check(InSkyAtmosphere);
#endif

	SkyAtmosphere = InSkyAtmosphere;
}

void ASky::SetExponentialHeightFog(UExponentialHeightFogComponent* InExponentialHeightFog)
{
#if WITH_EDITOR
	UE_LOG(LogCarla, Error, TEXT("ASky::SetExponentialHeightFog was passed an invalid UExponentialHeightFogComponent."));
#else
	check(InExponentialHeightFog);
#endif

	ExponentialHeightFog = InExponentialHeightFog;
}

void ASky::SetVolumetricCloud(UVolumetricCloudComponent* InVolumetricCloud)
{
#if WITH_EDITOR
	UE_LOG(LogCarla, Error, TEXT("ASky::SetVolumetricCloud was passed an invalid UVolumetricCloudComponent."));
#else
	check(InVolumetricCloud);
#endif

	VolumetricCloud = InVolumetricCloud;
}

void ASky::SetPostProcess(UPostProcessComponent* InPostProcess)
{
#if WITH_EDITOR
	UE_LOG(LogCarla, Error, TEXT("ASky::SetPostProcess was passed an invalid UPostProcessComponent."));
#else
	check(InPostProcess);
#endif

	PostProcess = InPostProcess;
}

void ASky::SetPlanetMesh(UStaticMeshComponent* InStaticMesh)
{
#if WITH_EDITOR
	UE_LOG(LogCarla, Error, TEXT("ASky::SetPlanetMesh was passed an invalid UStaticMeshComponent."));
#else
	check(InStaticMesh);
#endif

	PlanetMesh = InStaticMesh;
}

FTimespan ASky::GetTimespanFromSolarTime(float SolarTime)
{
	const auto Hour = (int32)FMath::TruncToFloat(SolarTime) % 24U;
	const auto Minute = (int32)FMath::TruncToFloat((SolarTime - Hour) * 60.0F) % 60;
	const auto Second = (int32)FMath::TruncToFloat(((SolarTime - Hour) - (Minute / 60.0F)) * 3600 + 0.5F) % 60;
	return FTimespan(Hour, Minute, Second);
}

FTimespan ASky::GetTimeOfDay() const
{
	return GetTimespanFromSolarTime(GetSolarTime());
}

bool ASky::IsValidDateTime(FDateTime DateTime)
{
	return FDateTime::Validate(
		DateTime.GetYear(), DateTime.GetMonth(),
		DateTime.GetDay(), DateTime.GetHour(),
		DateTime.GetMinute(), DateTime.GetSecond(),
		DateTime.GetMillisecond());
}

bool ASky::IsDaylightSavingTime(
	FDateTime CurrentDate,
	int32 StartMonth, int32 StartDay,
	int32 EndMonth, int32 EndDay,
	int32 SwitchHour) const
{
	const auto NowHMS = GetTimeOfDay();
	const auto Now = FDateTime(
		CurrentDate.GetYear(), CurrentDate.GetMonth(), CurrentDate.GetDay(),
		NowHMS.GetHours(), NowHMS.GetMinutes(), NowHMS.GetSeconds(), 0);
	const auto Start = FDateTime(CurrentDate.GetYear(), StartMonth, StartDay, SwitchHour);
	const auto End = FDateTime(CurrentDate.GetYear(), EndMonth, EndDay, SwitchHour);
	return Now >= Start && Now <= End;
}

// Taken from USunPositionFunctionLibrary::GetSunPosition:
//  - see "Engine\Plugins\Runtime\SunPosition\Source\SunPosition\Private\SunPosition.cpp".
void ASky::SetSunPosition(
	float Latitude,
	float Longitude,
	float TimeZone,
	bool IsDaylightSavingTime,
	FDateTime DateTime,
	float NorthOffset)
{
	if (!FDateTime::Validate(
		DateTime.GetYear(), DateTime.GetMonth(),
		DateTime.GetDay(), DateTime.GetHour(),
		DateTime.GetMinute(), DateTime.GetSecond(),
		DateTime.GetMillisecond()))
	{
		UE_LOG(LogCarla, Warning, TEXT("ASky::SetSunPosition: Invalid date."));
		return;
	}

	float TimeOffset = TimeZone;

	if (IsDaylightSavingTime)
		TimeOffset += 1.0f;

	double LatitudeRad = FMath::DegreesToRadians(Latitude);

	// Get the julian day (number of days since Jan 1st of the year 4713 BC)
	double JulianDay = DateTime.GetJulianDay();
	double JulianCentury = (JulianDay - 2451545.0) / 36525.0;

	// Get the sun's mean longitude , referred to the mean equinox of julian date
	double GeomMeanLongSunDeg = FMath::Fmod(280.46646 + JulianCentury * (36000.76983 + JulianCentury * 0.0003032), 360.0);
	double GeomMeanLongSunRad = FMath::DegreesToRadians(GeomMeanLongSunDeg);

	// Get the sun's mean anomaly
	double GeomMeanAnomSunDeg = 357.52911 + JulianCentury * (35999.05029 - 0.0001537 * JulianCentury);
	double GeomMeanAnomSunRad = FMath::DegreesToRadians(GeomMeanAnomSunDeg);

	// Get the earth's orbit eccentricity
	double EccentEarthOrbit = 0.016708634 - JulianCentury * (0.000042037 + 0.0000001267 * JulianCentury);

	// Get the sun's equation of the center
	double SunEqOfCtr = FMath::Sin(GeomMeanAnomSunRad) * (1.914602 - JulianCentury * (0.004817 + 0.000014 * JulianCentury))
		+ FMath::Sin(2.0 * GeomMeanAnomSunRad) * (0.019993 - 0.000101 * JulianCentury)
		+ FMath::Sin(3.0 * GeomMeanAnomSunRad) * 0.000289;

	// Get the sun's true longitude
	double SunTrueLongDeg = GeomMeanLongSunDeg + SunEqOfCtr;

	// Get the sun's true anomaly
	//	double SunTrueAnomDeg = GeomMeanAnomSunDeg + SunEqOfCtr;
	//	double SunTrueAnomRad = FMath::DegreesToRadians(SunTrueAnomDeg);

	// Get the earth's distance from the sun
	//	double SunRadVectorAUs = (1.000001018*(1.0 - EccentEarthOrbit*EccentEarthOrbit)) / (1.0 + EccentEarthOrbit*FMath::Cos(SunTrueAnomRad));

	// Get the sun's apparent longitude
	double SunAppLongDeg = SunTrueLongDeg - 0.00569 - 0.00478 * FMath::Sin(FMath::DegreesToRadians(125.04 - 1934.136 * JulianCentury));
	double SunAppLongRad = FMath::DegreesToRadians(SunAppLongDeg);

	// Get the earth's mean obliquity of the ecliptic
	double MeanObliqEclipticDeg = 23.0 + (26.0 + ((21.448 - JulianCentury * (46.815 + JulianCentury * (0.00059 - JulianCentury * 0.001813)))) / 60.0) / 60.0;

	// Get the oblique correction
	double ObliqCorrDeg = MeanObliqEclipticDeg + 0.00256 * FMath::Cos(FMath::DegreesToRadians(125.04 - 1934.136 * JulianCentury));
	double ObliqCorrRad = FMath::DegreesToRadians(ObliqCorrDeg);

	// Get the sun's right ascension
	double SunRtAscenRad = FMath::Atan2(FMath::Cos(ObliqCorrRad) * FMath::Sin(SunAppLongRad), FMath::Cos(SunAppLongRad));
	double SunRtAscenDeg = FMath::RadiansToDegrees(SunRtAscenRad);

	// Get the sun's declination
	double SunDeclinRad = FMath::Asin(FMath::Sin(ObliqCorrRad) * FMath::Sin(SunAppLongRad));
	double SunDeclinDeg = FMath::RadiansToDegrees(SunDeclinRad);

	double VarY = FMath::Pow(FMath::Tan(ObliqCorrRad / 2.0), 2.0);

	// Get the equation of time
	double EqOfTimeMinutes = 4.0 * FMath::RadiansToDegrees(VarY * FMath::Sin(2.0 * GeomMeanLongSunRad) - 2.0 * EccentEarthOrbit * FMath::Sin(GeomMeanAnomSunRad) + 4.0 * EccentEarthOrbit * VarY * FMath::Sin(GeomMeanAnomSunRad) * FMath::Cos(2.0 * GeomMeanLongSunRad) - 0.5 * VarY * VarY * FMath::Sin(4.0 * GeomMeanLongSunRad) - 1.25 * EccentEarthOrbit * EccentEarthOrbit * FMath::Sin(2.0 * GeomMeanAnomSunRad));

	// Get the hour angle of the sunrise
	double HASunriseDeg = FMath::RadiansToDegrees(FMath::Acos(FMath::Cos(FMath::DegreesToRadians(90.833)) / (FMath::Cos(LatitudeRad) * FMath::Cos(SunDeclinRad)) - FMath::Tan(LatitudeRad) * FMath::Tan(SunDeclinRad)));
	//	double SunlightDurationMinutes = 8.0 * HASunriseDeg;

	// Get the local time of the sun's rise and set
	double SolarNoonLST = (720.0 - 4.0 * Longitude - EqOfTimeMinutes + TimeOffset * 60.0) / 1440.0;
	double SunriseTimeLST = SolarNoonLST - HASunriseDeg * 4.0 / 1440.0;
	double SunsetTimeLST = SolarNoonLST + HASunriseDeg * 4.0 / 1440.0;

	// Get the true solar time
	double TrueSolarTimeMinutes = FMath::Fmod(DateTime.GetTimeOfDay().GetTotalMinutes() + EqOfTimeMinutes + 4.0 * Longitude - 60.0 * TimeOffset, 1440.0);

	// Get the hour angle of current time
	double HourAngleDeg = TrueSolarTimeMinutes < 0 ? TrueSolarTimeMinutes / 4.0 + 180 : TrueSolarTimeMinutes / 4.0 - 180.0;
	double HourAngleRad = FMath::DegreesToRadians(HourAngleDeg);

	// Get the solar zenith angle
	double SolarZenithAngleRad = FMath::Acos(FMath::Sin(LatitudeRad) * FMath::Sin(SunDeclinRad) + FMath::Cos(LatitudeRad) * FMath::Cos(SunDeclinRad) * FMath::Cos(HourAngleRad));
	double SolarZenithAngleDeg = FMath::RadiansToDegrees(SolarZenithAngleRad);

	// Get the sun elevation
	double SolarElevationAngleDeg = 90.0 - SolarZenithAngleDeg;
	double SolarElevationAngleRad = FMath::DegreesToRadians(SolarElevationAngleDeg);
	double TanOfSolarElevationAngle = FMath::Tan(SolarElevationAngleRad);

	// Get the approximated atmospheric refraction
	double ApproxAtmosphericRefractionDeg = 0.0;
	if (SolarElevationAngleDeg <= 85.0)
	{
		if (SolarElevationAngleDeg > 5.0)
		{
			ApproxAtmosphericRefractionDeg = 58.1 / TanOfSolarElevationAngle - 0.07 / FMath::Pow(TanOfSolarElevationAngle, 3) + 0.000086 / FMath::Pow(TanOfSolarElevationAngle, 5) / 3600.0;
		}
		else
		{
			if (SolarElevationAngleDeg > -0.575)
			{
				ApproxAtmosphericRefractionDeg = 1735.0 + SolarElevationAngleDeg * (-518.2 + SolarElevationAngleDeg * (103.4 + SolarElevationAngleDeg * (-12.79 + SolarElevationAngleDeg * 0.711)));
			}
			else
			{
				ApproxAtmosphericRefractionDeg = -20.772 / TanOfSolarElevationAngle;
			}
		}
		ApproxAtmosphericRefractionDeg /= 3600.0;
	}

	// Get the corrected solar elevation
	double SolarElevationcorrectedforatmrefractionDeg = SolarElevationAngleDeg + ApproxAtmosphericRefractionDeg;

	// Get the solar azimuth 
	double tmp = FMath::RadiansToDegrees(FMath::Acos(((FMath::Sin(LatitudeRad) * FMath::Cos(SolarZenithAngleRad)) - FMath::Sin(SunDeclinRad)) / (FMath::Cos(LatitudeRad) * FMath::Sin(SolarZenithAngleRad))));
	double SolarAzimuthAngleDegcwfromN = HourAngleDeg > 0.0 ? FMath::Fmod(tmp + 180.0, 360.0) : FMath::Fmod(540.0 - tmp, 360.0);

	// offset elevation angle to fit with UE coords system
	const auto Elevation = 180.0 + SolarElevationAngleDeg;
	const auto CorrectedElevation = 180.0 + SolarElevationcorrectedforatmrefractionDeg;
	const auto Azimuth = SolarAzimuthAngleDegcwfromN;
	const auto SolarNoon = FTimespan::FromDays(SolarNoonLST);
	const auto SunriseTime = FTimespan::FromDays(SunriseTimeLST);
	const auto SunsetTime = FTimespan::FromDays(SunsetTimeLST);

	auto Pitch = CorrectedElevation;
	auto Yaw = Azimuth + (double)NorthOffset;
	DirectionalLight->SetWorldRotation(FRotator(Pitch, Yaw, 0.0));
}

void ASky::SetSolarTime(float NewSolarTime)
{
	SolarTime = NewSolarTime;
}

void ASky::SetSkyParameters(const FSkyParameters& SkyParameters)
{
	Parameters = SkyParameters;
	UpdateChildComponents();
}

void ASky::UpdateDirectionalLight()
{
	const auto& Params = Parameters.DirectionalLight;
	if (IsValid(DirectionalLight))
	{
		DirectionalLight->Deactivate();
		DirectionalLight->SetIntensity(Params.Intensity);
		DirectionalLight->SetLightColor(Params.Color);
		DirectionalLight->SetUseTemperature(true);
		DirectionalLight->SetTemperature(Params.Temperature);
		DirectionalLight->SetEnableLightShaftOcclusion(Params.EnableLightShaftOcclusion);
		DirectionalLight->SetEnableLightShaftBloom(Params.EnableLightShaftBloom);
		DirectionalLight->SetVolumetricScatteringIntensity(Params.ScatteringIntensity);
		DirectionalLight->SetDynamicShadowDistanceMovableLight(Parameters.Shadows.CascadeShadowDistance);
		DirectionalLight->ContactShadowLength = Parameters.Shadows.ContactShadowLength;
		SetSunRotationFromAltitudeAndAzimuth(Params.SunAltitude, Params.SunAzimuth);
		DirectionalLight->Activate();
	}
}

void ASky::UpdateSkyLight()
{
	const auto& Params = Parameters.SkyLight;
	if (IsValid(SkyLight))
	{
		SkyLight->Deactivate();
		SkyLight->SetIntensity(Params.Intensity);
		SkyLight->SetLightColor(Params.Color);
		SkyLight->SetOcclusionContrast(Parameters.Shadows.DFAOContrast);
		SkyLight->SetOcclusionExponent(Parameters.Shadows.DFAOExponent);
		SkyLight->SetMinOcclusion(Parameters.Shadows.DFAOMinOcclusion);
		SkyLight->SetOcclusionTint(Parameters.Shadows.DFAOTint);
		SkyLight->bRealTimeCapture = true;
		SkyLight->Activate();
	}
}

void ASky::UpdateSkyAtmosphere()
{
	const auto& Params = Parameters.SkyAtmosphere;
	if (IsValid(SkyAtmosphere))
	{
		SkyAtmosphere->Deactivate();
		SkyAtmosphere->SetRayleighScatteringScale(Params.AirParticlesDensity);
		SkyAtmosphere->SetMieScatteringScale(Params.PollutionParticlesDensity);
		SkyAtmosphere->SetHeightFogContribution(Params.HeightFogContribution);
		SkyAtmosphere->Activate();
	}
}

void ASky::UpdateExponentialHeightFog()
{
	const auto& Params = Parameters.ExponentialHeightFog;
	if (IsValid(ExponentialHeightFog))
	{
#if 0
		ExponentialHeightFog->Deactivate();
		ExponentialHeightFog->SetFogInscatteringColor(Params.Color);
		ExponentialHeightFog->SetFogDensity(Params.Density);
		ExponentialHeightFog->SetFogHeightFalloff(Params.Falloff);
		ExponentialHeightFog->SetVolumetricFog(Params.VolumetricEnable);
		ExponentialHeightFog->SetVolumetricFogExtinctionScale(1.0F - Params.FogDistance / 100.0F);
		ExponentialHeightFog->Activate();
#endif
	}
}

void ASky::UpdateVolumetricCloud()
{
	const auto& Params = Parameters.VolumetricCloud;
	if (IsValid(VolumetricCloud))
	{
		VolumetricCloud->Deactivate();
		VolumetricCloud->Activate();
	}
}

void ASky::UpdatePostProcess()
{
	if (IsValid(PostProcess))
	{
		PostProcess->Deactivate();
		PostProcess->Settings = Parameters.PostProcessSettings;
		PostProcess->Activate();
	}
}

void ASky::UpdateChildComponents()
{
	UpdateDirectionalLight();
	UpdateSkyLight();
	UpdateSkyAtmosphere();
	UpdateExponentialHeightFog();
	UpdateVolumetricCloud();
	UpdatePostProcess();
}

void ASky::SetSunRotationFromAltitudeAndAzimuth(float Altitude, float Azimuth)
{
	auto Pitch = Altitude;
	auto Yaw = Azimuth;
	DirectionalLight->SetWorldRotation(FRotator(Pitch, Yaw, 0.0));
}
