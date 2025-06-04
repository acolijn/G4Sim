#include "NeutronHelper.hh"
#include "Randomize.hh"
#include "G4Neutron.hh"

#include "G4ProductionCutsTable.hh"
#include "G4AutoLock.hh"
#include "G4DynamicParticle.hh"
#include "G4MaterialCutsCouple.hh"
#include "G4NeutronElasticXS.hh"
#include "G4NeutronInelasticXS.hh"
#include "G4NeutronCaptureXS.hh"
#include "G4HadronElastic.hh"
#include "GammaRayHelper.hh"
#include "G4NeutronHPInelastic.hh"
#include "G4NeutronHPCapture.hh"
#include "G4AnalysisManager.hh"
#include <thread>
#include <vector>
#include "Randomize.hh"

#include "G4HadronicProcessStore.hh"

#include <fstream>
#include <sstream>
#include <iostream>
#include <string>
#include <cmath>

#include <G4NeutronHPElastic.hh>
#include <G4ParticleHPElasticData.hh>
#include <G4ParticleHPInelasticData.hh>
#include <G4NeutronHPCaptureData.hh>
#include <G4NeutronHPFissionData.hh>
#include <G4NeutronHPFission.hh>

#include "CustomNeutronHPElastic.hh"
// Declare a global mutex for thread safety
namespace {
    G4Mutex mutex = G4MUTEX_INITIALIZER;
}

namespace G4Sim {

NeutronHelper& NeutronHelper::Instance() {
    static thread_local NeutronHelper instance;
    return instance;
}

NeutronHelper::NeutronHelper() : customHPElastic(nullptr), elasticModelHP(nullptr), fissionHP(nullptr), fissionModel(nullptr), captureModel(nullptr),elasticXS(nullptr),elasticHP(nullptr) ,inelasticXS(nullptr),inelasticHP(nullptr), captureXS(nullptr),captureHP(nullptr), elasticModel(nullptr),inelasticModel(nullptr) , cdfsInitialized(false) {}

/**
 * Initialize neutron interaction models.
 */
void NeutronHelper::Initialize() {

    G4cout << "NeutronHelper::Initializing Elastic, Inelastic, and Capture models " << G4endl;

    elasticXS = new G4NeutronElasticXS();
    elasticHP = new G4ParticleHPElasticData();
    elasticHP->BuildPhysicsTable(*G4Neutron::Neutron());

    inelasticXS = new G4NeutronInelasticXS();
    inelasticHP = new G4ParticleHPInelasticData();
    inelasticHP->BuildPhysicsTable(*G4Neutron::Neutron());

    captureXS = new G4NeutronCaptureXS();
    captureHP = new G4NeutronHPCaptureData();
    captureHP->BuildPhysicsTable(*G4Neutron::Neutron());

    fissionHP = new G4NeutronHPFissionData();
    fissionHP->BuildPhysicsTable(*G4Neutron::Neutron());

    elasticModel = new G4HadronElastic();
    elasticModel->BuildPhysicsTable(*G4Neutron::Neutron());

    elasticModelHP = new G4NeutronHPElastic();
    elasticModelHP->BuildPhysicsTable(*G4Neutron::Neutron());
    
    customHPElastic = new CustomNeutronHPElastic();
    customHPElastic->BuildPhysicsTable(*G4Neutron::Neutron());


    inelasticModel = new G4NeutronHPInelastic();
    inelasticModel->BuildPhysicsTable(*G4Neutron::Neutron());

    captureModel = new G4NeutronHPCapture();
    captureModel->BuildPhysicsTable(*G4Neutron::Neutron());

    fissionModel = new G4NeutronHPFission();
    fissionModel->BuildPhysicsTable(*G4Neutron::Neutron());
}

/**
 * @brief Initializes the cumulative distribution functions (CDFs) for neutron scattering.
 */
void NeutronHelper::InitializeCDFs(G4double energy) {
    fInitialEnergy = energy;

    if (!cdfsInitialized) {
        Initialize();
        fElementsUsed.clear();
        G4MaterialTable* materialTable = G4Material::GetMaterialTable();
        for (size_t i = 0; i < materialTable->size(); ++i) {
            G4Material* material = (*materialTable)[i];
            const G4ElementVector* elementVector = material->GetElementVector();
            for (size_t j = 0; j < material->GetNumberOfElements(); ++j) {
                const G4Element* elm = (*elementVector)[j];
                if (std::find(fElementsUsed.begin(), fElementsUsed.end(), elm->GetName()) == fElementsUsed.end()) {
                    fElementsUsed.push_back(elm->GetName());
                    cdfDataMap[elm] = CreateCDF(elm, energy,material);
                }
            }
        }
        cdfsInitialized = true;
    }
}

/**
 * Create a cumulative distribution function (CDF) for neutron elastic scattering.
 */
CDFData NeutronHelper::CreateCDF(const G4Element* element, G4double energy, const G4Material* material) {
    G4int nPoints = 10000;
    G4double cosThetaMin = -1.0;
    G4double cosThetaMax =  1.0;
    G4double dCosTheta = (cosThetaMax - cosThetaMin) / nPoints;

    std::vector<G4double> cdf(nPoints + 1);
    std::vector<G4double> cosTheta(nPoints + 1);

    G4double sum = 0.0;
    for (G4int i = 0; i <= nPoints; ++i) {
        G4double cosThetaVal = cosThetaMin + i * dCosTheta;
        cosTheta[i] = cosThetaVal;

        G4DynamicParticle neutron(G4Neutron::Neutron(), G4ThreeVector(1,0,0), energy);
        
        // Use direct object instead of pointer for GetElementCrossSection()
        G4double diffCrossSection = elasticXS->GetElementCrossSection(&neutron, element->GetZ(), material) * dCosTheta;

        sum += diffCrossSection;
        cdf[i] = sum;
    }

    for (G4int i = 0; i <= nPoints; ++i) {
        cdf[i] /= sum;
    }

    CDFData cdfData;
    cdfData.cdf = cdf;
    cdfData.cosTheta = cosTheta;

    return cdfData;
}


/**
 * Get the elastic scattering cross-section for a neutron in a material.
 */
G4double NeutronHelper::GetElasticCrossSection(G4double energy, G4Material* material) {
    G4double crossSection = 0.0;
    const G4ElementVector* elementVector = material->GetElementVector();
    const G4double* fractionVector = material->GetFractionVector();

    for (size_t i = 0; i < material->GetNumberOfElements(); ++i) {
        const G4Element* element = (*elementVector)[i];

        
        
        crossSection += fractionVector[i] * elasticHP->ComputeCrossSection(
            new G4DynamicParticle(G4Neutron::Neutron(), G4ThreeVector(1,0,0), energy),
            element,
            material
        );
    }

    
    return crossSection;
}

/**
 * Get the inelastic scattering cross-section for a neutron in a material.
 */
G4double NeutronHelper::GetInelasticCrossSection(G4double energy, G4Material* material) {
    G4double crossSection = 0.0;
    const G4ElementVector* elementVector = material->GetElementVector();
    const G4double* fractionVector = material->GetFractionVector();

    for (size_t i = 0; i < material->GetNumberOfElements(); ++i) {
        const G4Element* element = (*elementVector)[i];

        crossSection += fractionVector[i] * inelasticHP->ComputeCrossSection(
            new G4DynamicParticle(G4Neutron::Neutron(), G4ThreeVector(1,0,0), energy),
            element,
            material
        );
    }

    return crossSection;
}
/**
 * Get the neutron capture cross-section.
 */
G4double NeutronHelper::GetCaptureCrossSection(G4double energy, G4Material* material) {
    G4double crossSection = 0.0;
    const G4ElementVector* elementVector = material->GetElementVector();
    const G4double* fractionVector = material->GetFractionVector();

    for (size_t i = 0; i < material->GetNumberOfElements(); ++i) {
        const G4Element* element = (*elementVector)[i];

        crossSection += fractionVector[i] * captureHP->ComputeCrossSection(
            new G4DynamicParticle(G4Neutron::Neutron(), G4ThreeVector(1,0,0), energy),
            element,
            material
        );
    }

    return crossSection;
}

G4double NeutronHelper::GetFissionCrossSection(G4double energy, G4Material* material) {
    G4double crossSection = 0.0;
    const G4ElementVector* elementVector = material->GetElementVector();
    const G4double* fractionVector = material->GetFractionVector();

    for (size_t i = 0; i < material->GetNumberOfElements(); ++i) {
        const G4Element* element = (*elementVector)[i];

        crossSection += fractionVector[i] * fissionHP->ComputeCrossSection(
            new G4DynamicParticle(G4Neutron::Neutron(), G4ThreeVector(1,0,0), energy),
            element,
            material
        );
    }

    return crossSection;
}

/**
 * Get the total neutron interaction cross-section.
 */
G4double NeutronHelper::GetTotalCrossSection(G4double energy, G4Material* material) {
    G4double crossSection = GetElasticCrossSection(energy, material) 
                            + GetInelasticCrossSection(energy, material) 
                            + GetCaptureCrossSection(energy, material)
                            + GetFissionCrossSection(energy, material);
    
    return crossSection;
}


/**
 * Get the neutron attenuation length for a given energy and material.
 */
G4double NeutronHelper::GetAttenuationLength(G4double energy, G4Material* material) {
    const G4ElementVector* elementVector = material->GetElementVector();
    const G4double* fractionVector = material->GetFractionVector();

    G4double mass_attenuation = 0.0;
    auto* neutron = new G4DynamicParticle(G4Neutron::Neutron(), G4ThreeVector(1,0,0), energy);
    for (size_t i = 0; i < material->GetNumberOfElements(); ++i) {
        const G4Element* element = (*elementVector)[i];

        // Get neutron cross-sections per ATOM (should be in cm²)
        G4double elasticcross = elasticHP->ComputeCrossSection(
            neutron,
            element,
            material
        );

        G4double inelasticcross = inelasticHP->ComputeCrossSection(
            neutron,
            element,
            material
        );
        //G4cout << "inelasticcross cross section: " << inelasticcross << G4endl;
        G4double capturecross = captureHP->ComputeCrossSection(
            neutron,
            element,
            material
        );

        G4double fissioncross = fissionHP->ComputeCrossSection(
            neutron,
            element,
            material
        );
        //G4cout << "capturecross cross section: " << capturecross << G4endl;
        G4double totalXS = elasticcross + inelasticcross + capturecross + fissioncross;

        // G4cout << "Element: " << element << G4endl;
        // G4cout << "energy: " << energy << G4endl;
        // G4cout << "Elastic xsec in cm2: " << elasticcross / CLHEP::cm2 << G4endl;
        // G4cout << "Total cross section fast sim cm2: " << totalXS / CLHEP::cm2 << G4endl;

        // Use atomic mass for conversion
        G4double A = element->GetA();  // Atomic mass in g/mol (same as GammaRayHelper)

        mass_attenuation += fractionVector[i] * totalXS * Avogadro / A;

    }
    delete neutron;
    // G4cout << "Mass attenuation: "<< mass_attenuation << G4endl;

    G4double rho = material->GetDensity(); // Material density in g/cm³

    // G4cout << "Density: " << rho << G4endl;

    G4double attenuationLength = 1 / (rho * mass_attenuation);

    // G4cout << "Attenuation length cm : " << attenuationLength << G4endl;

    return attenuationLength;
}

G4ThreeVector NeutronHelper::SampleScatteredDirectionFromEnergies(const G4ThreeVector& initialDir,
                                                                  G4double E0,
                                                                  G4double E1,
                                                                  G4double A) {
    // Compute cos(theta) from recoil formula
    G4double cosTheta_lab = 1.0 - ((E0 - E1) * std::pow(1.0 + A, 2)) / (2.0 * A * E0);

    // Clamp to physical range
    cosTheta_lab = std::max(-1.0, std::min(1.0, cosTheta_lab));
    G4double sinTheta = std::sqrt(1.0 - cosTheta_lab * cosTheta_lab);

    // Sample random azimuthal angle
    G4double phi = 2.0 * CLHEP::pi * G4UniformRand();
    G4double cosPhi = std::cos(phi);
    G4double sinPhi = std::sin(phi);

    // Local direction in z-frame
    G4ThreeVector localDir(sinTheta * cosPhi,
                           sinTheta * sinPhi,
                           cosTheta_lab);

    // Build orthonormal basis around initialDir
    G4ThreeVector z = initialDir.unit();
    G4ThreeVector x = z.orthogonal().unit();            // guaranteed to be non-parallel
    G4ThreeVector y = z.cross(x);                       // make it orthonormal

    G4RotationMatrix rotation;
    rotation.rotateAxes(x, y, z);  // right-handed basis

    return rotation * localDir;
}
/**
 * Sample a neutron scattering angle using the elastic CDF.
 */
InteractionData NeutronHelper::DoElasticNeutronScatter(const G4Step* step, G4ThreeVector x0) {
    G4double energy0 = step->GetPreStepPoint()->GetKineticEnergy(); 
 
    // Get material and element (fixing the const cast issue) 
    const G4Material* material = step->GetPreStepPoint()->GetMaterial(); 
    const G4Element* element = material->GetElement(0);  // Keep as const 
    G4int Z = element->GetZ(); 
    G4int A = element->GetN(); 
 
    // Create a dynamic particle for the neutron
    G4DynamicParticle* neutronDynamic = new G4DynamicParticle(
        G4Neutron::Neutron(), 
        step->GetPreStepPoint()->GetMomentumDirection(), 
        energy0
    ); 

    // Create a track with the dynamic particle and material
    G4Track* neutronTrack = new G4Track(
        neutronDynamic,
        0.0,  // Global time
        x0
    );

    neutronTrack->SetStep(step);

    // Convert track to hadronic projectile
    G4HadProjectile projectile(*neutronTrack);
    

    // Create target nucleus 
    G4Nucleus targetNucleus; 
    targetNucleus.SetParameters(A, Z); 
 
    // Apply the elastic scattering model 
    
    //G4HadFinalState* result = customHPElastic->ApplyYourself(projectile, targetNucleus, material); 
    G4HadFinalState* result = elasticModelHP->ApplyYourself(projectile, targetNucleus, material);

    // Extract new neutron energy and direction 
    G4double E1 = result->GetEnergyChange(); 
    
    G4ThreeVector newDirection = result->GetMomentumChange(); 

    // Compute the energy deposit 
    G4double energyDeposited = energy0 - E1; 

    

    G4AnalysisManager* analysisManager = G4AnalysisManager::Instance();
     
    analysisManager->FillH1(3, energyDeposited / keV);

    if (energyDeposited < 0) energyDeposited = 0;  // Prevent negative values 


    G4ThreeVector oldDir = step->GetPreStepPoint()->GetMomentumDirection();
    
    G4ThreeVector scatteredDir_new = SampleScatteredDirectionFromEnergies(oldDir, energy0, E1, A);
    

    // Prepare interaction data C
    InteractionData data; 
    data.x0 = x0; 
    data.dir = scatteredDir_new; 
    data.energy = E1; 
    data.energyDeposited = energyDeposited; 
    data.cosTheta = scatteredDir_new.cosTheta(); 
    data.weight = 1.0; 
    data.hit = new Hit();  // Assuming Hit is defined elsewhere 
    
    neutronTrack->SetTrackStatus(fStopAndKill);
    // Delete the track
    delete neutronTrack;

    
    G4double cosTheta_lab = oldDir.dot(scatteredDir_new);

    analysisManager->FillH2(0, cosTheta_lab, energyDeposited / keV);

    return data; 
}


// dummy model, applyyourself function does not work. Could implement a custom model of the applyyourself function
// look at how elastic scattering is done with the customHPElastic model
InteractionData NeutronHelper::DoNeutronCapture(const G4Step* step, G4ThreeVector x0) { 
    G4double energy0 = step->GetPreStepPoint()->GetKineticEnergy(); 
 
    // Get material and element (fixing the const cast issue) 
    const G4Material* material = step->GetPreStepPoint()->GetMaterial(); 
    const G4Element* element = material->GetElement(0);  // Keep as const 
    G4int Z = element->GetZ(); 
    G4int A = element->GetN(); 
 
    // Create a dynamic particle for the neutron
    G4DynamicParticle* neutronDynamic = new G4DynamicParticle(
        G4Neutron::Neutron(), 
        step->GetPreStepPoint()->GetMomentumDirection(), 
        energy0
    ); 

    // Create a track with the dynamic particle and material
    G4Track* neutronTrack = new G4Track(
        neutronDynamic,
        0.0,  // Global time
        step->GetPreStepPoint()->GetPosition()
    );

    neutronTrack->SetStep(step);

    // Convert track to hadronic projectile
    G4HadProjectile projectile(*neutronTrack);
    

    // Create target nucleus 
    G4Nucleus targetNucleus; 
    targetNucleus.SetParameters(A, Z); 
 
    // Apply the elastic scattering model 
    
    G4HadFinalState* result = customHPElastic->ApplyYourself(projectile, targetNucleus, material); 
 
    // Extract new neutron energy and direction 
    G4double E1 = result->GetEnergyChange(); 

    G4ThreeVector newDirection = result->GetMomentumChange(); 

    // Compute the energy deposit 
    G4double energyDeposited = energy0 - E1; 
 
    if (energyDeposited < 0) energyDeposited = 0;  // Prevent negative values 
 
    // Prepare interaction data 
    InteractionData data; 
    data.x0 = x0; 
    data.dir = newDirection; 
    data.energy = E1; 
    data.energyDeposited = energyDeposited; 
    data.cosTheta = newDirection.cosTheta(); 
    data.weight = 1.0; 
    data.hit = new Hit(); 
 
    return data; 
}

// dummy model, applyyourself function does not work. Could implement a custom model of the applyyourself function
// look at how elastic scattering is done with the customHPElastic model
InteractionData NeutronHelper::DoInelasticNeutronScatter(const G4Step* step, G4ThreeVector x0) 
{
    G4double energy0 = step->GetPreStepPoint()->GetKineticEnergy();

    // Get material and element (not strictly required, but we keep it)
    const G4Material* material = step->GetPreStepPoint()->GetMaterial();
    const G4Element* element = material->GetElement(0);
    G4int Z = element->GetZ();
    G4int A = element->GetN();

    // 1. Create a dynamic particle for the neutron
    G4DynamicParticle neutronDynamic(G4Neutron::Neutron(), step->GetPreStepPoint()->GetMomentumDirection(), energy0);
    G4HadProjectile projectile(neutronDynamic);  // Correct constructor usage


    // Create target nucleus
    G4Nucleus targetNucleus;
    targetNucleus.SetParameters(A, Z);

  
    G4HadFinalState* result = nullptr;

    // G4cout << "DEBUG: Applying inelastic model" << G4endl;
    result = elasticModel->ApplyYourself(projectile, targetNucleus);
    // G4cout << "DEBUG: Inelastic model applied" << G4endl;
   

    // Extract new neutron energy, direction
    G4double E1 = result->GetEnergyChange();
    G4ThreeVector newDirection = result->GetMomentumChange();

    // Compute the energy deposit
    G4double energyDeposited = energy0 - E1;
    if (energyDeposited < 0) energyDeposited = 0;

    // Prepare interaction data
    InteractionData data;
    data.x0 = x0;
    data.dir = newDirection;
    data.energy = E1;
    data.energyDeposited = energyDeposited;
    data.cosTheta = newDirection.cosTheta();
    data.weight = 1.0;
    data.hit = new Hit();

    // Clean up the fake track (also deletes neutronDynamic inside)


    return data;
}


std::pair<G4ThreeVector, G4double> NeutronHelper::GenerateInteractionPoint(const G4Step *step) {

    // Get neutron kinetic energy
    G4double energy = step->GetPreStepPoint()->GetKineticEnergy();

    // Retrieve material properties
    G4LogicalVolume* volume_pre = step->GetPreStepPoint()->GetTouchableHandle()->GetVolume()->GetLogicalVolume();
    G4Material* material = volume_pre->GetMaterial();

    // Compute neutron attenuation length (mean free path)
    G4double attenuation_length = GetAttenuationLength(energy, material);
    // Maximum step length
    G4double maxDistance = step->GetStepLength();

    // Get the entrance and exit points of the step
    G4ThreeVector entrance = step->GetPreStepPoint()->GetPosition();
    G4ThreeVector exit = step->GetPostStepPoint()->GetPosition();

    // Generate a random interaction distance using exponential distribution
    G4double rand = G4UniformRand();
    G4double maxCDF = 1 - std::exp(-maxDistance / attenuation_length);
    G4double adjustedRand = rand * maxCDF;
    G4double distance = -attenuation_length * std::log(1 - adjustedRand);

    // Compute the interaction position
    G4ThreeVector interactionPoint = entrance + (exit - entrance).unit() * distance;

    return std::make_pair(interactionPoint, maxCDF);
}



} // namespace G4Sim