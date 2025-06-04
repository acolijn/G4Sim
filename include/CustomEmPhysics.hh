#ifndef CUSTOMEMPHYSICS_HH
#define CUSTOMEMPHYSICS_HH

#include "G4VPhysicsConstructor.hh"
#include "G4EmLivermorePhysics.hh"
#include "G4ParticleDefinition.hh"
#include "PhysicsMessenger.hh"

class CustomEmPhysics : public G4VPhysicsConstructor  {
public:
    CustomEmPhysics(PhysicsMessenger* messenger, const G4String& name = "CustomPhysics");
    virtual ~CustomEmPhysics();

    virtual void ConstructParticle() override;
    virtual void ConstructProcess() override;

private:
    PhysicsMessenger* fMessenger;

    void RemoveRayleighScattering();
    void RemoveBremsstrahlung(G4ParticleDefinition* particle);
    void RemovePairProduction();
    void RemoveNeutronElastic();
    void RemoveNeutronInelastic();
    void RemoveNeutronCapture();
};

#endif