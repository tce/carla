// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "Carla/Weather/Weather.h"
#include "Carla/Weather/Sky.h"
#include "Carla/Sensor/SceneCaptureCamera.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SkyLightComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/StaticMeshComponent.h"
#include "ConstructorHelpers.h"

AWeather::AWeather(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    const auto PrecipitationPostProcessMaterialPath =
        TEXT("Material'/Game/Carla/Static/GenericMaterials/00_MastersOpt/Screen_posProcess/M_screenDrops.M_screenDrops'");
    const auto DustStormPostProcessMaterialPath =
        TEXT("Material'/Game/Carla/Static/GenericMaterials/00_MastersOpt/Screen_posProcess/M_screenDust_wind.M_screenDust_wind'");

    PrecipitationPostProcessMaterial = ConstructorHelpers::FObjectFinder<UMaterial>(
        PrecipitationPostProcessMaterialPath).Object;
    DustStormPostProcessMaterial = ConstructorHelpers::FObjectFinder<UMaterial>(
        DustStormPostProcessMaterialPath).Object;

    PrimaryActorTick.bCanEverTick = false;
    RootComponent = ObjectInitializer.CreateDefaultSubobject<USceneComponent>(
        this,
        TEXT("RootComponent"));
}

void AWeather::ApplyWeather(const FWeatherParameters& InWeather)
{
    SetWeatherParameters(InWeather);
    CheckWeatherPostProcessEffects();

#ifdef CARLA_WEATHER_EXTRA_LOG
    UE_LOG(LogCarla, Log, TEXT("Changing weather:"));
    UE_LOG(LogCarla, Log, TEXT("  - Cloudiness = %.2f"), CurrentWeather.Cloudiness);
    UE_LOG(LogCarla, Log, TEXT("  - Precipitation = %.2f"), CurrentWeather.Precipitation);
    UE_LOG(LogCarla, Log, TEXT("  - PrecipitationDeposits = %.2f"), CurrentWeather.PrecipitationDeposits);
    UE_LOG(LogCarla, Log, TEXT("  - WindIntensity = %.2f"), CurrentWeather.WindIntensity);
    UE_LOG(LogCarla, Log, TEXT("  - SunAzimuthAngle = %.2f"), CurrentWeather.SunAzimuthAngle);
    UE_LOG(LogCarla, Log, TEXT("  - SunAltitudeAngle = %.2f"), CurrentWeather.SunAltitudeAngle);
    UE_LOG(LogCarla, Log, TEXT("  - FogDensity = %.2f"), CurrentWeather.FogDensity);
    UE_LOG(LogCarla, Log, TEXT("  - FogDistance = %.2f"), CurrentWeather.FogDistance);
    UE_LOG(LogCarla, Log, TEXT("  - FogFalloff = %.2f"), CurrentWeather.FogFalloff);
    UE_LOG(LogCarla, Log, TEXT("  - Wetness = %.2f"), CurrentWeather.Wetness);
    UE_LOG(LogCarla, Log, TEXT("  - ScatteringIntensity = %.2f"), CurrentWeather.ScatteringIntensity);
    UE_LOG(LogCarla, Log, TEXT("  - MieScatteringScale = %.2f"), CurrentWeather.MieScatteringScale);
    UE_LOG(LogCarla, Log, TEXT("  - RayleighScatteringScale = %.2f"), CurrentWeather.RayleighScatteringScale);
    UE_LOG(LogCarla, Log, TEXT("  - DustStorm = %.2f"), CurrentWeather.DustStorm);
#endif // CARLA_WEATHER_EXTRA_LOG

    // Call the blueprint that actually changes the weather.
    RefreshWeather(CurrentWeather);
}

void AWeather::NotifyWeather(ASensor* Sensor)
{
    auto AsSceneCaptureCamera = Cast<ASceneCaptureCamera>(Sensor);
    
    if (AsSceneCaptureCamera != nullptr)
    {
        SceneCaptureCameras.Add(AsSceneCaptureCamera);
    }

    CheckWeatherPostProcessEffects();

    // Call the blueprint that actually changes the weather.
    RefreshWeather(CurrentWeather);
}

void AWeather::SetWeatherParameters(const FWeatherParameters& InWeather)
{
    CurrentWeather = InWeather;
}

const FWeatherParameters& AWeather::GetWeatherParameters() const
{
    return CurrentWeather;
}

const bool& AWeather::GetDayNightCycle() const
{
    return DayNightCycle;
}

UDirectionalLightComponent* AWeather::GetSunDirectionalLight() const
{
    return DirectionalLight;
}

USkyLightComponent* AWeather::GetSkyLight() const
{
    return SkyLight;
}

UExponentialHeightFogComponent* AWeather::GetExponentialHeightFog() const
{
    return ExponentialHeightFog;
}

UStaticMeshComponent* AWeather::GetPlanetMesh() const
{
    return PlanetMesh;
}

void AWeather::SetDayNightCycle(const bool& active)
{
    DayNightCycle = active;
}

void AWeather::SetSunDirectionalLight(UDirectionalLightComponent* InDirectionalLight)
{
    check(InDirectionalLight != nullptr);
    DirectionalLight = InDirectionalLight;
}

void AWeather::SetSkyLight(USkyLightComponent* InSkyLight)
{
    check(InSkyLight != nullptr);
    SkyLight = InSkyLight;
}

void AWeather::SetExponentialHeightFog(UExponentialHeightFogComponent* InExponentialHeightFog)
{
    check(InExponentialHeightFog != nullptr);
    ExponentialHeightFog = InExponentialHeightFog;
}

void AWeather::SetPlanetMesh(UStaticMeshComponent* InPlanetMesh)
{
    check(InPlanetMesh != nullptr);
    PlanetMesh = InPlanetMesh;
}

float AWeather::GetSunIntensityCurveTime() const
{
    auto SunPosition = CurrentWeather.SunAltitudeAngle;
    SunPosition = FMath::Fmod(SunPosition + 270.0F, 360.0F) - 180.0F;
    SunPosition = FMath::Abs(SunPosition / 180.0F) * 2.0F - 1.0F;
    return SunPosition;
}

void AWeather::UpdateSunDirectionalLight()
{
    auto Yaw = CurrentWeather.SunAzimuthAngle;
    auto Pitch = CurrentWeather.SunAltitudeAngle;
    DirectionalLight->SetWorldRotation(FRotator(Pitch, Yaw, 0.0));
}

void AWeather::ValidateCurrentWeatherParameters()
{
    CurrentWeather.PrecipitationDeposits = FMath::Clamp(
        CurrentWeather.PrecipitationDeposits,
        0, 100);

    CurrentWeather.FogDensity = FMath::Clamp(
        CurrentWeather.FogDensity,
        0, 100);

    CurrentWeather.Wetness = FMath::Clamp(
        CurrentWeather.Wetness,
        0, 100);

    CurrentWeather.DustStorm = FMath::Clamp(
        CurrentWeather.DustStorm,
        0, 100);
}

void AWeather::SetSkyComponents(ASky* InSky)
{
    check(InSky != nullptr);
    SetSunDirectionalLight(InSky->GetDirectionalLight());
    SetSkyLight(InSky->GetSkyLight());
    SetExponentialHeightFog(InSky->GetExponentialHeightFog());
}

void AWeather::CheckWeatherPostProcessEffects()
{
    if (CurrentWeather.Precipitation > 0.0f)
    {
        ActiveBlendables.Add(MakeTuple(
            PrecipitationPostProcessMaterial,
            CurrentWeather.Precipitation / 100.0f));
    }
    else
    {
        ActiveBlendables.Remove(PrecipitationPostProcessMaterial);
    }

    if (CurrentWeather.DustStorm > 0.0f)
    {
        ActiveBlendables.Add(MakeTuple(DustStormPostProcessMaterial, CurrentWeather.DustStorm / 100.0f));
    }
    else
    {
        ActiveBlendables.Remove(DustStormPostProcessMaterial);
    }

    for (int32 i = 0; i < SceneCaptureCameras.Num(); i++)
    {
        auto& Sensor = SceneCaptureCameras[i];

        if (!IsValid(Sensor))
        {
            SceneCaptureCameras.RemoveAtSwap(i);
            if (i == SceneCaptureCameras.Num())
                break;
        }

        for (auto& ActiveBlendable : ActiveBlendables)
        {
            Sensor->GetCaptureComponent2D()->PostProcessSettings.AddBlendable(
                ActiveBlendable.Key,
                ActiveBlendable.Value);
        }
    }
}
