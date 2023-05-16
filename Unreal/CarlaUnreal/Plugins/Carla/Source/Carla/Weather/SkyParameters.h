#pragma once
#include "CoreMinimal.h"
#include "SkyParameters.generated.h"



USTRUCT(BlueprintType)
struct CARLA_API FSkyParametersMiscellaneous
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Altitude = 1.0F;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Azimuth = 1.0F;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Clouds = 1.0F;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Fog = 1.0F;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Falloff = 1.0F;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FogDistance = 1.0F;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor DenseFogColor = FLinearColor(0.4F, 0.4F, 0.4F, 1.0F);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FPostProcessSettings PostProcessSettings;

	FSkyParametersMiscellaneous();
	FSkyParametersMiscellaneous(const FSkyParametersMiscellaneous&) = default;
	FSkyParametersMiscellaneous& operator=(const FSkyParametersMiscellaneous&) = default;
	~FSkyParametersMiscellaneous() = default;
};



USTRUCT(BlueprintType)
struct CARLA_API FSkyParametersDirectionalLight
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor Color = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Intensity = 10.0F;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Temperature = 6500.0F;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ScatteringIntensity = 1.0F;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool EnableLightShaftOcclusion = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool EnableLightShaftBloom = true;
};



USTRUCT(BlueprintType)
struct CARLA_API FSkyParametersSkyLight
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor Color = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Intensity = 1.0F;
};



USTRUCT(BlueprintType)
struct CARLA_API FSkyParametersSkyAtmosphere
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AirParticlesDensity = 0.0331F;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PollutionParticlesDensity = 0.03F;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float HeightFogContribution = 1.0F;
};



USTRUCT(BlueprintType)
struct CARLA_API FSkyParametersExponentialHeightFog
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor Color = FLinearColor(0.296875F, 0.296875F, 0.296875F, 1.0F);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Falloff = 0.2F;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Density = 0.01F;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool VolumetricEnable = true;
};



USTRUCT(BlueprintType)
struct CARLA_API FSkyParametersVolumetricCloud
{
	GENERATED_BODY()
};


USTRUCT(BlueprintType)
struct CARLA_API FSkyParametersShadows
{
	GENERATED_BODY()
		
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FColor DFAOTint = FColor();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DFAOContrast = 1.0F;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DFAOExponent = 0.3F;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DFAOMinOcclusion = 0.0F;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CascadeShadowDistance = 10000.0F;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ContactShadowLength = 0.1F;
};


USTRUCT(BlueprintType)
struct CARLA_API FSkyParameters
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FSkyParametersMiscellaneous Miscellaneous;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FSkyParametersDirectionalLight DirectionalLight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FSkyParametersSkyLight SkyLight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FSkyParametersSkyAtmosphere SkyAtmosphere;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FSkyParametersExponentialHeightFog ExponentialHeightFog;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FSkyParametersVolumetricCloud VolumetricCloud;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FSkyParametersShadows Shadows;
};