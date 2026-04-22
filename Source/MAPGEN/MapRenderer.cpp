#include "MapRenderer.h"
#include "Kismet/GameplayStatics.h"
#include "TerrainManager.h" 
#include "KingdomManagerComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Texture2D.h"
#include "Materials/MaterialInstanceDynamic.h"

AMapRenderer::AMapRenderer() {
    PrimaryActorTick.bCanEverTick = false;
    MapPlane = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MapPlane")); //physical plane
    RootComponent = MapPlane;
}

void AMapRenderer::BeginPlay() {
    Super::BeginPlay();
    
    if (MapPlane->GetMaterial(0)) {
        DynamicMaterial = MapPlane->CreateAndSetMaterialInstanceDynamic(0);
    }
}


int32 AMapRenderer::GetKingdomCount() {
    return WorldData.KingdomCount;
}

void AMapRenderer::GenerateEntireWorld() {
    
    WorldData.Moisture.Init(0.5f, WorldData.Width * WorldData.Height);
    WorldData.Heights.Init(0.0f, WorldData.Width * WorldData.Height); 
    
    ATerrainManager* TerrainMgr = Cast<ATerrainManager>(UGameplayStatics::GetActorOfClass(GetWorld(), ATerrainManager::StaticClass()));
    if (TerrainMgr) TerrainMgr->GenerateTerrain(WorldData); 
    

    UKingdomManagerComponent* KingdomMgr = FindComponentByClass<UKingdomManagerComponent>();

    if (KingdomMgr) KingdomMgr->GenerateRandomKingdoms(WorldData, WorldData.KingdomCount);
    
    RedrawMap(); 
    UpdateHeightStats();
}

void AMapRenderer::RedrawMap() {
    if (WorldData.Heights.Num() == 0 || WorldData.Width == 0 || WorldData.Height == 0) return;

    if (!MapTexture || MapTexture->GetSizeX() != WorldData.Width || MapTexture->GetSizeY() != WorldData.Height) {

        MapTexture = UTexture2D::CreateTransient(WorldData.Width, WorldData.Height, PF_B8G8R8A8);
        MapTexture->Filter = TF_Bilinear; 
        
        if (DynamicMaterial) DynamicMaterial->SetTextureParameterValue(FName("MainTexture"), MapTexture);

    }


    TArray<FColor> Pixels;
    Pixels.Init(FColor::Black, WorldData.Width * WorldData.Height);

    for (int32 y = 0; y < WorldData.Height; y++) {
        for (int32 x = 0; x < WorldData.Width; x++) {
            int32 Index = y * WorldData.Width + x;
            float h = WorldData.Heights[Index];
            
            float m = (WorldData.Moisture.Num() > Index) ? WorldData.Moisture[Index] : 0.5f;
            
            FLinearColor FinalColor = FLinearColor::Black;

            if (CurrentView == EViewMode::TerrainEdit) {
                FinalColor = CalculateTerrainEditColor(h, WorldData.SeaLevel);
            } else {
                if (h < WorldData.SeaLevel) {
                    // WATER RENDER
                    float depth = h / WorldData.SeaLevel;
                    float tightGlow = FMath::Pow(depth, 10.0f); 

                    FLinearColor OceanColor = FLinearColor::FromSRGBColor(FColor(20, 30, 50)); 
                    FLinearColor CoastColor = FLinearColor::FromSRGBColor(FColor(45, 80, 110)); 

                    FinalColor = FMath::Lerp(OceanColor, CoastColor, tightGlow);

                } else {
                    // LAND RENDER
                    if (CurrentView == EViewMode::Physical) {
                        FinalColor = GetBiomeColor(h, m, WorldData.SeaLevel);
                    } else if (CurrentView == EViewMode::Political) {
                        
                        int32 kID = WorldData.KingdomMap[Index];
                        FLinearColor KColor = GetKingdomColor(kID);
                        FLinearColor BiomeBase = GetBiomeColor(h, m, WorldData.SeaLevel);
                        
                        float Glow = (BiomeBase.R * 0.3f) + (BiomeBase.G * 0.59f) + (BiomeBase.B * 0.11f);
                        FLinearColor PaperTerrain = FLinearColor(Glow, Glow, Glow);
                        
                        FinalColor = FMath::Lerp(PaperTerrain, KColor, 0.85f);
                    }
                    
                    // SHADOWS 
                    if (x > 0 && y < WorldData.Height - 1) {
                        float hTL = WorldData.Heights[(y + 1) * WorldData.Width + (x - 1)];
                        float diff = h - hTL;
                        
                        FLinearColor ShadowColor = FLinearColor::FromSRGBColor(FColor(40, 30, 20));
                        
                        if (diff < 0) {
                            float ShadowAlpha = FMath::Clamp(FMath::Abs(diff) * 20.0f, 0.0f, 1.0f);
                            FinalColor = FMath::Lerp(FinalColor, ShadowColor, ShadowAlpha);
                        } else {
                            float HighlightAlpha = FMath::Clamp(diff * 15.0f, 0.0f, 1.0f);
                            FinalColor = FMath::Lerp(FinalColor, FLinearColor::White, HighlightAlpha);
                        }
                    }
     
                }
            }
            
            if (bShowBorders && h >= WorldData.SeaLevel) {
                if (x < WorldData.Width - 1 && y < WorldData.Height - 1) {
                    int32 rightID = WorldData.KingdomMap[Index + 1];
                    int32 downID = WorldData.KingdomMap[Index + WorldData.Width];
                    int32 currentID = WorldData.KingdomMap[Index];

                    if (currentID != rightID || currentID != downID) {
                      
                        FLinearColor BorderGray = FLinearColor(0.1f, 0.1f, 0.1f, 1.0f);
                        FinalColor = FMath::Lerp(FinalColor, BorderGray, 0.6f); 

                    }
                }
            }

           
            float CustomGamma = 1.9f; 

            FinalColor.R = FMath::Pow(FinalColor.R, 1.0f / CustomGamma);
            FinalColor.G = FMath::Pow(FinalColor.G, 1.0f / CustomGamma);
            FinalColor.B = FMath::Pow(FinalColor.B, 1.0f / CustomGamma);

            // w true unreal applies more brightness
            Pixels[Index] = FinalColor.ToFColor(false);

        }
    }

    // lock texture memory while writing
    FTexture2DMipMap& Mip = MapTexture->GetPlatformData()->Mips[0];
    void* TextureData = Mip.BulkData.Lock(LOCK_READ_WRITE);

    if (TextureData) {
        FMemory::Memcpy(TextureData, Pixels.GetData(), Pixels.Num() * sizeof(FColor));
        Mip.BulkData.Unlock();
    } 

    MapTexture->UpdateResource();
}

FLinearColor AMapRenderer::GetBiomeColor(float h, float m, float seaLevel) {

  
    float landRange = 1.0f - seaLevel;
    if (landRange <= 0.001f) landRange = 0.001f; //avoid nan

    //  calculate ratio (always have neatly distributed colours even when user raises sea level)
    float landHeight = FMath::Clamp((h - seaLevel) / landRange, 0.0f, 1.0f);

    FLinearColor sand = FLinearColor::FromSRGBColor(FColor(235, 215, 160));
    FLinearColor lushGrass = FLinearColor::FromSRGBColor(FColor(95, 135, 75));
    FLinearColor darkForest = FLinearColor::FromSRGBColor(FColor(35, 75, 35));
    FLinearColor rock = FLinearColor::FromSRGBColor(FColor(80, 75, 70));
    FLinearColor snow = FLinearColor::White;

    FLinearColor finalColor = sand;

    if (landHeight < 0.15f) finalColor = FMath::Lerp(sand, lushGrass, landHeight / 0.15f);
    else if (landHeight < 0.50f) finalColor = FMath::Lerp(lushGrass, darkForest, (landHeight - 0.15f) / 0.35f);
    else if (landHeight < 0.82f) finalColor = FMath::Lerp(darkForest, rock, (landHeight - 0.50f) / 0.32f);
    else finalColor = FMath::Lerp(rock, snow, (landHeight - 0.82f) / 0.18f);

    if (landHeight < 0.6f) 
    { 
        if (m < 0.35f) {
            FLinearColor desertColor = FLinearColor::FromSRGBColor(FColor(230, 190, 120));
            finalColor = FMath::Lerp(finalColor, desertColor, ((0.35f - m) / 0.35f) * 0.7f);
        } 
        else if (m > 0.75f) {
            FLinearColor jungleColor = FLinearColor::FromSRGBColor(FColor(30, 80, 30));
            finalColor = FMath::Lerp(finalColor, jungleColor, ((m - 0.75f) / 0.25f) * 0.7f);
        }
    }

    return finalColor;
}

FLinearColor AMapRenderer::CalculateTerrainEditColor(float h, float seaLevel) {

    if (h < seaLevel - 0.05f) return FLinearColor(0.05f, 0.1f, 0.3f); 
    if (h < seaLevel) return FLinearColor(0.15f, 0.45f, 0.65f); 
    
    float landRange = 1.0f - seaLevel;
    if (landRange <= 0.001f) landRange = 0.001f;
    
    float ratio = FMath::Clamp((h - seaLevel) / landRange, 0.0f, 1.0f);

    if (ratio < 0.15f) return FLinearColor(0.95f, 0.9f, 0.6f); // sand
    if (ratio < 0.50f) return FLinearColor(0.3f, 0.6f, 0.3f);  // grass
    if (ratio < 0.82f) return FLinearColor(0.5f, 0.4f, 0.3f);  // rock
    
    return FLinearColor::White; // snow
}

FLinearColor AMapRenderer::GetKingdomColor(int32 KingdomID) {
    if (KingdomID <= 0) return FLinearColor(0.05f, 0.05f, 0.05f); 

    int32 ColorIndex = (KingdomID - 1) % 10; 

    switch (ColorIndex)
    {
        case 0: return FLinearColor(0.08f, 0.01f, 0.02f); 
        case 1: return FLinearColor(0.02f, 0.06f, 0.03f); 
        case 2: return FLinearColor(0.02f, 0.03f, 0.10f); 
        case 3: return FLinearColor(0.15f, 0.08f, 0.01f); 
        case 4: return FLinearColor(0.07f, 0.02f, 0.09f); 
        case 5: return FLinearColor(0.02f, 0.08f, 0.09f); 
        case 6: return FLinearColor(0.12f, 0.04f, 0.01f);
        case 7: return FLinearColor(0.06f, 0.07f, 0.03f); 
        case 8: return FLinearColor(0.09f, 0.03f, 0.04f); 
        case 9: return FLinearColor(0.03f, 0.05f, 0.08f);
        default: return FLinearColor(0.05f, 0.05f, 0.05f); 
    }
}

void AMapRenderer::RedrawMapRegion(int32 MinX, int32 MinY, int32 MaxX, int32 MaxY) {
    if (!MapTexture || WorldData.Width == 0) return;

    MinX = FMath::Max(0, MinX - 2);
    MinY = FMath::Max(0, MinY - 2);
    MaxX = FMath::Min(WorldData.Width - 1, MaxX + 2);
    MaxY = FMath::Min(WorldData.Height - 1, MaxY + 2);

    int32 RegionW = MaxX - MinX + 1;
    int32 RegionH = MaxY - MinY + 1;
    if (RegionW <= 0 || RegionH <= 0) return;

    FColor* RegionData = new FColor[RegionW * RegionH];

    for (int32 y = MinY; y <= MaxY; y++) {
        for (int32 x = MinX; x <= MaxX; x++) {
            int32 GlobalIndex = y * WorldData.Width + x;
            float h = WorldData.Heights[GlobalIndex];
            float m = (WorldData.Moisture.Num() > GlobalIndex) ? WorldData.Moisture[GlobalIndex] : 0.5f;

            FLinearColor FinalColor = FLinearColor::Black;

            if (CurrentView == EViewMode::TerrainEdit) {
                FinalColor = CalculateTerrainEditColor(h, WorldData.SeaLevel);
            } else {
                if (h < WorldData.SeaLevel) {
                    float depth = h / WorldData.SeaLevel;
                    float tightGlow = FMath::Pow(depth, 10.0f);
                    FLinearColor OceanColor = FLinearColor::FromSRGBColor(FColor(20, 30, 50));
                    FLinearColor CoastColor = FLinearColor::FromSRGBColor(FColor(45, 80, 110));
                    FinalColor = FMath::Lerp(OceanColor, CoastColor, tightGlow);
                } else {
                    if (CurrentView == EViewMode::Physical) {
                        FinalColor = GetBiomeColor(h, m, WorldData.SeaLevel);
                    } else if (CurrentView == EViewMode::Political) {
                        int32 kID = WorldData.KingdomMap[GlobalIndex]; 
                        FLinearColor KColor = GetKingdomColor(kID);
                        FLinearColor BiomeBase = GetBiomeColor(h, m, WorldData.SeaLevel);
                        
                        float Glow = (BiomeBase.R * 0.3f) + (BiomeBase.G * 0.59f) + (BiomeBase.B * 0.11f);
                        FLinearColor PaperTerrain = FLinearColor(Glow, Glow, Glow);
                        
                        FinalColor = FMath::Lerp(PaperTerrain, KColor, 0.85f);
                    }
                    
                    if (x > 0 && y < WorldData.Height - 1) {
                        float hTL = WorldData.Heights[(y + 1) * WorldData.Width + (x - 1)];
                        float diff = h - hTL;
                        
                        FLinearColor ShadowColor = FLinearColor::FromSRGBColor(FColor(40, 30, 20));
                        
                        if (diff < 0) {
                            float ShadowAlpha = FMath::Clamp(FMath::Abs(diff) * 20.0f, 0.0f, 1.0f);
                            FinalColor = FMath::Lerp(FinalColor, ShadowColor, ShadowAlpha);
                        } else {
                            float HighlightAlpha = FMath::Clamp(diff * 15.0f, 0.0f, 1.0f);
                            FinalColor = FMath::Lerp(FinalColor, FLinearColor::White, HighlightAlpha);
                        }
                    }
                }
            }

            if (bShowBorders && h >= WorldData.SeaLevel) {
                if (x < WorldData.Width - 1 && y < WorldData.Height - 1) {
                    int32 rightID = WorldData.KingdomMap[GlobalIndex + 1];
                    int32 downID = WorldData.KingdomMap[GlobalIndex + WorldData.Width];
                    int32 currentID = WorldData.KingdomMap[GlobalIndex];

                    if (currentID != rightID || currentID != downID) {
                        FLinearColor BorderGray = FLinearColor(0.02f, 0.02f, 0.02f, 1.0f);
                        FinalColor = FMath::Lerp(FinalColor, BorderGray, 0.85f); 
                    }
                }
            }

            float CustomGamma = 1.9f; 
            FinalColor.R = FMath::Pow(FinalColor.R, 1.0f / CustomGamma);
            FinalColor.G = FMath::Pow(FinalColor.G, 1.0f / CustomGamma);
            FinalColor.B = FMath::Pow(FinalColor.B, 1.0f / CustomGamma);
            FinalColor.A = 1.0f; 

            int32 LocalBufferIndex = (y - MinY) * RegionW + (x - MinX);
            RegionData[LocalBufferIndex] = FinalColor.ToFColor(false);
        }
    }

    FUpdateTextureRegion2D* Region = new FUpdateTextureRegion2D(MinX, MinY, 0, 0, RegionW, RegionH);
    
    MapTexture->UpdateTextureRegions(0, 1, Region, RegionW * sizeof(FColor), sizeof(FColor), (uint8*)RegionData, [](uint8* SrcData, const FUpdateTextureRegion2D* Regions) {
            delete[] (FColor*)SrcData; 
            delete Regions;
        });
}


void AMapRenderer::SetSeaLevel(float NewSeaLevel) {
    WorldData.SeaLevel = NewSeaLevel;
}

void AMapRenderer::UpdateHeightStats() {
  
    if (WorldData.Heights.Num() == 0) return;
    
    int32 LandPixels = 0;
    int32 MountainPixels = 0;
    double TotalLandHeight = 0.0; 
    int32 TotalPixels = WorldData.Heights.Num();

    for (float h : WorldData.Heights) {
        if (h >= WorldData.SeaLevel) {
            LandPixels++;
            TotalLandHeight += h; 

            if (h >= 0.75f) {
                MountainPixels++;
            }
        }
    }

    if (TotalPixels > 0) LandmassPercentage = (float)LandPixels / (float)TotalPixels;
    
    
    if (LandPixels > 0) {
        AverageElevation = (float)(TotalLandHeight / (double)LandPixels);
        MountainPercentage = (float)MountainPixels / (float)LandPixels;
    } else {
        AverageElevation = 0.0f;
        MountainPercentage = 0.0f;
    }
}