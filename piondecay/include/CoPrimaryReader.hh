#ifndef CoPrimaryReader_hh
#define CoPrimaryReader_hh

#include <string>
#include <vector>

// One row from mu_pi_coprimary_geant4.csv
// All momenta in GeV/c, energies in GeV, vertex in cm
struct CoPrimaryEvent {
    // Neutrino info
    double Enu            = 0.0;
    double wgt            = 1.0;

    // GENIE interaction vertex (cm) — place both primaries here
    // Default (0,0,0) = tank center if not in CSV
    double vtx_x          = 0.0;
    double vtx_y          = 0.0;
    double vtx_z          = 0.0;

    // Primary mu-
    double mu_px          = 0.0;
    double mu_py          = 0.0;
    double mu_pz          = 1.0;
    double mu_E           = 0.0;
    double mu_p           = 0.0;

    // Primary pi+ (from delta decay)
    double pi_px          = 0.0;
    double pi_py          = 0.0;
    double pi_pz          = 1.0;
    double pi_E           = 0.0;
    double pi_p           = 0.0;

    // Opening angle between mu- and pi+
    double cos_open       = 1.0;
    double open_angle_deg = 0.0;
};

class CoPrimaryReader {
public:
    explicit CoPrimaryReader(const std::string& filename);

    bool        IsValid() const { return fValid; }
    std::size_t Size()    const { return fEntries.size(); }

    const CoPrimaryEvent& GetEntry(std::size_t index) const;

private:
    std::vector<CoPrimaryEvent> fEntries;
    bool fValid = false;
};

#endif
