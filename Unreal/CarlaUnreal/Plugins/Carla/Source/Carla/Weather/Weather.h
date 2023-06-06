// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "GameFramework/Actor.h"
#include "Carla/Weather/WeatherParameters.h"

#include "Weather.generated.h"

class ASky;
class ASensor;
class ASceneCaptureCamera;

class UDirectionalLightComponent;
class USkyLightComponent;
class UExponentialHeightFogComponent;
class UStaticMeshComponent;



UCLASS(Abstract)
class CARLA_API AWeather : public AActor
{
    GENERATED_BODY()

public:

    AWeather(const FObjectInitializer& ObjectInitializer);

    /// Update the weather parameters and notifies it to the blueprint's event
    UFUNCTION(BlueprintCallable)
    void ApplyWeather(const FWeatherParameters& WeatherParameters);

    /// Notifing the weather to the blueprint's event
    void NotifyWeather(ASensor* Sensor = nullptr);

    /// Update the weather parameters without notifing it to the blueprint's event
    UFUNCTION(BlueprintCallable)
    void SetWeatherParameters(const FWeatherParameters& WeatherParameters);

    /// Returns the current WeatherParameters
    UFUNCTION(BlueprintCallable, BlueprintPure)
    const FWeatherParameters& GetWeatherParameters() const;

    /// Returns whether the day night cycle is active (automatic on/off switch when changin to night mode)
    UFUNCTION(BlueprintCallable, BlueprintPure)
    const bool& GetDayNightCycle() const;

    UFUNCTION(BlueprintCallable, BlueprintPure)
    UDirectionalLightComponent* GetSunDirectionalLight() const;
    
    UFUNCTION(BlueprintCallable, BlueprintPure)
    USkyLightComponent* GetSkyLight() const;
    
    UFUNCTION(BlueprintCallable, BlueprintPure)
    UExponentialHeightFogComponent* GetExponentialHeightFog() const;
    
    UFUNCTION(BlueprintCallable, BlueprintPure)
    UStaticMeshComponent* GetPlanetMesh() const;

    /// Update the day night cycle
    void SetDayNightCycle(const bool& active);
    
protected:

    UFUNCTION(BlueprintImplementableEvent)
    void RefreshWeather(const FWeatherParameters& WeatherParameters);
    
    UFUNCTION(BlueprintCallable)
    void UpdateSunDirectionalLight();
    
    UFUNCTION(BlueprintCallable)
    void ValidateCurrentWeatherParameters();

    UFUNCTION(BlueprintCallable)
    void SetSkyComponents(ASky* InSky);

    UFUNCTION(BlueprintCallable)
    void SetSunDirectionalLight(UDirectionalLightComponent* InDirectionalLight);

    UFUNCTION(BlueprintCallable)
    void SetSkyLight(USkyLightComponent* InSkyLight);

    UFUNCTION(BlueprintCallable)
    void SetExponentialHeightFog(UExponentialHeightFogComponent* InExponentialHeightFog);

    UFUNCTION(BlueprintCallable)
    void SetPlanetMesh(UStaticMeshComponent* InPlanetMesh);
    
    UFUNCTION(BlueprintCallable, BlueprintPure)
    float GetSunIntensityCurveTime() const;

private:

    void CheckWeatherPostProcessEffects();

    UPROPERTY(VisibleAnywhere)
    FWeatherParameters CurrentWeather;

    UPROPERTY(VisibleAnywhere)
    TArray<ASceneCaptureCamera*> SceneCaptureCameras;

    TMap<UMaterial*, float> ActiveBlendables;

    TObjectPtr<UMaterial> PrecipitationPostProcessMaterial;

    TObjectPtr<UMaterial> DustStormPostProcessMaterial;

    TObjectPtr<UDirectionalLightComponent> DirectionalLight;
    TObjectPtr<USkyLightComponent> SkyLight;
    TObjectPtr<UExponentialHeightFogComponent> ExponentialHeightFog;
    TObjectPtr<UStaticMeshComponent> PlanetMesh;

    UPROPERTY(EditAnywhere, Category = "Weather")
    bool DayNightCycle = true;
};
