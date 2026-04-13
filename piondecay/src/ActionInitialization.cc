#include "ActionInitialization.hh"
#include "PrimaryGeneratorAction.hh"
#include "RunAction.hh"
#include "EventAction.hh"
#include "TrackingAction.hh"
#include "SteppingAction.hh"

void ActionInitialization::BuildForMaster() const
{
    SetUserAction(new RunAction());
}

void ActionInitialization::Build() const
{
    RunAction*     runAction     = new RunAction();
    EventAction*   eventAction   = new EventAction(runAction);
    SteppingAction* steppingAction = new SteppingAction(eventAction);

    // Wire SteppingAction back to EventAction so BeginOfEvent can clear it
    eventAction->SetSteppingAction(steppingAction);

    SetUserAction(new PrimaryGeneratorAction(eventAction));
    SetUserAction(runAction);
    SetUserAction(eventAction);
    SetUserAction(new TrackingAction(eventAction));
    SetUserAction(steppingAction);
}
