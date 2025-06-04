#ifndef NEUTRON_HELPER_H
#define NEUTRON_HELPER_H

#include "G4NeutronElasticXS.hh"
#include "G4NeutronInelasticXS.hh"
#include "G4NeutronCaptureXS.hh"
#include "G4HadronElastic.hh"
#include "G4NeutronHPInelastic.hh"

#include "G4Material.hh"
#include "G4ThreeVector.hh"
#include "G4PhysicalConstants.hh"
#include "G4SystemOfUnits.hh"
#include "G4DataVector.hh"
#include "G4AutoLock.hh"
#include "G4Track.hh"
#include "G4Step.hh"
#include "Hit.hh"
#include <map>
#include <mutex>
#include "GammaRayHelper.hh"
#include "G4NeutronHPCapture.hh"

#include <G4ParticleHPElasticData.hh>
#include <G4ParticleHPInelasticData.hh>
#include <G4NeutronHPCaptureData.hh>
#include <G4NeutronHPFissionData.hh>
#include <G4NeutronHPFission.hh>
#include <G4NeutronHPElastic.hh>    

#include "CustomNeutronHPElastic.hh"
/**
 * @namespace G4Sim
 * @brief A namespace for neutron transport and interaction models in Geant4.
 */
namespace G4Sim {

struct ElasticScatteringKinematics  {
    G4double E1;
    G4double cosTheta;
};

/**
 * @class NeutronHelper
 * @brief A helper class for handling neutron interactions.
 *
 * This class provides methods for calculating neutron cross sections,
 * elastic/inelastic scattering angles, and generating interaction points.
 */
class NeutronHelper {
public:
    static thread_local NeutronHelper& Instance();
    void Initialize();
    void InitializeCDFs(G4double energy);
    
    G4double GetElasticCrossSection(G4double energy, G4Material* material);
    G4double GetInelasticCrossSection(G4double energy, G4Material* material);
    G4double GetCaptureCrossSection(G4double energy, G4Material* material);
    G4double GetFissionCrossSection(G4double energy, G4Material* material);
    G4double GetTotalCrossSection(G4double energy, G4Material* material);
    G4double GetAttenuationLength(G4double energy, G4Material* material); 
    G4ThreeVector SampleScatteredDirectionFromEnergies(const G4ThreeVector& initialDir,
                                                   G4double E0,
                                                   G4double E1,
                                                   G4double A);
    std::pair<G4ThreeVector, G4double> GenerateInteractionPoint(const G4Step* step);


    InteractionData DoElasticNeutronScatter(const G4Step* step, G4ThreeVector x0);
    InteractionData DoInelasticNeutronScatter(const G4Step* step, G4ThreeVector x0);
    InteractionData DoNeutronCapture(const G4Step* step, G4ThreeVector x0);


    G4NeutronElasticXS* GetElasticModel() {
        return elasticXS;
    }

    G4ParticleHPElasticData* GetElasticHPModel() {
        return elasticHP;
    }

    G4NeutronInelasticXS* GetInelasticModel() {
        return inelasticXS;
    }

    G4ParticleHPInelasticData* GetInelasticHPModel() {
        return inelasticHP;
    }

    G4NeutronCaptureXS* GetCaptureModel() {
        return captureXS;
    }

    G4NeutronHPCaptureData* GetCaptureHPModel() {
        return captureHP;
    }

    G4HadronElastic* GetElasticScatteringModel() {
        return elasticModel;
    }

    G4NeutronHPInelastic* GetInelasticScatteringModel() {
        return inelasticModel;
    }

    G4NeutronHPCapture* GetCaptureScatteringModel() {
        return captureModel;
    }

    G4NeutronHPFission* GetFissionModel() {
        return fissionModel;
    }

    G4NeutronHPFissionData* GetFissionHPModel() {
        return fissionHP;
    }

    G4NeutronHPElastic* GetElasticModelHP() {
        return elasticModelHP;
    }

    CustomNeutronHPElastic* GetCustomElasticModelHP() {
        return customHPElastic;
    }
private:
    NeutronHelper();
    ~NeutronHelper() = default;
    CDFData CreateCDF(const G4Element* element, G4double energy, const G4Material* material);

    
  
    NeutronHelper(const NeutronHelper&) = delete;
    NeutronHelper& operator=(const NeutronHelper&) = delete;

    G4NeutronElasticXS* elasticXS;
    G4ParticleHPElasticData* elasticHP;

    G4NeutronInelasticXS* inelasticXS;
    G4ParticleHPInelasticData* inelasticHP;

    G4NeutronCaptureXS* captureXS;
    G4NeutronHPCaptureData* captureHP;
    
    G4HadronElastic* elasticModel;   
    G4NeutronHPInelastic* inelasticModel; 
    G4NeutronHPCapture* captureModel;

    G4NeutronHPFission* fissionModel;
    G4NeutronHPFissionData* fissionHP;
    G4NeutronHPElastic* elasticModelHP;

    CustomNeutronHPElastic* customHPElastic;


    std::vector<std::string> fElementsUsed;

    std::map<const G4Element*, CDFData> cdfDataMap; // CDFs for each element

    bool cdfsInitialized;
    G4double fInitialEnergy;
};

}
#endif // NEUTRON_HELPER_H