#ifndef RunAction_hh
#define RunAction_hh

#include "G4UserRunAction.hh"
#include "EventData.hh"

class TFile;
class TTree;

class RunAction : public G4UserRunAction
{
public:
    RunAction();
    ~RunAction() override;

    void BeginOfRunAction(const G4Run*) override;
    void EndOfRunAction  (const G4Run*) override;

    void FillEventTree(const EventSummary& ev);
    void FillStepTree (const StepData& s);

private:
    TFile* fRootFile  = nullptr;
    TTree* fEventTree = nullptr;
    TTree* fStepTree  = nullptr;

    // EventTree buffer — one EventSummary per event
    // Branch addresses point directly into this struct
    EventSummary fEvBuf;

    // StepTree flat scalar buffers — ROOT branches point here
    int    fS_event_id    = 0;
    int    fS_track_id    = 0;
    int    fS_parent_id   = 0;
    int    fS_step_num    = 0;
    char   fS_particle[16]= {};
    bool fS_from_primary_pi = false;
    double fS_x           = 0;
    double fS_y           = 0;
    double fS_z           = 0;
    double fS_px          = 0;
    double fS_py          = 0;
    double fS_pz          = 0;
    double fS_p_mag       = 0;
    double fS_KE          = 0;
    double fS_edep        = 0;
    double fS_step_len    = 0;
    double fS_global_time = 0;
    double fS_beta        = 0;
    double fS_gamma       = 0;
    double fS_cher_step   = 0;
    double fS_cher_cumul  = 0;
    double fS_track_len   = 0;
    char   fS_process[32] = {};
};

#endif
