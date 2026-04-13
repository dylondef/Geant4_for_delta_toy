#include "SteppingAction.hh"
#include "EventAction.hh"
#include "RunAction.hh"
#include "DetectorConstruction.hh"

#include "G4Step.hh"
#include "G4Track.hh"
#include "G4LogicalVolume.hh"
#include "G4ParticleDefinition.hh"
#include "G4SystemOfUnits.hh"
#include "G4PhysicalConstants.hh"
#include "G4VProcess.hh"
#include "G4RunManager.hh"

#include <cstring>
#include <cmath>

SteppingAction::SteppingAction(EventAction* eventAction)
: G4UserSteppingAction(), fEventAction(eventAction)
{}

//void SteppingAction::ClearEvent()
//{
//    fTrackParticle.clear();
//    fFromPrimaryPi.clear();
//    fCherCumul.clear();
//    fTrackLen.clear();
//    fStepCount.clear();
////}

void SteppingAction::UserSteppingAction(const G4Step* step)
{
    // ── Volume filter ─────────────────────────────────────────────────────────
    G4LogicalVolume* vol =
        step->GetPreStepPoint()->GetTouchableHandle()
             ->GetVolume()->GetLogicalVolume();

    const G4String& volName = vol->GetName();
    bool inWater = (volName == "ANNIETank");
    bool inSteel = (volName == "SteelShell");
    if (!inWater && !inSteel) return;

    const G4Track* track = step->GetTrack();
    const G4ParticleDefinition* pd = track->GetParticleDefinition();
    const G4String& pname = pd->GetParticleName();
    int track_id  = track->GetTrackID();
    int parent_id = track->GetParentID();

    // ── Register particle name map ────────────────────────────────────────────
    if (fTrackParticle.find(track_id) == fTrackParticle.end())
        fTrackParticle[track_id] = std::string(pname);

    // ── Primary pi+ ancestry map ──────────────────────────────────────────────
    if (fFromPrimaryPi.find(track_id) == fFromPrimaryPi.end()) {
        if (pname == "pi+" && parent_id == 0) {
            // This IS the primary pi+
            fFromPrimaryPi[track_id] = true;
            auto& sum = fEventAction->GetSummary();
            if (sum.pi_track_id < 0)
                sum.pi_track_id = track_id;
        }
        else if (fFromPrimaryPi.count(parent_id) &&
                 fFromPrimaryPi.at(parent_id)) {
            // Parent descends from primary pi+ — so does this track
            fFromPrimaryPi[track_id] = true;
        }
        else {
            // Not from primary pi+
            fFromPrimaryPi[track_id] = false;
            // Count secondary pi+ tracks specifically
            if (pname == "pi+" && parent_id != 0) {
                auto& sum = fEventAction->GetSummary();
                sum.n_secondary_pi += 1;
            }
        }
    }

    // ── Only fill StepData for water steps ────────────────────────────────────
    if (!inWater) return;

    // ── Per-track accumulators ────────────────────────────────────────────────
    double step_len_mm = step->GetStepLength() / mm;
    fTrackLen[track_id]  += step_len_mm;
    fStepCount[track_id] += 1;

    // ── Cherenkov yield (Frank-Tamm) ──────────────────────────────────────────
    double beta     = step->GetPreStepPoint()->GetBeta();
    int    charge_z = (int)std::round(pd->GetPDGCharge());
    double cher_this = 0.0;
    if (charge_z != 0) {
        std::vector<CherenkovStep> spec;
        cher_this = fCher.ComputePhotonCount(beta, charge_z, step_len_mm);
    }
    fCherCumul[track_id] += cher_this;

    // ── Build StepData ────────────────────────────────────────────────────────
    StepData s;
    s.event_id   = G4RunManager::GetRunManager()
                       ->GetCurrentEvent()->GetEventID();
    s.track_id   = track_id;
    s.parent_id  = parent_id;
    s.step_num   = track->GetCurrentStepNumber();

    strncpy(s.particle, pname.c_str(), 15);
    s.particle[15] = '\0';

    // Key new field — does this step's particle trace back to the primary pi+?
    s.from_primary_pi = (fFromPrimaryPi.count(track_id) &&
                          fFromPrimaryPi.at(track_id));

    const G4ThreeVector& pos = step->GetPostStepPoint()->GetPosition();
    s.x = pos.x() / mm;
    s.y = pos.y() / mm;
    s.z = pos.z() / mm;

    const G4ThreeVector& mom = step->GetPreStepPoint()->GetMomentum();
    s.px    = mom.x() / GeV;
    s.py    = mom.y() / GeV;
    s.pz    = mom.z() / GeV;
    s.p_mag = mom.mag() / GeV;

    s.KE          = step->GetPreStepPoint()->GetKineticEnergy() / GeV;
    s.edep        = step->GetTotalEnergyDeposit() / GeV;
    s.step_len    = step_len_mm;
    s.global_time = step->GetPreStepPoint()->GetGlobalTime() / ns;
    s.beta        = beta;

    double mass_MeV = pd->GetPDGMass();
    double E_MeV    = step->GetPreStepPoint()->GetTotalEnergy() / MeV;
    s.gamma = (mass_MeV > 0) ? E_MeV / mass_MeV : 1.0;

    s.cher_this_step = cher_this;
    s.cher_cumul     = fCherCumul[track_id];
    s.track_len      = fTrackLen[track_id];

    const G4VProcess* proc = step->GetPostStepPoint()->GetProcessDefinedStep();
    s.process = proc ? proc->GetProcessName() : "Transportation";

    fEventAction->RecordStep(s);
}
