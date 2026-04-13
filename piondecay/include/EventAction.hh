#ifndef EventAction_h
#define EventAction_h

#include "G4UserEventAction.hh"
#include "EventData.hh"

class RunAction;
class SteppingAction;

class EventAction : public G4UserEventAction
{
public:
    EventAction(RunAction* runAction);
    ~EventAction() override = default;

    void BeginOfEventAction(const G4Event*) override;
    void EndOfEventAction  (const G4Event*) override;

    // Called by SteppingAction / TrackingAction
    void RecordStep(const StepData& s);
    void RecordEvent(const EventSummary& ev) { fSummary = ev; fHasSummary = true; }

    EventSummary& GetSummary() { return fSummary; }

    // Set by ActionInitialization so BeginOfEvent can clear stepping accumulators
    void SetSteppingAction(SteppingAction* sa) { fSteppingAction = sa; }

private:
    RunAction*               fRunAction;
    SteppingAction*          fSteppingAction = nullptr;
    std::vector<StepData>    fSteps;
    EventSummary             fSummary;
    bool                     fHasSummary     = false;
};

#endif
