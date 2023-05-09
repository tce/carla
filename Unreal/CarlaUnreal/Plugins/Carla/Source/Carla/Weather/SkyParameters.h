#pragma once
#include "CoreMinimal.h"
#include "SkyParameters.generated.h"



USTRUCT(BlueprintType)
struct CARLA_API FMiscellaneousParameters
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Altitude;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Azimuth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Clouds;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Fog;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Falloff;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FogDistance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DenseFogColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FPostProcessSettings PostProcessSettings;

	FMiscellaneousParameters();
	FMiscellaneousParameters(const FMiscellaneousParameters&) = default;
	FMiscellaneousParameters& operator=(const FMiscellaneousParameters&) = default;
	~FMiscellaneousParameters() = default;
};



USTRUCT(BlueprintType)
struct CARLA_API FDirectionalLightParameters
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor Color;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Intensity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Temperature;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool EnableLightShaftOcclusion;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool EnableLightShaftBloom;
};



USTRUCT(BlueprintType)
struct CARLA_API FSkyLightParameters
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor Color;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Intensity;
};



USTRUCT(BlueprintType)
struct CARLA_API FSkyAtmosphereParameters
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AirParticlesDensity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PollutionParticlesDensity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ScatteringIntensity;
};



USTRUCT(BlueprintType)
struct CARLA_API FExponentialHeightFogParameters
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor Color;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Contribution;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Density;
};



USTRUCT(BlueprintType)
struct CARLA_API FVolumetricCloudParameters
{
	GENERATED_BODY()
};



USTRUCT(BlueprintType)
struct CARLA_API FSkyParameters
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMiscellaneousParameters Miscellaneous;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDirectionalLightParameters DirectionalLight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FSkyLightParameters SkyLight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FSkyAtmosphereParameters SkyAtmosphere;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FExponentialHeightFogParameters ExponentialHeightFog;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVolumetricCloudParameters VolumetricCloud;
};