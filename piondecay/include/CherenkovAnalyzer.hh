#ifndef CherenkovAnalyzer_h
#define CherenkovAnalyzer_h

// ─────────────────────────────────────────────────────────────────────────────
// CherenkovAnalyzer
//
// Computes Frank-Tamm Cherenkov photon yield analytically per step.
// No optical photon tracking — fast and suitable for large samples.
//
// Frank-Tamm formula (photons per unit path length per unit wavelength):
//   d²N/dxdλ = (2π α z² / λ²) * (1 - 1/(β²n²(λ)))
//   integrated over the visible range [λ_min, λ_max]
//
// For water: n ≈ 1.34 (average over visible), more precisely dispersive.
// ─────────────────────────────────────────────────────────────────────────────

#include <string>
#include <vector>

struct CherenkovStep {
    double wavelength_nm;   // photon wavelength [nm]
    double weight;          // relative weight (dN/dλ * Δλ)
};

class CherenkovAnalyzer {
public:
    CherenkovAnalyzer();

    // Returns number of Cherenkov photons for a step of length dx (mm)
    // for a particle with beta = v/c and charge z (integer)
    // Also fills wavelength spectrum weights
    double ComputePhotons(double beta, int charge_z, double dx_mm,
                          std::vector<CherenkovStep>& spectrum) const;

    // Just the photon count, no spectrum
    double ComputePhotonCount(double beta, int charge_z, double dx_mm) const;

    // Cherenkov threshold beta for water
    static double BetaThreshold() { return 1.0 / kN_water; }

    // Water refractive index (simple dispersion model)
    static double RefractiveIndex(double lambda_nm);

private:
    static constexpr double kAlpha   = 1.0 / 137.036;  // fine structure
    static constexpr double kN_water = 1.340;           // avg visible
    static constexpr double kLambda_min = 300.0;        // nm (UV cutoff water)
    static constexpr double kLambda_max = 700.0;        // nm (visible red)
    static constexpr int    kNbins = 40;                // wavelength bins

    std::vector<double> fLambdaBins;  // bin centers [nm]
    std::vector<double> fN;           // n(λ) at each bin
};

#endif
