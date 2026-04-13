#ifndef PrimaryGeneratorAction_h
#define PrimaryGeneratorAction_h

#include "G4VUserPrimaryGeneratorAction.hh"
#include "G4ParticleGun.hh"
#include "CoPrimaryReader.hh"

class EventAction;

class PrimaryGeneratorAction : public G4VUserPrimaryGeneratorAction
{
public:
    PrimaryGeneratorAction(EventAction* eventAction);
    ~PrimaryGeneratorAction() override;

    void GeneratePrimaries(G4Event*) override;

private:
    G4ParticleGun*   fMuonGun      = nullptr;   // fires mu-
    G4ParticleGun*   fPionGun      = nullptr;   // fires pi+
    CoPrimaryReader* fCoPrimReader = nullptr;
    EventAction*     fEventAction  = nullptr;
    int              fEventCount   = 0;
};

#endif
