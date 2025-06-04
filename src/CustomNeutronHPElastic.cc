#include "CustomNeutronHPElastic.hh"
#include "G4NeutronHPManager.hh"
#include "G4NeutronHPElasticFS.hh"
#include "G4SystemOfUnits.hh"
#include "G4HadronicException.hh"
#include "G4NeutronHPChannel.hh"
#include "G4NeutronHPThermalBoost.hh"
#include "G4Step.hh"
#include "G4TouchableHandle.hh"

CustomNeutronHPElastic::CustomNeutronHPElastic() 
    : G4NeutronHPElastic(), numEle(0), overrideSuspension(false) {
    G4NeutronHPElasticFS * theFS = new G4NeutronHPElasticFS;
    if(!getenv("G4NEUTRONHPDATA")) {
        throw G4HadronicException(__FILE__, __LINE__, "Please setenv G4NEUTRONHPDATA to point to the neutron cross-section files.");
    }
    dirName = getenv("G4NEUTRONHPDATA");
    dirName += "/Elastic";
    numEle = G4Element::GetNumberOfElements();

    for (G4int i = 0; i < numEle; i++) {
        theElastic.push_back(new G4NeutronHPChannel);
        theElastic[i]->Init((*(G4Element::GetElementTable()))[i], dirName);
        while(!theElastic[i]->Register(theFS));
    }
    delete theFS;
    SetMinEnergy(0.*eV);
    SetMaxEnergy(20.*MeV);
}

CustomNeutronHPElastic::~CustomNeutronHPElastic() {
    for (auto& channel : theElastic) {
        delete channel;
    }
    theElastic.clear();
}

G4HadFinalState* CustomNeutronHPElastic::ApplyYourself(const G4HadProjectile& aTrack, G4Nucleus& aNucleus, const G4Material* theMaterial) {
    if (numEle < (G4int)G4Element::GetNumberOfElements()) {
        addChannelForNewElement();
    }
    
    G4NeutronHPManager::GetInstance()->OpenReactionWhiteBoard();
    
    if (!theMaterial) {
        G4cerr << "Error: Material is null" << G4endl;
        return nullptr;
    } 

    G4int n = theMaterial->GetNumberOfElements();


    if (n == 0) {
       
        return nullptr;
    }

    G4int index = theMaterial->GetElement(0)->GetIndex();
  
    G4HadFinalState* result = nullptr;
   
    if (n != 1) {
        G4double* xSec = new G4double[n];
        G4double sum = 0;
      
        const G4double* NumAtomsPerVolume = theMaterial->GetVecNbOfAtomsPerVolume();
        G4NeutronHPThermalBoost aThermalE;
        
        for (G4int i = 0; i < n; i++) {
            index = theMaterial->GetElement(i)->GetIndex();
            G4double rWeight = NumAtomsPerVolume[i];
            G4double xsecValue = theElastic[index]->GetXsec(aThermalE.GetThermalEnergy(aTrack, theMaterial->GetElement(i), theMaterial->GetTemperature()));
            xSec[i] = xsecValue * rWeight;
            sum += xSec[i];
         
        }

        G4double random = G4UniformRand();
        G4double running = 0;
        

        for (G4int i = 0; i < n; i++) {
            running += xSec[i];
            index = theMaterial->GetElement(i)->GetIndex();
            if (sum == 0 || random <= running / sum) {
                
                break;
            }
        }
        delete[] xSec;
        result = theElastic[index]->ApplyYourself(aTrack);
    } else {
 
        result = theElastic[index]->ApplyYourself(aTrack);
    }
  
    if (overrideSuspension) {
        result->SetStatusChange(isAlive);
     
    }
    
    aNucleus.SetParameters(G4NeutronHPManager::GetInstance()->GetReactionWhiteBoard()->GetTargA(),
                           G4NeutronHPManager::GetInstance()->GetReactionWhiteBoard()->GetTargZ());
   
    G4NeutronHPManager::GetInstance()->CloseReactionWhiteBoard();
  

    return result;
}

void CustomNeutronHPElastic::addChannelForNewElement() {
    G4NeutronHPElasticFS* theFS = new G4NeutronHPElasticFS;
    for (G4int i = numEle; i < (G4int)G4Element::GetNumberOfElements(); i++) {
        G4cout << "G4NeutronHPElastic Preparing Data for the new element of " << (*(G4Element::GetElementTable()))[i]->GetName() << G4endl;
        theElastic.push_back(new G4NeutronHPChannel);
        theElastic[i]->Init((*(G4Element::GetElementTable()))[i], dirName);
        while(!theElastic[i]->Register(theFS));
    }
    delete theFS;
    numEle = (G4int)G4Element::GetNumberOfElements();
}