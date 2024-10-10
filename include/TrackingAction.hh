#ifndef TrackingAction_h
#define TrackingAction_h 1

#include "G4UserTrackingAction.hh"
#include "globals.hh"


namespace G4Sim {

class EventAction; // Forward declare EventAction within the G4Sim namespace

class TrackingAction : public G4UserTrackingAction
{
public:
    TrackingAction(EventAction* eventAction);
    virtual ~TrackingAction();

    virtual void PreUserTrackingAction(const G4Track* track);

private:
    EventAction* fEventAction;
    
};

} // namespace G4Sim

#endif