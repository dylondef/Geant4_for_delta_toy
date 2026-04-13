#ifndef EventData_h
#define EventData_h

#include <vector>
#include <string>

// ─────────────────────────────────────────────────────────────────────────────
// ANNIE tank dimensions — single source of truth, used by TrackingAction
// and containment checks in the notebook
// R = 160 cm, half-height = 200 cm
// ─────────────────────────────────────────────────────────────────────────────
namespace ANNIETank {
    constexpr double R        = 160.0;   // cm
    constexpr double HalfH    = 200.0;   // cm
    constexpr double SteelT   =   1.27;  // cm

    inline bool Inside(double x_cm, double y_cm, double z_cm) {
        return (x_cm*x_cm + y_cm*y_cm < R*R)
               && (z_cm > -HalfH) && (z_cm < HalfH);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Per-step data (fills StepTree)
// ─────────────────────────────────────────────────────────────────────────────
struct StepData {
    int    event_id      = 0;
    int    track_id      = 0;
    int    parent_id     = 0;
    int    step_num      = 0;
    char   particle[16]  = {};   // "mu-", "pi+", "mu+", "e+", ...
    bool  from_primary_pi = false; //this steps particle traces to the primary pi+
    double x=0, y=0, z=0;       // position [mm]
    double px=0, py=0, pz=0;    // momentum [GeV/c]
    double p_mag         = 0;
    double KE            = 0;    // [GeV]
    double edep          = 0;    // [GeV]
    double step_len      = 0;    // [mm]
    double global_time   = 0;    // [ns]
    double beta          = 0;
    double gamma         = 0;
    double cher_this_step= 0;    // Frank-Tamm photons this step
    double cher_cumul    = 0;    // cumulative for this track
    double track_len     = 0;    // cumulative [mm]
    std::string process  = "";
};

// ─────────────────────────────────────────────────────────────────────────────
// Per-event summary (fills EventTree)
// ─────────────────────────────────────────────────────────────────────────────
struct EventSummary {
    int    event_id       = 0;
    int pi_track_id       = -1; ///track id of the primary pion plus
    int n_secondary_pi    = 0 ; //count of non primary pi plus seen
    // ── GENIE kinematics (from CSV) ───────────────────────────────────────────
    double Enu            = 0.;   // neutrino energy [GeV]
    double cos_open       = 0.;   // cos(angle between mu- and pi+)
    double open_angle_deg = 0.;   // [degrees]

    // GENIE interaction vertex [cm]
    double vtx_x          = 0.;
    double vtx_y          = 0.;
    double vtx_z          = 0.;

    // ── Primary mu- (signal lepton) ───────────────────────────────────────────
    double mu_px0         = 0., mu_py0 = 0., mu_pz0 = 0.;
    double mu_p0          = 0.;   // [GeV/c]
    double mu_KE0         = 0.;   // [GeV]

    double mu_track_len   = 0.;   // [mm] — how far it got in tank
    double mu_time_stop   = 0.;   // [ns]
    double mu_x_stop      = 0., mu_y_stop = 0., mu_z_stop = 0.;  // [mm]
    double mu_KE_at_stop  = 0.;
    double mu_cher_total  = 0.;   // Cherenkov photons from primary mu-
    int    mu_nsteps      = 0;
    // Final process tells us the fate:
    //   "Transportation"        → exited the ANNIE volume (punch-through)
    //   "MuMinusCaptureAtRest"  → captured by oxygen, no Michel signal
    //   "Decay"                 → decayed to e- (not e+, so no Michel e+ here)
    char   mu_final_process[32] = {};
    bool   mu_contained   = false;  // stopped inside ANNIE cylinder

    // ── Primary pi+ (delta decay daughter) ───────────────────────────────────
    double pi_px0         = 0., pi_py0 = 0., pi_pz0 = 0.;
    double pi_p0          = 0.;
    double pi_KE0         = 0.;

    double pi_track_len   = 0.;
    double pi_time_end    = 0.;   // ns — when pi+ decays or interacts
    double pi_x_end       = 0., pi_y_end = 0., pi_z_end = 0.;
    double pi_cher_total  = 0.;
    int    pi_nsteps      = 0;
    // pi+ fate:
    //   "Decay"          → pi+ → mu+ + nu_mu  (dominant, BR ~99.99%)
    //   "pi+eNu"         → pi+ → e+ + nu_e    (rare, BR ~1.23e-4) ← KEY BACKGROUND
    //   "pi+Inelastic"   → hadronic interaction, chain ends
    char   pi_final_process[32] = {};
    bool   pi_contained   = false;

    // ── Secondary mu+ (from pi+ → mu+ + nu_mu, dominant decay) ──────────────
    // Only present when pi_final_process == "Decay"
    double smu_track_len   = 0.;
    double smu_time_stop   = 0.;
    double smu_x_stop      = 0., smu_y_stop = 0., smu_z_stop = 0.;
    double smu_KE_at_stop  = 0.;
    double smu_cher_total  = 0.;
    int    smu_nsteps      = 0;
    bool   smu_contained   = false;
    double smu_t_start     = 0.;
    double smu_KE0         = 0.;
    // ── e+ from NORMAL Michel decay: pi+ → mu+ → e+ + nu_e_bar + nu_mu ──────
    // parent_id chain: e+ → mu+ → pi+
    // This is the DELAYED signal (~2.2 μs after prompt)
    double michel_e_time_start= 0.;   // ns — when Michel e+ is born
    double michel_e_KE0       = 0.;   // GeV — Michel spectrum max 52.83 MeV
    double michel_e_cher_total = 0.;
    int    michel_e_nsteps     = 0;
    bool   michel_e_present    = false;

    // ── e+ from RARE pion decay: pi+ → e+ + nu_e  (BR ~ 1.23e-4) ────────────
    // parent_id chain: e+ → pi+ directly (no mu+ intermediate)
    // This is PROMPT — arrives with the pion Cherenkov light
    // Creates a fake "high light yield" event with NO delayed Michel signal
    // ~37 events expected per 30k — real background for delayed-tag analyses
    double pienu_e_time_start  = 0.;  // ns — should be ~same as pi+ stop time
    double pienu_e_KE0         = 0.;  // GeV — up to ~70 MeV (much harder than Michel)
    double pienu_e_cher_total  = 0.;
    int    pienu_e_nsteps      = 0;
    bool   pienu_present       = false;  // flag: this event had a pi→e decay

    // ── Combined totals ───────────────────────────────────────────────────────
    double total_cher     = 0.;   // all particles combined
    double prompt_cher    = 0.;   // mu- + pi+ + pienu_e (within ~50ns)
    double delayed_cher   = 0.;   // Michel e+ only (>500ns)
    double total_edep     = 0.;   // [GeV]
};

#endif
