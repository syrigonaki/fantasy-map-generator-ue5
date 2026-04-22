
#include "TerrainManager.h"


ATerrainManager::ATerrainManager() {
    PrimaryActorTick.bCanEverTick = false; 
    TerrainEditorComp = CreateDefaultSubobject<UTerrainEditorComponent>(TEXT("TerrainEditorComponent"));
}

void ATerrainManager::BeginPlay() {
    Super::BeginPlay();
    if (TerrainEditorComp) {
        UE_LOG(LogTemp, Warning, TEXT("Terrain Editor Component is alive!"));
    }
    
}

void ATerrainManager::GenerateTerrain(FWorldData& WorldData) {

    float baseOffset = FMath::RandRange(-50000.0f, 50000.0f);
    float mountainOffset = FMath::RandRange(-50000.0f, 50000.0f) + 12345.0f; 
    float islandOffset = FMath::RandRange(-50000.0f, 50000.0f) + 25000.0f;

    float globalContinentScale = 400.0f; 

    int32 MapHeight = WorldData.Height;
    int32 MapWidth = WorldData.Width;
    float GlobalSeaLevel = WorldData.SeaLevel;
    
    WorldData.Heights.Init(0.0f, MapWidth * MapHeight);

    for (int32 y = 0; y < MapHeight; y++) {
        for (int32 x = 0; x < MapWidth; x++) {

            float baseShape = GetOctaveNoise(x + baseOffset, y + baseOffset, globalContinentScale);
            float edgeDetail = GetOctaveNoise(x + baseOffset + 5000.0f, y + baseOffset + 5000.0f, 80.0f);
            float h = (baseShape * 0.85f) + (edgeDetail * 0.15f);

            float falloff = CalculateFalloff(x, y, MapWidth, MapHeight);
            h -= falloff;

            float mountainMask = GetOctaveNoise(x + mountainOffset, y + mountainOffset, 250.0f);
            float mThreshold = 0.45f;

            if (h > GlobalSeaLevel && mountainMask > mThreshold) {
                float sharpPeaks = GetRidgedNoise(x + mountainOffset, y + mountainOffset, 120.0f);
                
                float rawWeight = (mountainMask - mThreshold) / (1.0f - mThreshold); 
                
                float mountainWeight = FMath::SmoothStep(0.0f, 1.0f, rawWeight);
                
                h += (mountainWeight * 0.15f); 
                
                h += (sharpPeaks * mountainWeight * 0.9f); 
            }
          
            if (h < GlobalSeaLevel) {
                float islandMask = GetOctaveNoise(x + islandOffset, y + islandOffset, 300.0f);
                float iThreshold = 0.70f;

                if (islandMask > iThreshold) {
                    float islandDetail = GetRidgedNoise(x + islandOffset, y + islandOffset, 40.0f);
                    float islandWeight = (islandMask - iThreshold) / (1.0f - iThreshold);
                    float islandHeightBoost = (islandDetail * 0.2f) * islandWeight;
                    h += islandHeightBoost; 
                }
            }

            WorldData.Heights[y * MapWidth + x] = FMath::Clamp(h, 0.0f, 1.0f);
        }
    }
}

float ATerrainManager::CalculateFalloff(int32 x, int32 y, int32 width, int32 height) {
    float valueX = (x / (float)width) * 2.0f - 1.0f;
    float valueY = (y / (float)height) * 2.0f - 1.0f;

    float distance = FMath::Max(FMath::Abs(valueX), FMath::Abs(valueY));
    return FMath::Pow(distance, 6.0f); 
}

float ATerrainManager::GetOctaveNoise(float x, float y, float baseScale) {
    float amplitude = 1.0f;
    float frequency = 1.0f;
    float noiseHeight = 0.0f;
    float maxPossibleHeight = 0.0f;

    for (int32 i = 0; i < Octaves; i++) {
        float sampleX = (x / baseScale) * frequency;
        float sampleY = (y / baseScale) * frequency;
        
        float rawNoise = FMath::PerlinNoise2D(FVector2D(sampleX, sampleY));
        float unityStyleNoise = (rawNoise + 1.0f) / 2.0f;
        
        noiseHeight += unityStyleNoise * amplitude;
        maxPossibleHeight += amplitude;
        
        amplitude *= Persistence;
        frequency *= Lacunarity;
    }

    return noiseHeight / maxPossibleHeight;
}

float ATerrainManager::GetRidgedNoise(float x, float y, float baseScale) {
    float amplitude = 1.0f;
    float frequency = 1.0f;
    float noiseHeight = 0.0f;
    float maxPossibleHeight = 0.0f;

    for (int32 i = 0; i < Octaves; i++) {
        float sampleX = (x / baseScale) * frequency;
        float sampleY = (y / baseScale) * frequency;
 
        float rawNoise = FMath::PerlinNoise2D(FVector2D(sampleX, sampleY));
        float unityStyleNoise = (rawNoise + 1.0f) / 2.0f;

        float ridge = 1.0f - FMath::Abs(unityStyleNoise * 2.0f - 1.0f); 
        ridge *= ridge; 
        
        noiseHeight += ridge * amplitude;
        maxPossibleHeight += amplitude;
        
        amplitude *= Persistence;
        frequency *= Lacunarity;
    }

    return noiseHeight / maxPossibleHeight;
}