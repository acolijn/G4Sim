#include "CustomEmPhysics.hh"

// For processes
#include "G4ProcessManager.hh"
#include "G4ParticleDefinition.hh"

// For EM
#include "G4EmLivermorePhysics.hh"
#include "G4RayleighScattering.hh"
#include "G4GammaConversion.hh"
#include "G4Gamma.hh"
#include "G4Electron.hh"
#include "G4Positron.hh"

// For hadronic
#include "G4Neutron.hh"
#include "G4HadronElasticPhysics.hh"
#include "G4HadronPhysicsFTFP_BERT_HP.hh"
#include "G4NeutronTrackingCut.hh"

#include "G4VProcess.hh"
#include "PhysicsMessenger.hh"

CustomEmPhysics::CustomEmPhysics(PhysicsMessenger* messenger, const G4String& name)
 : G4VPhysicsConstructor(name), fMessenger(messenger)
{}

CustomEmPhysics::~CustomEmPhysics() {}

void CustomEmPhysics::ConstructParticle()
{
    // Typically the reference list or the hadronic/EM builders define the particles themselves.
    // You could call G4EmLivermorePhysics().ConstructParticle() here if needed.
}

/**
 * Constructs the physics processes for the custom electromagnetic physics list.
    * This method is called by the base class method to construct the standard processes.
    * It then removes the Rayleigh scattering, pair production, or bremsstrahlung processes
    * based on the settings provided by the messenger.
    */
void CustomEmPhysics::ConstructProcess() {
    //1) Add EM processes from Livermore
    G4EmLivermorePhysics emLivermore;
    emLivermore.ConstructProcess(); // Adds Rayleigh, eBrem, etc.

    if (!fMessenger->IsRayleighEnabled()) {
        G4cout << "CustomEmPhysics::ConstructProcess: Removing Rayleigh scattering" << G4endl;
        RemoveRayleighScattering();
    }
    if (!fMessenger->IsPairEnabled()) {
        G4cout << "CustomEmPhysics::ConstructProcess: Removing pair production" << G4endl;
        RemovePairProduction();
    }
    if (!fMessenger->IsBremEnabled()) {
        G4cout << "CustomEmPhysics::ConstructProcess: Removing bremsstrahlung" << G4endl;
        RemoveBremsstrahlung(G4Electron::Electron());
        RemoveBremsstrahlung(G4Positron::Positron());
    }
    if (!fMessenger->IsNeutronElasticEnabled()) {
        G4cout << "CustomEmPhysics::ConstructProcess: Removing neutron elastic" << G4endl;
        RemoveNeutronElastic();
    }
    if (!fMessenger->IsNeutronInelasticEnabled()) {
        G4cout << "CustomEmPhysics::ConstructProcess: Removing neutron inelastic" << G4endl;
        RemoveNeutronInelastic();
    }
    if (!fMessenger->IsNeutronCaptureEnabled()) {
        G4cout << "CustomEmPhysics::ConstructProcess: Removing neutron capture" << G4endl;
        RemoveNeutronCapture();
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


void CustomEmPhysics::RemoveNeutronElastic() {
    G4ProcessManager* pManager = G4Neutron::Neutron()->GetProcessManager();

    if (!pManager) {
        G4cout << "CustomEmPhysics::RemoveNeutronElastic: No process manager for neutron" << G4endl;
        return;
    }

    G4int nProcesses = pManager->GetProcessListLength();
    for (G4int i = 0; i < nProcesses; ++i) {
        G4VProcess* process = (*pManager->GetProcessList())[i];
        if (process && process->GetProcessName() == "hadElastic") {
            G4cout << "CustomEmPhysics::RemoveNeutronElastic: Removing hadElastic" << G4endl;
            pManager->RemoveProcess(process);
            break;
        }
    }
}

void CustomEmPhysics::RemoveNeutronInelastic() {
    G4ProcessManager* pManager = G4Neutron::Neutron()->GetProcessManager();

    if (!pManager) {
        G4cout << "CustomEmPhysics::RemoveNeutronInelastic: No process manager for neutron" << G4endl;
        return;
    }
    
    G4int nProcesses = pManager->GetProcessListLength();
    for (G4int i = 0; i < nProcesses; ++i) {
        G4VProcess* process = (*pManager->GetProcessList())[i];
        if (process && process->GetProcessName() == "neutronInelastic") {
            G4cout << "CustomEmPhysics::RemoveNeutronInelastic: Removing neutronInelastic" << G4endl;
            pManager->RemoveProcess(process);
            break;
        }
    }
}

void CustomEmPhysics::RemoveNeutronCapture() {
    G4ProcessManager* pManager = G4Neutron::Neutron()->GetProcessManager();

    if (!pManager) {
        G4cout << "CustomEmPhysics::RemoveNeutronCapture: No process manager for neutron" << G4endl;
        return;
    }

    G4int nProcesses = pManager->GetProcessListLength();
    for (G4int i = 0; i < nProcesses; ++i) {
        G4VProcess* process = (*pManager->GetProcessList())[i];
        if (process && process->GetProcessName() == "nCapture") {
            G4cout << "CustomEmPhysics::RemoveNeutronCapture: Removing nCapture" << G4endl;
            pManager->RemoveProcess(process);
            break;
        }
    }
}

