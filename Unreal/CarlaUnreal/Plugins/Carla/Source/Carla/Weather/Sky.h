#pragma once
#include "CoreMinimal.h"
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
struct FSkyParameters;
struct FWeatherParameters;



UCLASS(BlueprintType)
class CARLA_API ASky :
	public AActor
{
	GENERATED_BODY()

	void Setup(const FSkyParameters& SkyParameters);

public:

	ASky(const FObjectInitializer& ObjectInitializer);
	ASky(const FObjectInitializer& ObjectInitializer, const FSkyParameters& SkyParameters);

	
	
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
	const FSkyParameters& GetActiveParameters() const { return Parameters; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
	static FSkyParameters DeriveSkyParametersFromWeatherParameters(const FWeatherParameters& WeatherParameters);



	UFUNCTION(BlueprintCallable, CallInEditor)
	void SetSkyParameters(const FSkyParameters& SkyParameters);

	UFUNCTION(BlueprintCallable, CallInEditor)
	void SetWeatherParameters(const FWeatherParameters& WeatherParameters);

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



	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Components")
	FSkyParameters Parameters;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Components")
	TObjectPtr<UDirectionalLightComponent> DirectionalLight;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Components")
	TObjectPtr<USkyLightComponent> SkyLight;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Components")
	TObjectPtr<USkyAtmosphereComponent> SkyAtmosphere;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Components")
	TObjectPtr<UExponentialHeightFogComponent> ExponentialHeightFog;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Components")
	TObjectPtr<UVolumetricCloudComponent> VolumetricCloud;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Components")
	TObjectPtr<UPostProcessComponent> PostProcess;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Components")
	TArray<FWeatherReflectionRange> WeatherReflectionRanges;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Components")
	TMap<FString, TObjectPtr<ULevelStreamingDynamic>> ReflectionSublevelMap;

	FString CurrentLoadedLevel;

	FString DefaultReflectionLevel;

};