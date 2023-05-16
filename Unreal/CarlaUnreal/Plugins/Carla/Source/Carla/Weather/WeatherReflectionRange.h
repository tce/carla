#pragma once
#include "CoreMinimal.h"
#include "WeatherReflectionRange.generated.h"



USTRUCT(BlueprintType)
struct CARLA_API FWeatherReflectionRange
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString SublevelName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FFloatRange SubAltitudeAngle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FFloatRange FogRange;
};