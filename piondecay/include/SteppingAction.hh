#ifndef SteppingAction_hh
#define SteppingAction_hh

#include "G4UserSteppingAction.hh"
#include "G4Step.hh"
#include "CherenkovAnalyzer.hh"
#include <map>
#include <string>

class EventAction;

class SteppingAction : public G4UserSteppingAction
{
public:
    explicit SteppingAction(EventAction* eventAction);
    ~SteppingAction() override = default;

    void UserSteppingAction(const G4Step*) override;

    // Called by EventAction::BeginOfEventAction each event
    void ClearEvent() {
        fCherCumul.clear();
        fTrackLen.clear();
        fFromPrimaryPi.clear();
        fStepCount.clear();
        fTrackParticle.clear();
    }

    // Called by EventAction::RecordStep to look up parent particle name
    // Returns "" if track_id not seen yet (e.g. neutrinos that leave no steps)
    const std::string& GetParticleName(int track_id) const {
        static const std::string empty = "";
        auto it = fTrackParticle.find(track_id);
        return (it != fTrackParticle.end()) ? it->second : empty;
    }

private:
    EventAction*      fEventAction;
    CherenkovAnalyzer fCher;

    // Per-track accumulators (reset each event via ClearEvent)
    std::map<int, bool> fFromPrimaryPi;  // track_id → descended from primary pi
    std::map<int, double>      fCherCumul;      // track_id → cumulative Cherenkov
    std::map<int, double>      fTrackLen;        // track_id → cumulative length [mm]
    std::map<int, int>         fStepCount;       // track_id → step count
    std::map<int, std::string> fTrackParticle;   // track_id → particle name
                                                 // KEY: lets EventAction look up
                                                 // what particle owns parent_id
                                                 // to distinguish pi→e from mu→e
};

#endif
