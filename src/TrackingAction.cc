#include "TrackingAction.hh"
#include "EventAction.hh"
#include "G4Track.hh"
#include "G4Gamma.hh"
#include "G4VProcess.hh"

namespace G4Sim {

TrackingAction::TrackingAction(EventAction* eventAction)
    : G4UserTrackingAction(), fEventAction(eventAction) {}

TrackingAction::~TrackingAction() {}

void TrackingAction::PreUserTrackingAction(const G4Track* track)
{
    // Check if the particle is a gamma
    if (track->GetDefinition() == G4Gamma::Gamma())
    {
        // Get the creator process
        const G4VProcess* creatorProcess = track->GetCreatorProcess();
        if (creatorProcess)
        {
            G4String processName = creatorProcess->GetProcessName();
            
            // Check if the process is bremsstrahlung
            if (processName == "eBrem")
            {
                G4VPhysicalVolume* volume = track->GetVolume();
                if (volume)
                {
                    G4String volumeName = volume->GetName();
                    if (volumeName == "LiquidXenon" || volumeName == "LXeFiducial")
                    {
                        fEventAction->AddEventType(BREM_GAMMA);
                        G4int trackID = track->GetTrackID();
                        fEventAction->AddBremsGammaToTrack(trackID);
                        
                    }
                }
                
            }
        }
    }
}

} // namespace G4Sim