#include "TrackingAction.hh"
#include "EventAction.hh"
#include "DetectorConstruction.hh"

#include "G4Track.hh"
#include "G4ParticleDefinition.hh"
#include "G4SystemOfUnits.hh"
#include "G4VProcess.hh"

TrackingAction::TrackingAction(EventAction* eventAction)
: G4UserTrackingAction(), fEventAction(eventAction)
{}

void TrackingAction::PreUserTrackingAction(const G4Track* track)
{
    const G4String& pname = track->GetParticleDefinition()->GetParticleName();
    auto& sum = fEventAction->GetSummary();

    const G4VProcess* creator  = track->GetCreatorProcess();
    const G4String creatorName = creator ? creator->GetProcessName() : "primary";

    // ── Secondary mu+ birth time and KE ──────────────────────────────────────
    // Only record smu_t_start for the FIRST mu+ we see — that's the one from
    // the primary pi+ decay. Secondary mu+ from hadronic secondary pi+ will
    // appear later and smu_t_start will already be set so they're skipped.
    if (pname == "mu+" && track->GetParentID() > 0) {
        if (sum.smu_t_start == 0.0) {
            sum.smu_t_start = track->GetGlobalTime() / ns;
            sum.smu_KE0     = track->GetKineticEnergy() / GeV;
        }
    }

    // ── Michel / pienu e+ timing ──────────────────────────────────────────────
    // First e+ from a Decay process gets its timing recorded here.
    // EventAction::RecordStep routes definitively via parent map.
    if (pname == "e+" && creatorName == "Decay") {
        double ke0 = track->GetKineticEnergy() / GeV;
        double t0  = track->GetGlobalTime() / ns;
        if (!sum.michel_e_present && !sum.pienu_present) {
            sum.michel_e_KE0        = ke0;
            sum.michel_e_time_start = t0;
            sum.pienu_e_KE0         = ke0;  // tentative — corrected by EventAction
        }
    }
}

void TrackingAction::PostUserTrackingAction(const G4Track* track)
{
    const G4String& pname = track->GetParticleDefinition()->GetParticleName();
    auto& sum = fEventAction->GetSummary();

    const G4ThreeVector& pos = track->GetPosition();
    double x_mm = pos.x() / mm;
    double y_mm = pos.y() / mm;
    double z_mm = pos.z() / mm;
    double x_cm = x_mm / 10.0;
    double y_cm = y_mm / 10.0;
    double z_cm = z_mm / 10.0;
    double t    = track->GetGlobalTime() / ns;

    const G4VProcess* proc = track->GetStep()
                                 ? track->GetStep()->GetPostStepPoint()
                                         ->GetProcessDefinedStep()
                                 : nullptr;
    const char* procName = proc ? proc->GetProcessName().c_str()
                                : "Transportation";

    // ── Primary mu- ───────────────────────────────────────────────────────────
    if (pname == "mu-" && track->GetParentID() == 0) {
        sum.mu_x_stop     = x_mm;
        sum.mu_y_stop     = y_mm;
        sum.mu_z_stop     = z_mm;
        sum.mu_time_stop  = t;
        sum.mu_KE_at_stop = track->GetKineticEnergy() / GeV;
        sum.mu_contained  = ANNIETank::Inside(x_cm, y_cm, z_cm);
        strncpy(sum.mu_final_process, procName, 31);
        sum.mu_final_process[31] = '\0';
    }

    // ── Primary pi+ ───────────────────────────────────────────────────────────
    else if (pname == "pi+" && track->GetParentID() == 0) {
        sum.pi_x_end     = x_mm;
        sum.pi_y_end     = y_mm;
        sum.pi_z_end     = z_mm;
        sum.pi_time_end  = t;
        // NOTE: pi_time_end = true decay/interaction time — pion sits at rest
        // below Cherenkov threshold for ~26 ns before this fires.
        // Use StepTree last step for Cherenkov cutoff; pi_time_end for timing.
        sum.pi_contained = ANNIETank::Inside(x_cm, y_cm, z_cm);
        strncpy(sum.pi_final_process, procName, 31);
        sum.pi_final_process[31] = '\0';
    }

    // ── Secondary mu+ ─────────────────────────────────────────────────────────
    else if (pname == "mu+") {
        sum.smu_x_stop     = x_mm;
        sum.smu_y_stop     = y_mm;
        sum.smu_z_stop     = z_mm;
        sum.smu_time_stop  = t;
        sum.smu_KE_at_stop = track->GetKineticEnergy() / GeV;
        sum.smu_contained  = ANNIETank::Inside(x_cm, y_cm, z_cm);
        // mu+ lab lifetime = smu_time_stop - smu_t_start (~2197 ns × gamma)
    }

    // ── e+ — routed in EventAction::RecordStep, nothing to do here ────────────
}
