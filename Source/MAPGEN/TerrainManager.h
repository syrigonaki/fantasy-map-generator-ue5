#pragma once


#include "WorldDataStruct.h"
#include "TerrainEditorComponent.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TerrainManager.generated.h"


UCLASS()
class MAPGEN_API ATerrainManager : public AActor {
        GENERATED_BODY()
        
    public:	
        ATerrainManager();

    protected:
        virtual void BeginPlay() override;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Global Noise Settings")
        int32 Octaves = 6;

        UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Terrain")
        class UTerrainEditorComponent* TerrainEditorComp;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Global Noise Settings", meta = (ClampMin = "0.0", ClampMax = "1.0"))
        float Persistence = 0.5f;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Global Noise Settings")
        float Lacunarity = 2.0f;

      

    public:
        UFUNCTION(BlueprintCallable, Category = "Terrain Generation")
        void GenerateTerrain(UPARAM(ref) FWorldData& DataToModify);

    private:
        float CalculateFalloff(int32 x, int32 y, int32 width, int32 height);
        float GetOctaveNoise(float x, float y, float baseScale);
        float GetRidgedNoise(float x, float y, float baseScale);
};


