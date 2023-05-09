#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Sky.generated.h"



class UDirectionalLightComponent;
class USkyLightComponent;
class USkyAtmosphereComponent;
class UExponentialHeightFogComponent;
class UVolumetricCloudComponent;
class UPostProcessComponent;
struct FSkyParameters;
struct FWeatherParameters;



UCLASS(BlueprintType)
class CARLA_API ASky : public AActor
{
	GENERATED_BODY()
public:

	ASky(const FObjectInitializer& ObjectInitializer);

	
	
	void UpdateParameters(const FSkyParameters& SkyParameters);
	void UpdateParameters(const FWeatherParameters& WeatherParameters);

	UFUNCTION(BlueprintCallable, CallInEditor)
	void UpdateSkyParameters(const FSkyParameters& SkyParameters) { UpdateParameters(SkyParameters); }

	UFUNCTION(BlueprintCallable, CallInEditor)
	void UpdateWeatherParameters(const FWeatherParameters& WeatherParameters) { UpdateParameters(WeatherParameters); }



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



private:

	UPROPERTY(Category = "Components")
	TObjectPtr<UDirectionalLightComponent> DirectionalLight;

	UPROPERTY(Category = "Components")
	TObjectPtr<USkyLightComponent> SkyLight;

	UPROPERTY(Category = "Components")
	TObjectPtr<USkyAtmosphereComponent> SkyAtmosphere;

	UPROPERTY(Category = "Components")
	TObjectPtr<UExponentialHeightFogComponent> ExponentialHeightFog;

	UPROPERTY(Category = "Components")
	TObjectPtr<UVolumetricCloudComponent> VolumetricCloud;

	UPROPERTY(Category = "Components")
	TObjectPtr<UPostProcessComponent> PostProcess;

};