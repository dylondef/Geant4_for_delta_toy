#include "RunAction.hh"

#include "G4Run.hh"
#include "G4SystemOfUnits.hh"
#include "G4Threading.hh"
#include "G4AutoLock.hh"

#include "TFile.h"
#include "TTree.h"

#include <cstring>
#include <iostream>

namespace { G4Mutex rootMutex = G4MUTEX_INITIALIZER; }

RunAction::RunAction()  : G4UserRunAction() {}
RunAction::~RunAction() {}

void RunAction::BeginOfRunAction(const G4Run*)
{
    if (!G4Threading::IsMasterThread()) return;
    G4AutoLock lock(&rootMutex);

    fRootFile = new TFile("annie_coprimary.root", "RECREATE");

    // ════════════════════════════════════════════════════════════════════════
    // EventTree — one entry per event
    // ════════════════════════════════════════════════════════════════════════
    fEventTree = new TTree("EventTree",
        "Per-event summary | ANNIE geometry | mu- + pi+ co-primary");

    // ── Bookkeeping ───────────────────────────────────────────────────────────
    fEventTree->Branch("event_id",          &fEvBuf.event_id,          "event_id/I");

    // ── GENIE kinematics ──────────────────────────────────────────────────────
    fEventTree->Branch("Enu",               &fEvBuf.Enu,               "Enu/D");
    fEventTree->Branch("cos_open",          &fEvBuf.cos_open,          "cos_open/D");
    fEventTree->Branch("open_angle_deg",    &fEvBuf.open_angle_deg,    "open_angle_deg/D");

    fEventTree->Branch("vtx_x",             &fEvBuf.vtx_x,             "vtx_x/D");
    fEventTree->Branch("vtx_y",             &fEvBuf.vtx_y,             "vtx_y/D");
    fEventTree->Branch("vtx_z",             &fEvBuf.vtx_z,             "vtx_z/D");

    // ── Primary mu- initial state ─────────────────────────────────────────────
    fEventTree->Branch("mu_px0",            &fEvBuf.mu_px0,            "mu_px0/D");
    fEventTree->Branch("mu_py0",            &fEvBuf.mu_py0,            "mu_py0/D");
    fEventTree->Branch("mu_pz0",            &fEvBuf.mu_pz0,            "mu_pz0/D");
    fEventTree->Branch("mu_p0",             &fEvBuf.mu_p0,             "mu_p0/D");
    fEventTree->Branch("mu_KE0",            &fEvBuf.mu_KE0,            "mu_KE0/D");

    // Primary mu- track
    fEventTree->Branch("mu_track_len",      &fEvBuf.mu_track_len,      "mu_track_len/D");
    fEventTree->Branch("mu_time_stop",      &fEvBuf.mu_time_stop,      "mu_time_stop/D");
    fEventTree->Branch("mu_x_stop",         &fEvBuf.mu_x_stop,         "mu_x_stop/D");
    fEventTree->Branch("mu_y_stop",         &fEvBuf.mu_y_stop,         "mu_y_stop/D");
    fEventTree->Branch("mu_z_stop",         &fEvBuf.mu_z_stop,         "mu_z_stop/D");
    fEventTree->Branch("mu_KE_at_stop",     &fEvBuf.mu_KE_at_stop,     "mu_KE_at_stop/D");
    fEventTree->Branch("mu_cher_total",     &fEvBuf.mu_cher_total,     "mu_cher_total/D");
    fEventTree->Branch("mu_nsteps",         &fEvBuf.mu_nsteps,         "mu_nsteps/I");
    fEventTree->Branch("mu_contained",      &fEvBuf.mu_contained,      "mu_contained/O");
    fEventTree->Branch("mu_final_process",   fEvBuf.mu_final_process,  "mu_final_process/C");

    // ── Primary pi+ initial state ─────────────────────────────────────────────
    fEventTree->Branch("pi_px0",            &fEvBuf.pi_px0,            "pi_px0/D");
    fEventTree->Branch("pi_py0",            &fEvBuf.pi_py0,            "pi_py0/D");
    fEventTree->Branch("pi_pz0",            &fEvBuf.pi_pz0,            "pi_pz0/D");
    fEventTree->Branch("pi_p0",             &fEvBuf.pi_p0,             "pi_p0/D");
    fEventTree->Branch("pi_KE0",            &fEvBuf.pi_KE0,            "pi_KE0/D");

    // Primary pi+ track
    fEventTree->Branch("pi_track_len",      &fEvBuf.pi_track_len,      "pi_track_len/D");
    fEventTree->Branch("pi_time_end",       &fEvBuf.pi_time_end,       "pi_time_end/D");
    fEventTree->Branch("pi_x_end",          &fEvBuf.pi_x_end,          "pi_x_end/D");
    fEventTree->Branch("pi_y_end",          &fEvBuf.pi_y_end,          "pi_y_end/D");
    fEventTree->Branch("pi_z_end",          &fEvBuf.pi_z_end,          "pi_z_end/D");
    fEventTree->Branch("pi_cher_total",     &fEvBuf.pi_cher_total,     "pi_cher_total/D");
    fEventTree->Branch("pi_nsteps",         &fEvBuf.pi_nsteps,         "pi_nsteps/I");
    fEventTree->Branch("pi_contained",      &fEvBuf.pi_contained,      "pi_contained/O");
    fEventTree->Branch("pi_final_process",   fEvBuf.pi_final_process,  "pi_final_process/C");
    // NEW: primary pi+ track_id — cross-reference with StepTree from_primary_pi
    fEventTree->Branch("pi_track_id",       &fEvBuf.pi_track_id,       "pi_track_id/I");
    // NEW: count of secondary pi+ from hadronic interactions — Michel fake tag source
    fEventTree->Branch("n_secondary_pi",    &fEvBuf.n_secondary_pi,    "n_secondary_pi/I");

    // ── Secondary mu+ (from pi+ → mu+ + nu_mu) ───────────────────────────────
    fEventTree->Branch("smu_track_len",     &fEvBuf.smu_track_len,     "smu_track_len/D");
    fEventTree->Branch("smu_time_stop",     &fEvBuf.smu_time_stop,     "smu_time_stop/D");
    fEventTree->Branch("smu_x_stop",        &fEvBuf.smu_x_stop,        "smu_x_stop/D");
    fEventTree->Branch("smu_y_stop",        &fEvBuf.smu_y_stop,        "smu_y_stop/D");
    fEventTree->Branch("smu_z_stop",        &fEvBuf.smu_z_stop,        "smu_z_stop/D");
    fEventTree->Branch("smu_KE_at_stop",    &fEvBuf.smu_KE_at_stop,    "smu_KE_at_stop/D");
    fEventTree->Branch("smu_cher_total",    &fEvBuf.smu_cher_total,    "smu_cher_total/D");
    fEventTree->Branch("smu_nsteps",        &fEvBuf.smu_nsteps,        "smu_nsteps/I");
    fEventTree->Branch("smu_contained",     &fEvBuf.smu_contained,     "smu_contained/O");
    // NEW: mu+ birth time (= pi+ decay time) and birth KE
    fEventTree->Branch("smu_t_start",       &fEvBuf.smu_t_start,       "smu_t_start/D");
    fEventTree->Branch("smu_KE0",           &fEvBuf.smu_KE0,           "smu_KE0/D");

    // ── Michel e+ (from pi+ → mu+ → e+, DELAYED ~2.2 μs) ────────────────────
    fEventTree->Branch("michel_e_time_start",&fEvBuf.michel_e_time_start,"michel_e_time_start/D");
    fEventTree->Branch("michel_e_KE0",      &fEvBuf.michel_e_KE0,      "michel_e_KE0/D");
    fEventTree->Branch("michel_e_cher_total",&fEvBuf.michel_e_cher_total,"michel_e_cher_total/D");
    fEventTree->Branch("michel_e_nsteps",   &fEvBuf.michel_e_nsteps,   "michel_e_nsteps/I");
    fEventTree->Branch("michel_e_present",  &fEvBuf.michel_e_present,  "michel_e_present/O");

    // ── Rare pi → e+ nu_e decay (PROMPT background, BR ~ 1.23e-4) ────────────
    fEventTree->Branch("pienu_e_time_start",&fEvBuf.pienu_e_time_start, "pienu_e_time_start/D");
    fEventTree->Branch("pienu_e_KE0",       &fEvBuf.pienu_e_KE0,       "pienu_e_KE0/D");
    fEventTree->Branch("pienu_e_cher_total",&fEvBuf.pienu_e_cher_total, "pienu_e_cher_total/D");
    fEventTree->Branch("pienu_e_nsteps",    &fEvBuf.pienu_e_nsteps,    "pienu_e_nsteps/I");
    fEventTree->Branch("pienu_present",     &fEvBuf.pienu_present,     "pienu_present/O");

    // ── Combined totals ───────────────────────────────────────────────────────
    fEventTree->Branch("total_cher",        &fEvBuf.total_cher,        "total_cher/D");
    fEventTree->Branch("prompt_cher",       &fEvBuf.prompt_cher,       "prompt_cher/D");
    fEventTree->Branch("delayed_cher",      &fEvBuf.delayed_cher,      "delayed_cher/D");
    fEventTree->Branch("total_edep",        &fEvBuf.total_edep,        "total_edep/D");

    // ════════════════════════════════════════════════════════════════════════
    // StepTree — one entry per Geant4 step inside ANNIETank water volume
    // ════════════════════════════════════════════════════════════════════════
    fStepTree = new TTree("StepTree", "Per-step detail | ANNIE water volume only");

    fStepTree->Branch("event_id",        &fS_event_id,        "event_id/I");
    fStepTree->Branch("track_id",        &fS_track_id,        "track_id/I");
    fStepTree->Branch("parent_id",       &fS_parent_id,       "parent_id/I");
    fStepTree->Branch("step_num",        &fS_step_num,        "step_num/I");
    fStepTree->Branch("particle",         fS_particle,        "particle/C");
    // NEW: true if this step's particle descends from the primary pi+
    // Includes: primary pi+ itself, mu+ from pi+ decay, Michel e+, etc.
    // False for: hadronic secondary pi+, their mu+, their Michel e+ (fake tags)
    fStepTree->Branch("from_primary_pi", &fS_from_primary_pi, "from_primary_pi/O");
    fStepTree->Branch("x",               &fS_x,               "x/D");
    fStepTree->Branch("y",               &fS_y,               "y/D");
    fStepTree->Branch("z",               &fS_z,               "z/D");
    fStepTree->Branch("px",              &fS_px,              "px/D");
    fStepTree->Branch("py",              &fS_py,              "py/D");
    fStepTree->Branch("pz",              &fS_pz,              "pz/D");
    fStepTree->Branch("p_mag",           &fS_p_mag,           "p_mag/D");
    fStepTree->Branch("KE",              &fS_KE,              "KE/D");
    fStepTree->Branch("edep",            &fS_edep,            "edep/D");
    fStepTree->Branch("step_len",        &fS_step_len,        "step_len/D");
    fStepTree->Branch("time",            &fS_global_time,     "time/D");
    fStepTree->Branch("beta",            &fS_beta,            "beta/D");
    fStepTree->Branch("gamma",           &fS_gamma,           "gamma/D");
    fStepTree->Branch("cher_step",       &fS_cher_step,       "cher_step/D");
    fStepTree->Branch("cher_cumul",      &fS_cher_cumul,      "cher_cumul/D");
    fStepTree->Branch("track_len",       &fS_track_len,       "track_len/D");
    fStepTree->Branch("process",          fS_process,         "process/C");
}

void RunAction::EndOfRunAction(const G4Run*)
{
    if (!G4Threading::IsMasterThread()) return;
    G4AutoLock lock(&rootMutex);
    if (fRootFile) {
        fRootFile->Write();
        fRootFile->Close();
        delete fRootFile;
        fRootFile = nullptr;
        std::cout << "\n[RunAction] ROOT file 'annie_coprimary.root' written.\n";
    }
}

void RunAction::FillEventTree(const EventSummary& ev)
{
    G4AutoLock lock(&rootMutex);
    fEvBuf = ev;
    fEventTree->Fill();
}

void RunAction::FillStepTree(const StepData& s)
{
    G4AutoLock lock(&rootMutex);
    fS_event_id       = s.event_id;
    fS_track_id       = s.track_id;
    fS_parent_id      = s.parent_id;
    fS_step_num       = s.step_num;
    strncpy(fS_particle, s.particle, 15); fS_particle[15] = '\0';
    fS_from_primary_pi= s.from_primary_pi;
    fS_x              = s.x;
    fS_y              = s.y;
    fS_z              = s.z;
    fS_px             = s.px;
    fS_py             = s.py;
    fS_pz             = s.pz;
    fS_p_mag          = s.p_mag;
    fS_KE             = s.KE;
    fS_edep           = s.edep;
    fS_step_len       = s.step_len;
    fS_global_time    = s.global_time;
    fS_beta           = s.beta;
    fS_gamma          = s.gamma;
    fS_cher_step      = s.cher_this_step;
    fS_cher_cumul     = s.cher_cumul;
    fS_track_len      = s.track_len;
    strncpy(fS_process, s.process.c_str(), 31); fS_process[31] = '\0';
    fStepTree->Fill();
}
