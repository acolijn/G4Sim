#ifndef PHYSICSMESSENGER_HH
#define PHYSICSMESSENGER_HH

#include "G4UImessenger.hh"
#include "globals.hh"

// Forward declare if needed
class G4UIcmdWithABool;

class PhysicsMessenger : public G4UImessenger {
public:
    PhysicsMessenger();
    virtual ~PhysicsMessenger();

    void SetNewValue(G4UIcommand* command, G4String newValue) override;

    // Accessor methods used after macro is parsed:
    G4bool IsBremEnabled() const       { return fBremEnabled; }
    G4bool IsPairEnabled() const       { return fPairEnabled; }
    G4bool IsRayleighEnabled() const   { return fRayleighEnabled; }
    G4bool IsNeutronElasticEnabled() const { return fNeutronElasticEnabled; }
    G4bool IsNeutronInelasticEnabled() const { return fNeutronInelasticEnabled; }
    G4bool IsNeutronCaptureEnabled() const { return fNeutronCaptureEnabled; }

private:
    // UI command pointers
    G4UIcmdWithABool* fBremCmd;
    G4UIcmdWithABool* fPairCmd;
    G4UIcmdWithABool* fRayCmd;
    G4UIcmdWithABool* fNeutronElasticCmd;
    G4UIcmdWithABool* fNeutronInelasticCmd;
    G4UIcmdWithABool* fNeutronCaptureCmd;

    // Internal booleans (default true = "all processes enabled")
    G4bool fBremEnabled;
    G4bool fPairEnabled;
    G4bool fRayleighEnabled;
    G4bool fNeutronElasticEnabled;
    G4bool fNeutronInelasticEnabled;
    G4bool fNeutronCaptureEnabled;

};

#endif