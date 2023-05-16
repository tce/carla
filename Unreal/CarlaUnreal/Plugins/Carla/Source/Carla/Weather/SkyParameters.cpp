#include "SkyParameters.h"

FSkyParametersMiscellaneous::FSkyParametersMiscellaneous() :
	Altitude(),
	Azimuth(),
	Clouds(),
	Fog(),
	Falloff(),
	FogDistance(),
	DenseFogColor(),
	PostProcessSettings()
{
	PostProcessSettings.bOverride_ReflectionMethod = true;
	PostProcessSettings.bOverride_DynamicGlobalIlluminationMethod = true;
	PostProcessSettings.bOverride_LumenReflectionQuality = true;
	PostProcessSettings.bOverride_LumenSceneLightingQuality = true;
	PostProcessSettings.bOverride_LumenSceneDetail = true;
	PostProcessSettings.bOverride_LumenSceneViewDistance = true;
	PostProcessSettings.bOverride_LumenSceneLightingUpdateSpeed = true;
	PostProcessSettings.bOverride_LumenFinalGatherQuality = true;
	PostProcessSettings.bOverride_LumenFinalGatherLightingUpdateSpeed = true;
	PostProcessSettings.bOverride_LumenMaxTraceDistance = true;
	PostProcessSettings.bOverride_LumenDiffuseColorBoost = true;
	PostProcessSettings.bOverride_LumenSkylightLeaking = true;
	PostProcessSettings.bOverride_LumenFullSkylightLeakingDistance = true;
	PostProcessSettings.bOverride_LumenRayLightingMode = true;
	PostProcessSettings.bOverride_LumenFrontLayerTranslucencyReflections = true;
	PostProcessSettings.bOverride_LumenSurfaceCacheResolution = true;

	PostProcessSettings.ReflectionMethod = EReflectionMethod::Lumen;
	PostProcessSettings.DynamicGlobalIlluminationMethod = EDynamicGlobalIlluminationMethod::Lumen;
	PostProcessSettings.LumenDiffuseColorBoost = 1.0F;
	PostProcessSettings.LumenSceneLightingQuality = 1.0F;
	PostProcessSettings.LumenSceneDetail = 1.0F;
	PostProcessSettings.LumenSceneViewDistance = 50000.0F;
	PostProcessSettings.LumenSceneLightingUpdateSpeed = 1.0F;
	PostProcessSettings.LumenFinalGatherQuality = 1.0F;
	PostProcessSettings.LumenFinalGatherLightingUpdateSpeed = 1.0F;
	PostProcessSettings.LumenMaxTraceDistance = 5000.0F;
	PostProcessSettings.LumenDiffuseColorBoost = 1.0F;
	PostProcessSettings.LumenSkylightLeaking = 0.0F;
	PostProcessSettings.LumenFullSkylightLeakingDistance = 0.0F;
	PostProcessSettings.LumenSurfaceCacheResolution = 1.0F;
	PostProcessSettings.LumenReflectionQuality = 1.0F;
	PostProcessSettings.LumenRayLightingMode = ELumenRayLightingModeOverride::HitLighting;
	PostProcessSettings.LumenFrontLayerTranslucencyReflections = true;
}
