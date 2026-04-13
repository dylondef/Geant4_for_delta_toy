#ifndef TrackingAction_h
#define TrackingAction_h

#include "G4UserTrackingAction.hh"
#include "G4Track.hh"

class EventAction;

class TrackingAction : public G4UserTrackingAction
{
public:
    TrackingAction(EventAction* eventAction);
    ~TrackingAction() override = default;

    void PreUserTrackingAction (const G4Track*) override;
    void PostUserTrackingAction(const G4Track*) override;

private:
    EventAction* fEventAction;
};

#endif
