
#pragma once

#include "CoreMinimal.h"
#include "WorldDataStruct.generated.h"

USTRUCT(BlueprintType)
struct FWorldData {
    GENERATED_BODY()

    UPROPERTY(EditAnywhere)
    int32 Width = 1920;

    UPROPERTY(EditAnywhere)
    int32 Height = 1080;

    UPROPERTY(EditAnywhere)
    int32 KingdomCount = 5;

    UPROPERTY(EditAnywhere)
    float SeaLevel = 0.45f;

    TArray<float> Heights;
    TArray<float> Moisture;
    TArray<int32> KingdomMap;
};
