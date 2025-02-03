#include "PhysicsMessenger.hh"
#include "G4UIcmdWithABool.hh"
#include "G4UIdirectory.hh"
#include "G4UIcommand.hh"
#include "G4SystemOfUnits.hh"
#include "G4ios.hh"

PhysicsMessenger::PhysicsMessenger()
 : G4UImessenger(),
   fBremCmd(nullptr),
   fPairCmd(nullptr),
   fRayCmd(nullptr),
   fBremEnabled(true),
   fPairEnabled(true),
   fRayleighEnabled(true)
{
    // You can optionally define a directory, e.g. /physics/
    // G4UIdirectory* physDir = new G4UIdirectory("/physics/");
    // physDir->SetGuidance("Physics toggles directory");

    fBremCmd = new G4UIcmdWithABool("/physics/setBremEnabled", this);
    fBremCmd->SetGuidance("Enable or disable e- bremsstrahlung processes (and e+).");
    fBremCmd->SetParameterName("BremEnabled", /*omittable=*/ false);

    fPairCmd = new G4UIcmdWithABool("/physics/setPairEnabled", this);
    fPairCmd->SetGuidance("Enable or disable pair production (gamma conversion).");
    fPairCmd->SetParameterName("PairEnabled", false);

    fRayCmd = new G4UIcmdWithABool("/physics/setRayleighEnabled", this);
    fRayCmd->SetGuidance("Enable or disable Rayleigh scattering for gammas.");
    fRayCmd->SetParameterName("RayleighEnabled", false);
}

PhysicsMessenger::~PhysicsMessenger()
{
    delete fBremCmd;
    delete fPairCmd;
    delete fRayCmd;
    // If you allocated a G4UIdirectory*, also delete it here
}

/**
    * Set the new value for the physics toggles.
    * @param command The command that was invoked.
    * @param newValue The new value for the command.
     */
void PhysicsMessenger::SetNewValue(G4UIcommand* command, G4String newValue)
{
    if (command == fBremCmd) {
        fBremEnabled = fBremCmd->GetNewBoolValue(newValue);
        G4cout << "[PhysicsMessenger] bremEnabled -> " << (fBremEnabled ? "true" : "false") << G4endl;
    }
    else if (command == fPairCmd) {
        fPairEnabled = fPairCmd->GetNewBoolValue(newValue);
        G4cout << "[PhysicsMessenger] pairEnabled -> " << (fPairEnabled ? "true" : "false") << G4endl;
    }
    else if (command == fRayCmd) {
        fRayleighEnabled = fRayCmd->GetNewBoolValue(newValue);
        G4cout << "[PhysicsMessenger] rayleighEnabled -> " << (fRayleighEnabled ? "true" : "false") << G4endl;
    }
}