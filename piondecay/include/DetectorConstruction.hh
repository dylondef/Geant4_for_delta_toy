#ifndef DetectorConstruction_hh
#define DetectorConstruction_hh

#include "G4VUserDetectorConstruction.hh"
#include "G4LogicalVolume.hh"

class DetectorConstruction : public G4VUserDetectorConstruction
{
public:
    DetectorConstruction();
    ~DetectorConstruction() override = default;

    G4VPhysicalVolume* Construct() override;

    // ANNIE tank dimensions — used by TrackingAction for containment checks
    static constexpr double kTankRadius    = 160.0;  // cm
    static constexpr double kTankHalfHeight= 200.0;  // cm
    static constexpr double kSteelThickness=  1.27;  // cm (~0.5 inch mild steel)

    // Containment check (cylinder): call from TrackingAction
    static bool InsideTank(double x_cm, double y_cm, double z_cm) {
        return (x_cm*x_cm + y_cm*y_cm < kTankRadius*kTankRadius)
               && (z_cm > -kTankHalfHeight) && (z_cm < kTankHalfHeight);
    }

private:
    G4LogicalVolume* fWaterLogical = nullptr;
};

#endif
