#pragma once

#include "WorldDataStruct.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MapRenderer.generated.h"

UENUM(BlueprintType)
enum class EViewMode : uint8 {
    Physical UMETA(DisplayName = "Physical"),
    Political UMETA(DisplayName = "Political"),
    TerrainEdit UMETA(DisplayName = "Terrain Edit")
};

UCLASS()
class MAPGEN_API AMapRenderer : public AActor {
    GENERATED_BODY()
    
public:	
    AMapRenderer();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Map Renderer")
    class UStaticMeshComponent* MapPlane;

    UPROPERTY()
    class UTexture2D* MapTexture;

    UPROPERTY()
    class UMaterialInstanceDynamic* DynamicMaterial;

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map Settings")
    EViewMode CurrentView = EViewMode::Physical;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map Features")
    bool bShowBorders = true;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Map Data")
    FWorldData WorldData;

    UPROPERTY(BlueprintReadOnly, Category = "Terrain Stats")
    float LandmassPercentage = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Terrain Stats")
    float MountainPercentage = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Terrain Stats")
    float AverageElevation = 0.0f;



    UFUNCTION(BlueprintCallable, Category = "Generation")
    void GenerateEntireWorld();

    UFUNCTION(BlueprintCallable, Category = "Map Renderer")
    void RedrawMap(); 

    UFUNCTION(BlueprintCallable, Category = "Map Renderer")
    void RedrawMapRegion(int32 MinX, int32 MinY, int32 MaxX, int32 MaxY);

    UFUNCTION(BlueprintCallable, Category = "Map Renderer")
    void SetSeaLevel(float newSeaLevel);


    UFUNCTION(BlueprintCallable, Category = "World Data")
    int32 GetKingdomCount();

    UFUNCTION(BlueprintCallable, Category = "Map Renderer")
    void UpdateHeightStats();

private:
    
    FLinearColor GetBiomeColor(float h, float m, float seaLevel);
    FLinearColor CalculateTerrainEditColor(float h, float seaLevel);
    FLinearColor GetKingdomColor(int32 KingdomID);
};