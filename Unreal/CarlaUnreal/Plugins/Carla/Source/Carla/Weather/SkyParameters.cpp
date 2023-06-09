#include "Carla/Weather/SkyParameters.h"



FSkyParameters USkyParametersBlueprintFunctionLibrary::WeatherParametersToSkyParameters(
    const FWeatherParameters& WeatherParameters,
	const FSkyParameters& Default)
{
	auto Out = Default;
	Out.DirectionalLight.SunAltitude = WeatherParameters.SunAzimuthAngle;
	Out.DirectionalLight.SunAzimuth = WeatherParameters.SunAltitudeAngle;
	Out.DirectionalLight.ScatteringIntensity = WeatherParameters.ScatteringIntensity;
	// Out.DirectionalLight.EnableLightShaftOcclusion = WeatherParameters;
	// Out.DirectionalLight.EnableLightShaftBloom = WeatherParameters;
	// Out.DirectionalLight.CascadeShadowDistance = WeatherParameters;
	// Out.DirectionalLight.ContactShadowLength = WeatherParameters;
	// Out.SkyAtmosphere.AirParticlesDensity = WeatherParameters;
	// Out.SkyAtmosphere.PollutionParticlesDensity = WeatherParameters;
	// Out.SkyAtmosphere.HeightFogContribution = WeatherParameters;
	// Out.ExponentialHeightFog.Color = WeatherParameters;
	Out.ExponentialHeightFog.Falloff = WeatherParameters.FogFalloff;
	Out.ExponentialHeightFog.Density = WeatherParameters.FogDensity;
	Out.ExponentialHeightFog.FogDistance = WeatherParameters.FogDistance;
	// Out.ExponentialHeightFog.VolumetricEnable = WeatherParameters;
    return Out;
}
