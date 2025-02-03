#include "CustomEmPhysics.hh"
#include "G4ProcessManager.hh"
#include "G4RayleighScattering.hh"
#include "G4Gamma.hh"
#include "G4Electron.hh"
#include "G4Positron.hh"
#include "G4VProcess.hh"
#include "G4GammaConversion.hh"
#include "PhysicsMessenger.hh"

CustomEmPhysics::CustomEmPhysics(PhysicsMessenger* messenger)
 : G4EmLivermorePhysics(), fMessenger(messenger)
{}

CustomEmPhysics::~CustomEmPhysics() {}

/**
 * Constructs the physics processes for the custom electromagnetic physics list.
    * This method is called by the base class method to construct the standard processes.
    * It then removes the Rayleigh scattering, pair production, or bremsstrahlung processes
    * based on the settings provided by the messenger.
    */
void CustomEmPhysics::ConstructProcess() {
    // Call the base class method to construct the standard processes
    G4EmLivermorePhysics::ConstructProcess();

    if (!fMessenger->IsRayleighEnabled()) {
        RemoveRayleighScattering();
    }
    if (!fMessenger->IsPairEnabled()) {
        G4cout << "CustomEmPhysics::ConstructProcess: Removing pair production" << G4endl;
        RemovePairProduction();
    }
    if (!fMessenger->IsBremEnabled()) {
        RemoveBremsstrahlung(G4Electron::Electron());
        RemoveBremsstrahlung(G4Positron::Positron());
    }
}

void CustomEmPhysics::RemoveRayleighScattering() {
    G4ProcessManager* pManager = G4Gamma::Gamma()->GetProcessManager();

    if (!pManager) return;

    // Find and remove Rayleigh scattering
    G4RayleighScattering* rayleighScattering = nullptr;
    G4int nProcesses = pManager->GetProcessListLength();
    for (G4int i = 0; i < nProcesses; ++i) {
        G4VProcess* process = (*pManager->GetProcessList())[i];
        if ((rayleighScattering = dynamic_cast<G4RayleighScattering*>(process)) != nullptr) {
            G4cout << "CustomEmPhysics::ConstructProcess: Removing Rayleigh scattering" << G4endl;
            pManager->RemoveProcess(rayleighScattering);
            break;
        }
    }
}

void CustomEmPhysics::RemoveBremsstrahlung(G4ParticleDefinition* particle) {
    G4ProcessManager* pManager = particle->GetProcessManager();

    if (!pManager) {
        G4cout << "CustomEmPhysics::RemoveBremsstrahlung: No process manager for " 
               << particle->GetParticleName() << G4endl;
        return;
    }

    // Find and remove the bremsstrahlung process
    G4int nProcesses = pManager->GetProcessListLength();
    for (G4int i = 0; i < nProcesses; ++i) {
        G4VProcess* process = (*pManager->GetProcessList())[i];
        if (process && process->GetProcessName() == "eBrem") {
            G4cout << "CustomEmPhysics::RemoveBremsstrahlung: Removing eBrem from "
                   << particle->GetParticleName() << G4endl;
            pManager->RemoveProcess(process);
            break;
        }
    }
}

void CustomEmPhysics::RemovePairProduction() {
    G4ProcessManager* pManager = G4Gamma::Gamma()->GetProcessManager();

    if (!pManager) {
        G4cout << "CustomEmPhysics::RemovePairProduction: No process manager for gamma" << G4endl;
        return;
    }

    // Find and remove the pair production process
    G4GammaConversion* pairProduction = nullptr;
    G4int nProcesses = pManager->GetProcessListLength();
    for (G4int i = 0; i < nProcesses; ++i) {
        G4VProcess* process = (*pManager->GetProcessList())[i];
        if ((pairProduction = dynamic_cast<G4GammaConversion*>(process)) != nullptr) {
            G4cout << "CustomEmPhysics::RemovePairProduction: Removing pair production" << G4endl;
            pManager->RemoveProcess(pairProduction);
            break;
        }
    }
}