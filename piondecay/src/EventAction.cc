#include "EventAction.hh"
#include "SteppingAction.hh"
#include "RunAction.hh"
#include "G4Event.hh"
#include <cstring>

EventAction::EventAction(RunAction* runAction)
: G4UserEventAction(), fRunAction(runAction)
{}

void EventAction::BeginOfEventAction(const G4Event*)
{
    fSteps.clear();
    // Preserve GENIE kinematics written by PrimaryGeneratorAction
    // but reset all tracking-filled fields for the new event
    EventSummary fresh;
    fresh.event_id       = fSummary.event_id;
    fresh.Enu            = fSummary.Enu;
    fresh.cos_open       = fSummary.cos_open;
    fresh.open_angle_deg = fSummary.open_angle_deg;
    fresh.vtx_x          = fSummary.vtx_x;
    fresh.vtx_y          = fSummary.vtx_y;
    fresh.vtx_z          = fSummary.vtx_z;
    fresh.mu_px0         = fSummary.mu_px0;
    fresh.mu_py0         = fSummary.mu_py0;
    fresh.mu_pz0         = fSummary.mu_pz0;
    fresh.mu_p0          = fSummary.mu_p0;
    fresh.mu_KE0         = fSummary.mu_KE0;
    fresh.pi_px0         = fSummary.pi_px0;
    fresh.pi_py0         = fSummary.pi_py0;
    fresh.pi_pz0         = fSummary.pi_pz0;
    fresh.pi_p0          = fSummary.pi_p0;
    fresh.pi_KE0         = fSummary.pi_KE0;
    fSummary    = fresh;
    fHasSummary = true;
    if (fSteppingAction) fSteppingAction->ClearEvent();
}
void EventAction::RecordStep(const StepData& s)
{
    fSteps.push_back(s);

    const char* p = s.particle;

    // ── Primary mu- (signal lepton, parent_id == 0) ───────────────────────────
    if (strcmp(p, "mu-") == 0 && s.parent_id == 0) {
        fSummary.mu_cher_total += s.cher_this_step;
        fSummary.mu_nsteps     += 1;
        fSummary.mu_track_len   = s.track_len;
    }

    // ── Primary pi+ (parent_id == 0) ─────────────────────────────────────────
    else if (strcmp(p, "pi+") == 0 && s.parent_id == 0) {
        fSummary.pi_cher_total += s.cher_this_step;
        fSummary.pi_nsteps     += 1;
        fSummary.pi_track_len   = s.track_len;
    }

    // ── Secondary mu+ (from pi+ → mu+ decay, parent_id > 0) ──────────────────
    else if (strcmp(p, "mu+") == 0) {
        fSummary.smu_cher_total += s.cher_this_step;
        fSummary.smu_nsteps     += 1;
        fSummary.smu_track_len   = s.track_len;
    }

    // ── e+ — must distinguish TWO sources by parent particle ─────────────────
    //
    // SOURCE A — Normal Michel: pi+ → mu+ → e+
    //   e+ parent_id points to a mu+ track
    //   Arrives DELAYED (~2.2 μs), energy ≤ 52.83 MeV
    //   This is the signal tag for CC-pi+ events
    //
    // SOURCE B — Rare pion decay: pi+ → e+ + nu_e  (BR ~ 1.23e-4)
    //   e+ parent_id points directly to the pi+ track (no mu+ intermediate)
    //   Arrives PROMPT (same time as pion), energy up to ~70 MeV
    //   This is a BACKGROUND — looks like extra prompt light, no delayed tag
    //   ~37 events expected per 30k events
    //
    // Discrimination: look up what particle owns parent_id in fTrackParticle
    // map (maintained by SteppingAction, accessed via getter)
    else if (strcmp(p, "e+") == 0) {
        const std::string& parent_pname =
            fSteppingAction->GetParticleName(s.parent_id);

        if (parent_pname == "mu+") {
            // Normal Michel e+ — delayed, from pion decay chain
            fSummary.michel_e_cher_total += s.cher_this_step;
            fSummary.michel_e_nsteps     += 1;
            fSummary.michel_e_present     = true;
        }
        else if (parent_pname == "pi+") {
            // Rare pi → e+ nu_e — PROMPT background
            fSummary.pienu_e_cher_total += s.cher_this_step;
            fSummary.pienu_e_nsteps     += 1;
            fSummary.pienu_present       = true;
        }
        // (e+ from other parents — annihilation products etc — ignored)
    }

    fSummary.total_edep += s.edep;
}

void EventAction::EndOfEventAction(const G4Event*)
{
    // ── Compute totals ────────────────────────────────────────────────────────
    fSummary.total_cher = fSummary.mu_cher_total
                        + fSummary.pi_cher_total
                        + fSummary.smu_cher_total
                        + fSummary.michel_e_cher_total
                        + fSummary.pienu_e_cher_total;

    // Prompt = mu- + pi+ + pienu_e+ (all arrive within ~50ns of vertex)
    fSummary.prompt_cher  = fSummary.mu_cher_total
                          + fSummary.pi_cher_total
                          + fSummary.pienu_e_cher_total;

    // Delayed = Michel e+ only (>500ns, follows muon lifetime)
    fSummary.delayed_cher = fSummary.michel_e_cher_total;

    // Fill trees
    for (const auto& s : fSteps)
        fRunAction->FillStepTree(s);

    if (fHasSummary)
        fRunAction->FillEventTree(fSummary);
}
