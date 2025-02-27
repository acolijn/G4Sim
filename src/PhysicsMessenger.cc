#include "PhysicsMessenger.hh"
#include "G4UIcmdWithABool.hh"
#include "G4UIdirectory.hh"
#include "G4UIcommand.hh"
#include "G4SystemOfUnits.hh"
#include "G4ios.hh"

/**
 * @class PhysicsMessenger
 * @brief A messenger class to handle user commands for toggling physics processes.
 *
 * The PhysicsMessenger class listens for commands that allow users to enable or disable
 * specific physics processes in a Geant4 simulation. It provides commands for
 * bremsstrahlung, pair production, and Rayleigh scattering.
 */

PhysicsMessenger::PhysicsMessenger()
 : G4UImessenger(),
   fBremCmd(nullptr),
   fPairCmd(nullptr),
   fRayCmd(nullptr),
    fNeutronElasticCmd(nullptr),
    fNeutronInelasticCmd(nullptr),
    fNeutronCaptureCmd(nullptr),
   fBremEnabled(true),
   fPairEnabled(true),
    fRayleighEnabled(true),
    fNeutronElasticEnabled(true),
    fNeutronInelasticEnabled(true),
    fNeutronCaptureEnabled(true)
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

    fNeutronElasticCmd = new G4UIcmdWithABool("/physics/setNeutronElasticEnabled", this);
    fNeutronElasticCmd->SetGuidance("Enable or disable neutron elastic scattering.");
    fNeutronElasticCmd->SetParameterName("NeutronElasticEnabled", false);

    fNeutronInelasticCmd = new G4UIcmdWithABool("/physics/setNeutronInelasticEnabled", this);
    fNeutronInelasticCmd->SetGuidance("Enable or disable neutron inelastic scattering.");
    fNeutronInelasticCmd->SetParameterName("NeutronInelasticEnabled", false);

    fNeutronCaptureCmd = new G4UIcmdWithABool("/physics/setNeutronCaptureEnabled", this);
    fNeutronCaptureCmd->SetGuidance("Enable or disable neutron capture.");
    fNeutronCaptureCmd->SetParameterName("NeutronCaptureEnabled", false);
}


/**
 * @brief Destructor for the PhysicsMessenger class.
 *
 * Cleans up the dynamically allocated commands.
 */
PhysicsMessenger::~PhysicsMessenger()
{
    delete fBremCmd;
    delete fPairCmd;
    delete fRayCmd;
    delete fNeutronElasticCmd;
    delete fNeutronInelasticCmd;
    delete fNeutronCaptureCmd;
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

    else if (command == fNeutronElasticCmd) {
        fNeutronElasticEnabled = fNeutronElasticCmd->GetNewBoolValue(newValue);
        G4cout << "[PhysicsMessenger] neutronElasticEnabled -> " << (fNeutronElasticEnabled ? "true" : "false") << G4endl;
    }

    else if (command == fNeutronInelasticCmd) {
        fNeutronInelasticEnabled = fNeutronInelasticCmd->GetNewBoolValue(newValue);
        G4cout << "[PhysicsMessenger] neutronInelasticEnabled -> " << (fNeutronInelasticEnabled ? "true" : "false") << G4endl;
    }

    else if (command == fNeutronCaptureCmd) {
        fNeutronCaptureEnabled = fNeutronCaptureCmd->GetNewBoolValue(newValue);
        G4cout << "[PhysicsMessenger] neutronCaptureEnabled -> " << (fNeutronCaptureEnabled ? "true" : "false") << G4endl;
    }
    else {
        G4cerr << "[PhysicsMessenger] Unknown command: " << command->GetCommandPath() << G4endl;
    }
}