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

	UFUNCTION(BlueprintCallable, BlueprintPure)
	const FSkyParameters& GetSkyParameters() const { return Parameters; }
	

	
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
	void SetSkyParameters(const FSkyParameters& SkyParameters);

	UFUNCTION(BlueprintCallable, CallInEditor)
	void UpdateDirectionalLight();

	UFUNCTION(BlueprintCallable, CallInEditor)
	void UpdateSkyLight();

	UFUNCTION(BlueprintCallable, CallInEditor)
	void UpdateSkyAtmosphere();

	UFUNCTION(BlueprintCallable, CallInEditor)
	void UpdateExponentialHeightFog();

	UFUNCTION(BlueprintCallable, CallInEditor)
	void UpdateVolumetricCloud();

	UFUNCTION(BlueprintCallable, CallInEditor)
	void UpdatePostProcess();

	UFUNCTION(BlueprintCallable, CallInEditor)
	void UpdateChildComponents();

	UFUNCTION(BlueprintCallable)
	void SetSunRotationFromAltitudeAndAzimuth(float Altitude, float Azimuth);



	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Components")
	FSkyParameters Parameters;

protected:

	UPROPERTY(EditAnywhere)
	float SolarTime;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UDirectionalLightComponent> DirectionalLight;

	UPROPERTY(EditAnywhere)
	TObjectPtr<USkyLightComponent> SkyLight;

	UPROPERTY(EditAnywhere)
	TObjectPtr<USkyAtmosphereComponent> SkyAtmosphere;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UExponentialHeightFogComponent> ExponentialHeightFog;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UVolumetricCloudComponent> VolumetricCloud;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UPostProcessComponent> PostProcess;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UStaticMeshComponent> PlanetMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FWeatherReflectionRange> WeatherReflectionRanges;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FString, TObjectPtr<ULevelStreamingDynamic>> ReflectionSublevelMap;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString CurrentLoadedLevel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString DefaultReflectionLevel;

};