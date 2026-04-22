#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "WorldDataStruct.h"
#include "TerrainEditorComponent.generated.h"

class AMapRenderer; 

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MAPGEN_API UTerrainEditorComponent : public UActorComponent {
    GENERATED_BODY()

    public: 
        UTerrainEditorComponent();
        
        UPROPERTY(VisibleAnywhere)
        class UDecalComponent* CursorDecal;

    protected:
        virtual void BeginPlay() override;
        virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    public:
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Brush Settings")
        bool bIsBrushEnabled = false; 

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Brush Settings")
        float BrushRadius = 15.0f; 
        
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Brush Settings", meta=(ClampMin="0.001", ClampMax="1"))
        float SculptSpeed = 0.5f; 

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Brush Settings")
        bool bIsIncreasing = true;

        UPROPERTY(EditAnywhere, Category = "Brush Settings")
        bool bFlipHorizontal = true;

        UPROPERTY(EditAnywhere, Category = "Brush Settings")
        bool bFlipVertical = true;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Brush Settings")
        bool bSwapAxes = true;

        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Brush Settings")
        float CursorScale = 2.0f;
  
        UPROPERTY()
        AMapRenderer* MapRenderer;

    private:
        void HandlePaint(FWorldData& WorldData, float DeltaTime);
        void ApplyOrganicBrush(FWorldData& WorldData, int32 cx, int32 cy, bool bIncreasing, float DeltaTime);

        FIntPoint LastPaintPos;
        bool bMapNeedsRedraw;
        bool bWasClickingLastFrame;

        APlayerController* PC;

        int32 PaintMinX;
        int32 PaintMinY;
        int32 PaintMaxX;
        int32 PaintMaxY;
};