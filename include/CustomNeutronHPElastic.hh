#ifndef CustomNeutronHPElastic_h
#define CustomNeutronHPElastic_h

#include "G4NeutronHPElastic.hh"
#include "G4Nucleus.hh"
#include "G4HadFinalState.hh"
#include <vector>
#include "G4NeutronHPChannel.hh"
#include "G4NeutronHPThermalBoost.hh"



class CustomNeutronHPElastic : public G4NeutronHPElastic {
public:
    CustomNeutronHPElastic();
    virtual ~CustomNeutronHPElastic();

    // Override the ApplyYourself method
    G4HadFinalState* ApplyYourself(const G4HadProjectile& aTrack, G4Nucleus& aNucleus, const G4Material* material);
    void addChannelForNewElement();  // Necessary to handle dynamic element addition

private:
    std::vector<G4NeutronHPChannel*> theElastic;  // Vector of neutron HP channels
    G4String dirName;
    G4int numEle;
    bool overrideSuspension;
};

#endif