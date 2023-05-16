#include "Sky.h"
#include "WeatherParameters.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/VolumetricCloudComponent.h"
#include "Components/PostProcessComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/LevelStreamingDynamic.h"

void ASky::Setup(const FSkyParameters& SkyParameters)
{
	DirectionalLight = CreateDefaultSubobject<UDirectionalLightComponent>(TEXT("ASky::DirectionalLight"));
	SkyLight = CreateDefaultSubobject<USkyLightComponent>(TEXT("ASky::SkyLight"));
	SkyAtmosphere = CreateDefaultSubobject<USkyAtmosphereComponent>(TEXT("ASky::SkyAtmosphere"));
	VolumetricCloud = CreateDefaultSubobject<UVolumetricCloudComponent>(TEXT("ASky::VolumetricCloud"));
	PostProcess = CreateDefaultSubobject<UPostProcessComponent>(TEXT("ASky::PostProcess"));
	ExponentialHeightFog = CreateDefaultSubobject<UExponentialHeightFogComponent>(TEXT("ASky::ExponentialHeightFog"));
	UpdateChildComponents();
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

FSkyParameters ASky::DeriveSkyParametersFromWeatherParameters(
	const FWeatherParameters& WeatherParameters)
{
	FSkyParameters Result = {};
	return Result;
}

void ASky::SetSkyParameters(const FSkyParameters& SkyParameters)
{
	this->Parameters = Parameters;
	UpdateChildComponents();
}

void ASky::SetWeatherParameters(const FWeatherParameters& WeatherParameters)
{
	SetSkyParameters(DeriveSkyParametersFromWeatherParameters(WeatherParameters));
}

void ASky::UpdateDirectionalLight()
{
	const auto& Params = Parameters.DirectionalLight;
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
	DirectionalLight->Activate();
}

void ASky::UpdateSkyLight()
{
	const auto& Params = Parameters.SkyLight;
	SkyLight->Deactivate();
	SkyLight->SetIntensity(Params.Intensity);
	SkyLight->SetLightColor(Params.Color);
	SkyLight->SetOcclusionContrast(Parameters.Shadows.DFAOContrast);
	SkyLight->SetOcclusionExponent(Parameters.Shadows.DFAOExponent);
	SkyLight->SetMinOcclusion(Parameters.Shadows.DFAOMinOcclusion);
	SkyLight->SetOcclusionTint(Parameters.Shadows.DFAOTint);
	SkyLight->Activate();
}

void ASky::UpdateSkyAtmosphere()
{
	const auto& Params = Parameters.SkyAtmosphere;
	SkyAtmosphere->Deactivate();
	SkyAtmosphere->SetRayleighScatteringScale(Params.AirParticlesDensity);
	SkyAtmosphere->SetMieScatteringScale(Params.PollutionParticlesDensity);
	SkyAtmosphere->SetHeightFogContribution(Params.HeightFogContribution);
	SkyAtmosphere->Activate();
}

void ASky::UpdateExponentialHeightFog()
{
	const auto& Params = Parameters.ExponentialHeightFog;
	ExponentialHeightFog->Deactivate();
	ExponentialHeightFog->SetFogInscatteringColor(Params.Color);
	ExponentialHeightFog->SetFogDensity(Params.Density);
	ExponentialHeightFog->SetFogHeightFalloff(Params.Falloff);
	ExponentialHeightFog->SetVolumetricFog(Params.VolumetricEnable);
	ExponentialHeightFog->Activate();
}

void ASky::UpdateVolumetricCloud()
{
	const auto& Params = Parameters.VolumetricCloud;
	VolumetricCloud->Deactivate();
	VolumetricCloud->Activate();
}

void ASky::UpdatePostProcess()
{
	const auto& Params = Parameters.Miscellaneous.PostProcessSettings;
	PostProcess->Deactivate();
	PostProcess->Settings = Params;
	PostProcess->Activate();
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
