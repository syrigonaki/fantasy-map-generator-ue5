#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "WorldDataStruct.h" 
#include "KingdomManagerComponent.generated.h"
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))

class MAPGEN_API UKingdomManagerComponent : public UActorComponent {
        GENERATED_BODY()

    public:	
        UKingdomManagerComponent();

        UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Kingdoms")
        TMap<int32, FString> KingdomNames;

        UFUNCTION(BlueprintCallable, Category = "Kingdom Generation")
        void GenerateRandomKingdoms(UPARAM(ref) FWorldData& WorldData, int32 Count);

    private:
        FIntPoint PickValidCapital(const FWorldData& WorldData);
        TArray<FIntPoint> Get8Neighbors(FIntPoint P, int32 Width, int32 Height);
        TArray<FIntPoint> GetNeighbors(FIntPoint P, int32 Width, int32 Height);
        FString GenerateKingdomName();
};