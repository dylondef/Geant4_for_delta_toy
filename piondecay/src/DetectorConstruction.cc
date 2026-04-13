#include "DetectorConstruction.hh"

#include "G4Tubs.hh"
#include "G4Box.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4NistManager.hh"
#include "G4SystemOfUnits.hh"
#include "G4VisAttributes.hh"
#include "G4Colour.hh"

DetectorConstruction::DetectorConstruction()
: G4VUserDetectorConstruction()
{}

G4VPhysicalVolume* DetectorConstruction::Construct()
{
    G4NistManager* nist = G4NistManager::Instance();

    // ── Materials ─────────────────────────────────────────────────────────────
    G4Material* air   = nist->FindOrBuildMaterial("G4_AIR");
    G4Material* water = nist->FindOrBuildMaterial("G4_WATER");
    G4Material* steel = nist->FindOrBuildMaterial("G4_STAINLESS-STEEL");

    // ── Dimensions (all in Geant4 units — mm internally, we define in cm) ────
    // ANNIE inner tank
    const G4double tankR    = kTankRadius     * cm;   // 160 cm
    const G4double tankHH   = kTankHalfHeight * cm;   // 200 cm half-height
    const G4double steelT   = kSteelThickness * cm;   // 1.27 cm steel wall

    // Steel shell outer dimensions
    const G4double steelR   = tankR  + steelT;
    const G4double steelHH  = tankHH + steelT;

    // World box — just large enough to hold the steel shell + 50cm clearance
    const G4double worldHH  = steelHH + 50.*cm;
    const G4double worldR   = steelR  + 50.*cm;

    // ── World ─────────────────────────────────────────────────────────────────
    G4Box* worldSolid = new G4Box("World", worldR, worldR, worldHH);
    G4LogicalVolume* worldLog =
        new G4LogicalVolume(worldSolid, air, "World");
    G4VPhysicalVolume* worldPhys =
        new G4PVPlacement(nullptr, G4ThreeVector(),
                          worldLog, "World", nullptr, false, 0, true);
    worldLog->SetVisAttributes(G4VisAttributes::GetInvisible());

    // ── Steel shell (outer cylinder) ──────────────────────────────────────────
    // Full cylinder including caps — approximated as one solid tube
    // In reality ANNIE has flanged ends but this is close enough for
    // Cherenkov and ranging purposes
    G4Tubs* steelSolid = new G4Tubs(
        "SteelShell",
        0.,          // inner radius (solid — water is placed inside)
        steelR,      // outer radius
        steelHH,     // half-height
        0., 360.*deg // full phi
    );
    G4LogicalVolume* steelLog =
        new G4LogicalVolume(steelSolid, steel, "SteelShell");
    new G4PVPlacement(nullptr, G4ThreeVector(),
                      steelLog, "SteelShell",
                      worldLog, false, 0, true);

    G4VisAttributes* steelVis =
        new G4VisAttributes(G4Colour(0.6, 0.6, 0.6, 0.4));  // grey, semi-transparent
    steelVis->SetVisibility(true);
    steelLog->SetVisAttributes(steelVis);

    // ── Inner water volume (the active ANNIE tank) ────────────────────────────
    G4Tubs* waterSolid = new G4Tubs(
        "ANNIETank",
        0.,
        tankR,
        tankHH,
        0., 360.*deg
    );
    fWaterLogical = new G4LogicalVolume(waterSolid, water, "ANNIETank");
    new G4PVPlacement(nullptr, G4ThreeVector(),
                      fWaterLogical, "ANNIETank",
                      steelLog, false, 0, true);

    G4VisAttributes* waterVis =
        new G4VisAttributes(G4Colour(0.0, 0.5, 1.0, 0.15));  // blue, transparent
    waterVis->SetVisibility(true);
    fWaterLogical->SetVisAttributes(waterVis);

    return worldPhys;
}
