#include "Carla/Weather/SkyParameters.h"
#include "Carla/Weather/WeatherParameters.h"

FSkyParameters::FSkyParameters() :
	DirectionalLight(),
	SkyLight(),
	SkyAtmosphere(),
	ExponentialHeightFog(),
	VolumetricCloud(),
	PostProcessSettings()
{
	PostProcessSettings.bOverride_LumenReflectionQuality = true;
	PostProcessSettings.bOverride_LumenSceneLightingQuality = true;
	PostProcessSettings.bOverride_LumenSceneDetail = true;
	PostProcessSettings.bOverride_LumenSceneViewDistance = true;
	PostProcessSettings.bOverride_LumenSceneLightingUpdateSpeed = true;
	PostProcessSettings.bOverride_LumenFinalGatherQuality = true;
	PostProcessSettings.bOverride_LumenFinalGatherLightingUpdateSpeed = true;
	PostProcessSettings.bOverride_LumenMaxTraceDistance = true;
	PostProcessSettings.bOverride_LumenDiffuseColorBoost = false;
	PostProcessSettings.bOverride_LumenSkylightLeaking = false;
	PostProcessSettings.bOverride_LumenFullSkylightLeakingDistance = false;
	PostProcessSettings.bOverride_LumenRayLightingMode = true;
	PostProcessSettings.bOverride_LumenFrontLayerTranslucencyReflections = true;
	PostProcessSettings.bOverride_LumenSurfaceCacheResolution = true;
	PostProcessSettings.LumenSceneLightingQuality = 2.0F;
	PostProcessSettings.LumenSceneDetail = 4.0F;
	PostProcessSettings.LumenSceneViewDistance = 20000;
	PostProcessSettings.LumenSceneLightingUpdateSpeed = 1.0F;
	PostProcessSettings.LumenFinalGatherQuality = 1.0F;
	PostProcessSettings.LumenFinalGatherLightingUpdateSpeed = 1.0F;
	PostProcessSettings.LumenMaxTraceDistance = 20000;
	PostProcessSettings.LumenSurfaceCacheResolution = 1.0F;
	PostProcessSettings.LumenReflectionQuality = 1.0F;
	PostProcessSettings.LumenRayLightingMode = ELumenRayLightingModeOverride::SurfaceCache;
	PostProcessSettings.LumenFrontLayerTranslucencyReflections = 1.0F;
}

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
