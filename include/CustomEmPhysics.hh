#ifndef CUSTOMEMPHYSICS_HH
#define CUSTOMEMPHYSICS_HH

#include "G4EmLivermorePhysics.hh"
#include "G4ParticleDefinition.hh"
#include "PhysicsMessenger.hh"

class CustomEmPhysics : public G4EmLivermorePhysics {
public:
    CustomEmPhysics(PhysicsMessenger* messenger);
    virtual ~CustomEmPhysics();

    void ConstructProcess() override;

private:
    PhysicsMessenger* fMessenger;

    void RemoveRayleighScattering();
    void RemoveBremsstrahlung(G4ParticleDefinition* particle);
    void RemovePairProduction();
};

#endif