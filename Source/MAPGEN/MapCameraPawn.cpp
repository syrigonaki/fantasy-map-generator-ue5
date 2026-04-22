#include "MapCameraPawn.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"

AMapCameraPawn::AMapCameraPawn() {
    PrimaryActorTick.bCanEverTick = true;

    RootComp = CreateDefaultSubobject<USceneComponent>(TEXT("RootComp"));
    RootComponent = RootComp;

    MapCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("MapCamera"));
    MapCamera->SetupAttachment(RootComp);
    MapCamera->ProjectionMode = ECameraProjectionMode::Orthographic;
    
    // looking straight down
    MapCamera->SetWorldRotation(FRotator(-90.0f, 0.0f, 0.0f));
}
void AMapCameraPawn::BeginPlay() {
    Super::BeginPlay();
    PC = Cast<APlayerController>(GetController());
    MapCamera->SetOrthoWidth(1920.0f); 
}

void AMapCameraPawn::Tick(float DeltaTime) {
    Super::Tick(DeltaTime);
    if (!PC) return; 

    HandleZoom(DeltaTime);
    HandlePan(DeltaTime);
    
    ClampCameraToMap(); 
}

void AMapCameraPawn::HandleZoom(float DeltaTime) {
    float ScrollValue = 0.0f;
    
    if (PC->WasInputKeyJustPressed(EKeys::MouseScrollUp)) ScrollValue = 1.0f;
    if (PC->WasInputKeyJustPressed(EKeys::MouseScrollDown)) ScrollValue = -1.0f;

    if (FMath::Abs(ScrollValue) > 0.0f) {
        float CurrentWidth = MapCamera->OrthoWidth;
        float ZoomStep = CurrentWidth * 0.15f; 
        float ZoomDirection = ScrollValue > 0 ? -1.0f : 1.0f; 

        float NewWidth = CurrentWidth + (ZoomDirection * ZoomStep);
        
        MapCamera->SetOrthoWidth(FMath::Clamp(NewWidth, 200.0f, MaxOrthoWidth));
    }
}

void AMapCameraPawn::HandlePan(float DeltaTime) {
    if (PC->IsInputKeyDown(EKeys::RightMouseButton)) {
        float MouseX, MouseY;
        PC->GetInputMouseDelta(MouseX, MouseY);

        if (FMath::Abs(MouseX) > 0.0f || FMath::Abs(MouseY) > 0.0f) {
            
           
            float BasePanSpeed = 20.0f; 
            float AdaptiveSpeed = (MapCamera->OrthoWidth / 1000.0f) * BasePanSpeed;
            
            FVector Move(-MouseY, -MouseX, 0.0f); // do not remove this

            AddActorWorldOffset(Move * AdaptiveSpeed, true);
        }
    }
}

void AMapCameraPawn::ClampCameraToMap() {
    float HorizontalHalfSize = MapCamera->OrthoWidth / 2.0f;
    float VerticalHalfSize = HorizontalHalfSize / MapCamera->AspectRatio;

    float MapHalfX = 1080.0f / 2.0f; 
    float MapHalfY = 1920.0f / 2.0f; 

    float MinX = -MapHalfX + VerticalHalfSize;
    float MaxX = MapHalfX - VerticalHalfSize;
    float MinY = -MapHalfY + HorizontalHalfSize;
    float MaxY = MapHalfY - HorizontalHalfSize;

    if (MaxX < MinX) { MinX = 0; MaxX = 0; }
    if (MaxY < MinY) { MinY = 0; MaxY = 0; }

    FVector Pos = GetActorLocation();
    Pos.X = FMath::Clamp(Pos.X, MinX, MaxX);
    Pos.Y = FMath::Clamp(Pos.Y, MinY, MaxY);
    SetActorLocation(Pos);
}