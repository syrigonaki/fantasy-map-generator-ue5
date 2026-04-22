#include "KingdomManagerComponent.h"

UKingdomManagerComponent::UKingdomManagerComponent() {
    PrimaryComponentTick.bCanEverTick = false;
}

void UKingdomManagerComponent::GenerateRandomKingdoms(FWorldData& WorldData, int32 Count) {
    int32 Width = WorldData.Width;
    int32 Height = WorldData.Height;

    WorldData.KingdomMap.Init(0, Width * Height);
    KingdomNames.Empty();

    TArray<FIntPoint> Frontier;

    for (int32 i = 0; i < Count; i++) {
        FIntPoint Cap = PickValidCapital(WorldData);
        WorldData.KingdomMap[Cap.Y * Width + Cap.X] = i + 1;
        Frontier.Add(Cap);
        
        if (!KingdomNames.Contains(i + 1)) {
            KingdomNames.Add(i + 1, GenerateKingdomName());
        }
    }

    int32 Safety = 0;
    while (Frontier.Num() > 0 && Safety < 5000000) {

        Safety++;
        
        // pick random
        int32 Index = FMath::RandRange(0, Frontier.Num() - 1);
        FIntPoint Current = Frontier[Index];
        int32 CurrentIndex = Current.Y * Width + Current.X;
        int32 CurrentID = WorldData.KingdomMap[CurrentIndex];

        bool bCanExpandFurther = false;
        
        
        TArray<FIntPoint> Neighbors = Get8Neighbors(Current, Width, Height);

        for (const FIntPoint& Neighbor : Neighbors) {
            int32 NIndex = Neighbor.Y * Width + Neighbor.X;
            
            if (WorldData.KingdomMap[NIndex] == 0) {
                float H = WorldData.Heights[NIndex];
                float CurrentH = WorldData.Heights[CurrentIndex];
                
        
                float Noise = FMath::PerlinNoise2D(FVector2D(Neighbor.X * 0.1f, Neighbor.Y * 0.1f)) * 0.2f;
                
               // higher regions have bigger cost

                float HeightDiff = FMath::Abs(H - CurrentH);
                float Resistance = 0.05f + (HeightDiff * 25.0f) + Noise; 

                if (FMath::FRand() < Resistance) {
                    bCanExpandFurther = true;
                    continue; 
                }

                WorldData.KingdomMap[NIndex] = CurrentID;
                Frontier.Add(Neighbor);
                bCanExpandFurther = true;
            }
        }

        if (!bCanExpandFurther) Frontier.RemoveAtSwap(Index);
    }
}


TArray<FIntPoint> UKingdomManagerComponent::Get8Neighbors(FIntPoint P, int32 Width, int32 Height) {
    TArray<FIntPoint> Neighbors;
    for (int32 x = -1; x <= 1; x++) {
        for (int32 y = -1; y <= 1; y++) {
            if (x == 0 && y == 0) continue; 

            FIntPoint CheckP(P.X + x, P.Y + y);
            if (CheckP.X >= 0 && CheckP.X < Width && CheckP.Y >= 0 && CheckP.Y < Height) {
                Neighbors.Add(CheckP);
            }
        }
    }
    return Neighbors;
}

FIntPoint UKingdomManagerComponent::PickValidCapital(const FWorldData& WorldData) {
    for (int32 i = 0; i < 5000; i++) {
        int32 X = FMath::RandRange(0, WorldData.Width - 1);
        int32 Y = FMath::RandRange(0, WorldData.Height - 1);
        int32 Index = Y * WorldData.Width + X;

        if (WorldData.Heights[Index] >= WorldData.SeaLevel && WorldData.KingdomMap[Index] == 0) {
            return FIntPoint(X, Y);
        }
    }
    return FIntPoint(0, 0);
}

TArray<FIntPoint> UKingdomManagerComponent::GetNeighbors(FIntPoint P, int32 Width, int32 Height) { // this made it too square
    
    TArray<FIntPoint> Neighbors;
    Neighbors.Reserve(4); 
    if (P.X > 0) Neighbors.Add(FIntPoint(P.X - 1, P.Y));
    if (P.X < Width - 1) Neighbors.Add(FIntPoint(P.X + 1, P.Y));
    if (P.Y > 0) Neighbors.Add(FIntPoint(P.X, P.Y - 1));
    if (P.Y < Height - 1) Neighbors.Add(FIntPoint(P.X, P.Y + 1));

    return Neighbors;
}

FString UKingdomManagerComponent::GenerateKingdomName() { 
    return "Kingdom"; //TODO: add some simple name generation
}