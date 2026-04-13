#include "CherenkovAnalyzer.hh"
#include <cmath>

// ── Water dispersion: Sellmeier coefficients (Daimon & Masumura 2007) ─────────
// n²(λ) = 1 + Σ Bᵢλ²/(λ²-Cᵢ)   λ in micrometres
double CherenkovAnalyzer::RefractiveIndex(double lambda_nm)
{
    double lam = lambda_nm * 1e-3;  // nm -> µm
    double lam2 = lam * lam;

    // Sellmeier terms for water at 20°C
    const double B[] = { 0.75831, 0.08495, 0.00143 };
    const double C[] = { 0.01007, 0.08997, 897.0   };  // µm²

    double n2 = 1.0;
    for (int i = 0; i < 3; ++i)
        n2 += B[i] * lam2 / (lam2 - C[i]);

    return std::sqrt(n2);
}

CherenkovAnalyzer::CherenkovAnalyzer()
{
    double dlam = (kLambda_max - kLambda_min) / kNbins;
    for (int i = 0; i < kNbins; ++i) {
        double lam = kLambda_min + (i + 0.5) * dlam;
        fLambdaBins.push_back(lam);
        fN.push_back(RefractiveIndex(lam));
    }
}

double CherenkovAnalyzer::ComputePhotons(double beta, int charge_z,
                                          double dx_mm,
                                          std::vector<CherenkovStep>& spectrum) const
{
    spectrum.clear();
    if (beta <= 0.0) return 0.0;

    double dx_cm = dx_mm * 0.1;   // mm -> cm
    double beta2 = beta * beta;
    double z2    = static_cast<double>(charge_z * charge_z);
    double dlam  = (kLambda_max - kLambda_min) / kNbins;
    double total = 0.0;

    // Frank-Tamm: d²N/dxdλ = 2πα z²/λ² * sin²θ_c
    // sin²θ_c = 1 - 1/(β²n²)
    // integrate over λ bins, dx in cm
    // Result in photons (dimensionless)
    // Note: 2πα/λ² in units of cm⁻¹ nm⁻¹ -> need λ in cm
    //   2πα z² * (1/λ²) * sin²θ_c * dλ * dx
    //   λ in nm -> λ_cm = λ_nm * 1e-7
    //   1/λ_cm² = 1e14 / λ_nm²
    //   so prefactor = 2π * α * z² * 1e7 (per cm per nm)

    const double prefactor = 2.0 * M_PI * kAlpha * z2 * 1.0e7; // cm⁻¹ nm⁻¹

    for (int i = 0; i < kNbins; ++i) {
        double lam  = fLambdaBins[i];
        double n    = fN[i];
        double bn2  = beta2 * n * n;
        if (bn2 <= 1.0) {
            spectrum.push_back({lam, 0.0});
            continue;
        }
        double sin2 = 1.0 - 1.0 / bn2;
        double dN   = prefactor * sin2 / (lam * lam) * dlam * dx_cm;
        spectrum.push_back({lam, dN});
        total += dN;
    }
    return total;
}

double CherenkovAnalyzer::ComputePhotonCount(double beta, int charge_z,
                                              double dx_mm) const
{
    std::vector<CherenkovStep> spec;
    return ComputePhotons(beta, charge_z, dx_mm, spec);
}
