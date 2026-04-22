#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "WorldDataStruct.h" 
#include "MapCameraPawn.generated.h"

UCLASS()
class MAPGEN_API AMapCameraPawn : public APawn {
    GENERATED_BODY()

    public:
        AMapCameraPawn();

    protected:
        virtual void BeginPlay() override;
        
       
        virtual void Tick(float DeltaTime) override; 

      
        UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
        class USceneComponent* RootComp;

        UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
        class UCameraComponent* MapCamera;

       
        UPROPERTY(EditAnywhere, Category = "Camera Settings")
        float PanSpeed = 200.0f;

        UPROPERTY(EditAnywhere, Category = "Camera Settings")
        float ZoomSpeed = 5000.0f;

        UPROPERTY(EditAnywhere, Category = "Camera Settings")
        float MinOrthoWidth = 1000.0f; 

        UPROPERTY(EditAnywhere, Category = "Camera Settings")
        float MaxOrthoWidth = 18000.0f;

        FWorldData MapData;

    private:
        void HandleZoom(float DeltaTime);
        void HandlePan(float DeltaTime);
        void ClampCameraToMap();

        APlayerController* PC;
};