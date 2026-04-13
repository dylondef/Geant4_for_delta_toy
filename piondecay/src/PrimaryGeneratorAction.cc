#include "PrimaryGeneratorAction.hh"
#include "EventAction.hh"

#include "G4ParticleGun.hh"
#include "G4ParticleTable.hh"
#include "G4ParticleDefinition.hh"
#include "G4SystemOfUnits.hh"
#include "G4Event.hh"
#include "G4ThreeVector.hh"

#include <cmath>

PrimaryGeneratorAction::PrimaryGeneratorAction(EventAction* eventAction)
: G4VUserPrimaryGeneratorAction(), fEventAction(eventAction)
{
    G4ParticleTable* pt = G4ParticleTable::GetParticleTable();

    // ── mu- gun ───────────────────────────────────────────────────────────────
    fMuonGun = new G4ParticleGun(1);
    fMuonGun->SetParticleDefinition(pt->FindParticle("mu-"));
    fMuonGun->SetParticlePosition(G4ThreeVector(0, 0, 0));

    // ── pi+ gun ───────────────────────────────────────────────────────────────
    fPionGun = new G4ParticleGun(1);
    fPionGun->SetParticleDefinition(pt->FindParticle("pi+"));
    fPionGun->SetParticlePosition(G4ThreeVector(0, 0, 0));

    // ── CSV reader ────────────────────────────────────────────────────────────
    // Try one directory up (run from build/), then current directory
    fCoPrimReader = new CoPrimaryReader("../mu_pi_coprimary_geant4.csv");
    if (!fCoPrimReader->IsValid()) {
        delete fCoPrimReader;
        fCoPrimReader = new CoPrimaryReader("mu_pi_coprimary_geant4.csv");
    }
    if (!fCoPrimReader->IsValid()) {
        G4cerr << "[PrimaryGenerator] WARNING: mu_pi_coprimary_geant4.csv not found!\n"
               << "  Falling back to: mu- at 600 MeV/c +Z, pi+ at 300 MeV/c +Z\n";
    } else {
        G4cout << "[PrimaryGenerator] Loaded " << fCoPrimReader->Size()
               << " co-primary events from CSV\n";
    }
}

PrimaryGeneratorAction::~PrimaryGeneratorAction()
{
    delete fMuonGun;
    delete fPionGun;
    delete fCoPrimReader;
}

void PrimaryGeneratorAction::GeneratePrimaries(G4Event* event)
{
    // ── Kinematic defaults (fallback if no CSV) ───────────────────────────────
    double mu_px = 0.0, mu_py = 0.0, mu_pz = 0.600;   // GeV/c
    double pi_px = 0.0, pi_py = 0.0, pi_pz = 0.300;
    double vtx_x = 0.0, vtx_y = 0.0, vtx_z = 0.0;     // cm
    double Enu   = 0.0, cos_open = 1.0, open_angle_deg = 0.0;

    if (fCoPrimReader && fCoPrimReader->IsValid()) {
        const CoPrimaryEvent& ev = fCoPrimReader->GetEntry(fEventCount);
        mu_px = ev.mu_px;  mu_py = ev.mu_py;  mu_pz = ev.mu_pz;
        pi_px = ev.pi_px;  pi_py = ev.pi_py;  pi_pz = ev.pi_pz;
        vtx_x = ev.vtx_x;  vtx_y = ev.vtx_y;  vtx_z = ev.vtx_z;
        Enu           = ev.Enu;
        cos_open      = ev.cos_open;
        open_angle_deg= ev.open_angle_deg;
    }

    // ── Vertex position (cm → mm for Geant4) ─────────────────────────────────
    G4ThreeVector vertex(vtx_x * cm, vtx_y * cm, vtx_z * cm);
    fMuonGun->SetParticlePosition(vertex);
    fPionGun->SetParticlePosition(vertex);

    // ── KE = E - m  (momenta in GeV/c, masses in GeV/c²) ────────────────────
    const double kMassPi = 0.139570;
    const double kMassMu = 0.105658;
    auto ke_from_p = [](double px, double py, double pz, double mass) {
        double pmag = std::sqrt(px*px + py*py + pz*pz);
        return std::sqrt(pmag*pmag + mass*mass) - mass;
    };

    double mu_KE = ke_from_p(mu_px, mu_py, mu_pz, kMassMu);
    double pi_KE = ke_from_p(pi_px, pi_py, pi_pz, kMassPi);

    // Guard against negative or tiny KE (numerical edge cases)
    if (mu_KE < 1e-4) mu_KE = 1e-4;
    if (pi_KE < 1e-4) pi_KE = 1e-4;

    // ── Normalise direction vectors ───────────────────────────────────────────
    auto norm3 = [](double x, double y, double z) -> G4ThreeVector {
        double mag = std::sqrt(x*x + y*y + z*z);
        if (mag < 1e-12) return G4ThreeVector(0, 0, 1);
        return G4ThreeVector(x/mag, y/mag, z/mag);
    };

    // ── Fire mu- ─────────────────────────────────────────────────────────────
    fMuonGun->SetParticleMomentumDirection(norm3(mu_px, mu_py, mu_pz));
    fMuonGun->SetParticleEnergy(mu_KE * GeV);
    fMuonGun->GeneratePrimaryVertex(event);

    // ── Fire pi+ from same vertex, same t=0 ──────────────────────────────────
    // Delta lifetime ~5.5e-24 s → both particles are co-primary at t=0
    fPionGun->SetParticleMomentumDirection(norm3(pi_px, pi_py, pi_pz));
    fPionGun->SetParticleEnergy(pi_KE * GeV);
    fPionGun->GeneratePrimaryVertex(event);

    // ── Store initial state in EventSummary ───────────────────────────────────
    auto& sum = fEventAction->GetSummary();
    sum.event_id       = fEventCount;
    sum.Enu            = Enu;
    sum.cos_open       = cos_open;
    sum.open_angle_deg = open_angle_deg;
    sum.vtx_x          = vtx_x;
    sum.vtx_y          = vtx_y;
    sum.vtx_z          = vtx_z;

    sum.mu_px0 = mu_px;  sum.mu_py0 = mu_py;  sum.mu_pz0 = mu_pz;
    sum.mu_p0  = std::sqrt(mu_px*mu_px + mu_py*mu_py + mu_pz*mu_pz);
    sum.mu_KE0 = mu_KE;

    sum.pi_px0 = pi_px;  sum.pi_py0 = pi_py;  sum.pi_pz0 = pi_pz;
    sum.pi_p0  = std::sqrt(pi_px*pi_px + pi_py*pi_py + pi_pz*pi_pz);
    sum.pi_KE0 = pi_KE;

    fEventAction->RecordEvent(sum);
    ++fEventCount;
}
