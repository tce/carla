#include "Sky.h"
#include "SkyParameters.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/VolumetricCloudComponent.h"
#include "Components/PostProcessComponent.h"
#include "Components/ExponentialHeightFogComponent.h"

ASky::ASky(const FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer)
{
	DirectionalLight = CreateDefaultSubobject<UDirectionalLightComponent>(TEXT("ASky::DirectionalLight"));
	SkyLight = CreateDefaultSubobject<USkyLightComponent>(TEXT("ASky::SkyLight"));
	SkyAtmosphere = CreateDefaultSubobject<USkyAtmosphereComponent>(TEXT("ASky::SkyAtmosphere"));
	VolumetricCloud = CreateDefaultSubobject<UVolumetricCloudComponent>(TEXT("ASky::VolumetricCloud"));
	PostProcess = CreateDefaultSubobject<UPostProcessComponent>(TEXT("ASky::PostProcess"));
	ExponentialHeightFog = CreateDefaultSubobject<UExponentialHeightFogComponent>(TEXT("ASky::ExponentialHeightFog"));
}

void ASky::UpdateParameters(const FSkyParameters& SkyParameters)
{
}

void ASky::UpdateParameters(const FWeatherParameters& WeatherParameters)
{
}

void ASky::UpdateDirectionalLight()
{
}

void ASky::UpdateSkyLight()
{
}

void ASky::UpdateSkyAtmosphere()
{
}

void ASky::UpdateExponentialHeightFog()
{
}

void ASky::UpdateVolumetricCloud()
{
}

void ASky::UpdatePostProcess()
{
}