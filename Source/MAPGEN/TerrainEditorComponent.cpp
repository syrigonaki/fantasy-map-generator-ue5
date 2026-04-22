#include "TerrainEditorComponent.h"
#include "MapRenderer.h" 
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

UTerrainEditorComponent::UTerrainEditorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    LastPaintPos = FIntPoint(-1, -1);
    bWasClickingLastFrame = false;
    
    BrushRadius = 10.0f;
    SculptSpeed = 0.5f; 
    bIsBrushEnabled = false;
}

void UTerrainEditorComponent::BeginPlay()
{
    Super::BeginPlay();
    PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);

    if (!MapRenderer)
    {
        MapRenderer = Cast<AMapRenderer>(UGameplayStatics::GetActorOfClass(GetWorld(), AMapRenderer::StaticClass()));
    }
}

void UTerrainEditorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) 
{   
   

    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!bIsBrushEnabled || !PC || !MapRenderer) return;

    bMapNeedsRedraw = false; 
    
    // reset the paint box every time
    PaintMinX = 999999;
    PaintMinY = 999999;
    PaintMaxX = -1;
    PaintMaxY = -1;

    

    FHitResult Hit;
    if (PC->GetHitResultUnderCursor(ECC_Visibility, false, Hit)) {
        float ActualCircleRadius = BrushRadius * CursorScale;
        DrawDebugCircle(GetWorld(), Hit.Location + FVector(0,0,5), ActualCircleRadius, 32, FColor::Cyan, false, 0.0f, 0, 1.0f, FVector(1,0,0), FVector(0,1,0), false);
    }

    bool bIsClicking = PC->IsInputKeyDown(EKeys::LeftMouseButton);

    if (bIsClicking && !bWasClickingLastFrame) {
        LastPaintPos = FIntPoint(-1, -1);
    }

    if (bIsClicking) {
        HandlePaint(MapRenderer->WorldData, DeltaTime);
    }

    if (bMapNeedsRedraw) {
        MapRenderer->RedrawMapRegion(PaintMinX, PaintMinY, PaintMaxX, PaintMaxY); 
    }

    if (!bIsClicking && bWasClickingLastFrame) {
        LastPaintPos = FIntPoint(-1, -1);
        MapRenderer->RedrawMap(); 
    }

    bWasClickingLastFrame = bIsClicking;
}

void UTerrainEditorComponent::HandlePaint(FWorldData& WorldData, float DeltaTime) {
    FHitResult Hit;
    if (PC->GetHitResultUnderCursor(ECC_Visibility, false, Hit)) {
        
        FBox Bounds = MapRenderer->GetComponentsBoundingBox(true);
        
        float pctX = FMath::GetMappedRangeValueClamped(FVector2D(Bounds.Min.X, Bounds.Max.X), FVector2D(0.0f, 1.0f), Hit.Location.X);
        float pctY = FMath::GetMappedRangeValueClamped(FVector2D(Bounds.Min.Y, Bounds.Max.Y), FVector2D(0.0f, 1.0f), Hit.Location.Y);

        int32 cx = FMath::FloorToInt(pctY * WorldData.Width);
        int32 cy = FMath::FloorToInt((1.0f - pctX) * WorldData.Height);

        if (cx < 0 || cx >= WorldData.Width || cy < 0 || cy >= WorldData.Height) return;

        if (LastPaintPos.X != -1) {
            float Distance = FVector2D::Distance(FVector2D(LastPaintPos), FVector2D(cx, cy));
            
            if (Distance > (WorldData.Width / 2)) {
                LastPaintPos = FIntPoint(cx, cy);
                return;
            }

           int32 Steps = FMath::CeilToInt(Distance); 
           
            for (int32 i = 0; i <= Steps; i++) {
                float t = (Steps == 0) ? 1.0f : (float)i / Steps;
                int32 LerpX = FMath::FloorToInt(FMath::Lerp((float)LastPaintPos.X, (float)cx, t));
                int32 LerpY = FMath::FloorToInt(FMath::Lerp((float)LastPaintPos.Y, (float)cy, t));
                ApplyOrganicBrush(WorldData, LerpX, LerpY, bIsIncreasing, DeltaTime);
            }

        } else {
            ApplyOrganicBrush(WorldData, cx, cy, bIsIncreasing, DeltaTime); 
        }

        LastPaintPos = FIntPoint(cx, cy);
    }
}

void UTerrainEditorComponent::ApplyOrganicBrush(FWorldData& WorldData, int32 cx, int32 cy, bool bIncreasing, float DeltaTime) {
    int32 r = FMath::CeilToInt(BrushRadius);
    float Strength = SculptSpeed * DeltaTime * 0.2f; 

    for (int32 y = cy - r; y <= cy + r; y++) {
        for (int32 x = cx - r; x <= cx + r; x++) {
            
            if (x > 0 && x < WorldData.Width - 1 && y > 0 && y < WorldData.Height - 1) {
                
                float Dist = FVector2D::Distance(FVector2D(cx, cy), FVector2D(x, y));
                
                if (Dist <= BrushRadius) {
                    float Falloff = FMath::SmoothStep(0.0f, 1.0f, 1.0f - (Dist / BrushRadius));
                    int32 Index = y * WorldData.Width + x;
                    
                    float CurrentH = WorldData.Heights[Index];
                    float Change = Strength * Falloff;
                    float TargetH = bIncreasing ? (CurrentH + Change) : (CurrentH - Change);
                    
                    WorldData.Heights[Index] = FMath::Clamp(TargetH, 0.0f, 1.0f); 

                    PaintMinX = FMath::Min(PaintMinX, x);
                    PaintMinY = FMath::Min(PaintMinY, y);
                    PaintMaxX = FMath::Max(PaintMaxX, x);
                    PaintMaxY = FMath::Max(PaintMaxY, y);
                    bMapNeedsRedraw = true;
                }
            }
        }
    }
}