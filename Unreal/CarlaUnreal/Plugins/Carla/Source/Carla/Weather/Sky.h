#pragma once
#include "SkyParameters.h"
#include "WeatherReflectionRange.h"
#include "GameFramework/Actor.h"
#include "Sky.generated.h"



class UDirectionalLightComponent;
class USkyLightComponent;
class USkyAtmosphereComponent;
class UExponentialHeightFogComponent;
class UVolumetricCloudComponent;
class UPostProcessComponent;
class ULevelStreamingDynamic;
class UStaticMeshComponent;
struct FSkyParameters;
struct FWeatherParameters;



UCLASS(Abstract)
class CARLA_API ASky :
	public AActor
{
	GENERATED_BODY()

	void Setup(const FSkyParameters& SkyParameters);

public:

	ASky(const FObjectInitializer& ObjectInitializer);
	ASky(const FObjectInitializer& ObjectInitializer, const FSkyParameters& SkyParameters);

	
	
	UFUNCTION(BlueprintCallable)
	void SetDirectionalLight(UDirectionalLightComponent* InDirectionalLight);

	UFUNCTION(BlueprintCallable)
	void SetSkyLight(USkyLightComponent* InSkyLight);

	UFUNCTION(BlueprintCallable)
	void SetSkyAtmosphere(USkyAtmosphereComponent* InSkyAtmosphere);

	UFUNCTION(BlueprintCallable)
	void SetExponentialHeightFog(UExponentialHeightFogComponent* InExponentialHeightFog);

	UFUNCTION(BlueprintCallable)
	void SetVolumetricCloud(UVolumetricCloudComponent* InVolumetricCloud);

	UFUNCTION(BlueprintCallable)
	void SetPostProcess(UPostProcessComponent* InPostProcess);
	
	UFUNCTION(BlueprintCallable)
	void SetPlanetMesh(UStaticMeshComponent* InStaticMesh);


	
	UFUNCTION(BlueprintCallable, BlueprintPure)
	float GetSolarTime() const { return SolarTime; }
	
	UFUNCTION(BlueprintCallable, BlueprintPure)
	static FTimespan GetTimespanFromSolarTime(float SolarTime);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	FTimespan GetTimeOfDay() const;
	
	UFUNCTION(BlueprintCallable, BlueprintPure)
	static bool IsValidDateTime(FDateTime DateTime);
	
	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool IsDaylightSavingTime(
		FDateTime CurrentDate,
		int32 StartMonth, int32 StartDay,
		int32 EndMonth, int32 EndDay,
		int32 SwitchHour) const;

	UFUNCTION(BlueprintCallable, BlueprintPure)
	UDirectionalLightComponent* GetDirectionalLight() const { return DirectionalLight; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
	USkyLightComponent* GetSkyLight() const { return SkyLight; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
	USkyAtmosphereComponent* GetSkyAtmosphere() const { return SkyAtmosphere; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
	UExponentialHeightFogComponent* GetExponentialHeightFog() const { return ExponentialHeightFog; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
	UVolumetricCloudComponent* GetVolumetricCloud() const { return VolumetricCloud; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
	UPostProcessComponent* GetPostProcess() const { return PostProcess; }
	
	UFUNCTION(BlueprintCallable, BlueprintPure)
	UStaticMeshComponent* GetPlanetMesh() const { return PlanetMesh; }
	

	
	UFUNCTION(BlueprintCallable, CallInEditor)
	void SetSunPosition(
		float Latitude,
		float Longitude,
		float TimeZone,
		bool IsDaylightSavingTime,
		FDateTime DateTime,
		float NorthOffset);
	
	UFUNCTION(BlueprintCallable)
	void SetSolarTime(float NewSolarTime);
	
	UFUNCTION(BlueprintCallable, CallInEditor)
	void UpdateDirectionalLight(const FSkyParametersDirectionalLight& Parameters);

	UFUNCTION(BlueprintCallable, CallInEditor)
	void UpdateSkyLight(const FSkyParametersSkyLight& Parameters);

	UFUNCTION(BlueprintCallable, CallInEditor)
	void UpdateSkyAtmosphere(const FSkyParametersSkyAtmosphere& Parameters);

	UFUNCTION(BlueprintCallable, CallInEditor)
	void UpdateExponentialHeightFog(const FSkyParametersExponentialHeightFog& Parameters);

	UFUNCTION(BlueprintCallable, CallInEditor)
	void UpdateVolumetricCloud(const FSkyParametersVolumetricCloud& Parameters);

	UFUNCTION(BlueprintCallable, CallInEditor)
	void UpdatePostProcess(const FPostProcessSettings& Parameters);

	UFUNCTION(BlueprintCallable, CallInEditor)
	void UpdateChildComponents(const FSkyParameters& Parameters);

	UFUNCTION(BlueprintCallable)
	void SetSunRotationFromAltitudeAndAzimuth(float Altitude, float Azimuth);



	UPROPERTY(EditAnywhere, Category = "Parameters")
	float SolarTime;

protected:

	UPROPERTY(EditAnywhere, Category = "Components")
	TObjectPtr<UDirectionalLightComponent> DirectionalLight;

	UPROPERTY(EditAnywhere, Category = "Components")
	TObjectPtr<USkyLightComponent> SkyLight;

	UPROPERTY(EditAnywhere, Category = "Components")
	TObjectPtr<USkyAtmosphereComponent> SkyAtmosphere;

	UPROPERTY(EditAnywhere, Category = "Components")
	TObjectPtr<UExponentialHeightFogComponent> ExponentialHeightFog;

	UPROPERTY(EditAnywhere, Category = "Components")
	TObjectPtr<UVolumetricCloudComponent> VolumetricCloud;

	UPROPERTY(EditAnywhere, Category = "Components")
	TObjectPtr<UPostProcessComponent> PostProcess;

	UPROPERTY(EditAnywhere, Category = "Components")
	TObjectPtr<UStaticMeshComponent> PlanetMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FWeatherReflectionRange> WeatherReflectionRanges;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FString, ULevelStreamingDynamic*> ReflectionSublevelMap;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString CurrentLoadedLevel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString DefaultReflectionLevel;

};
