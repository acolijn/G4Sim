#ifndef _RunAction_h
#define _RunAction_h 1

#include "G4UserEventAction.hh"
#include "G4UserRunAction.hh"
#include "G4Accumulable.hh"
#include "G4AnalysisManager.hh"
#include "globals.hh"
#include "GammaRayHelper.hh"
#include "RunActionMessenger.hh"
#include "NeutronHelper.hh"
class G4Run;

/**
 * @namespace G4Sim
 * @brief A namespace for fast simulation classes in Geant4.
/*/
namespace G4Sim
{

class EventAction;
class RunActionMessenger;

/**
 * @class RunAction
 * @brief User-defined run action class for the simulation.
 *
 * This class inherits from G4UserRunAction and is responsible for defining the actions to be taken at the beginning and end of each run.
 * It also provides methods for initializing and defining various ntuples used for data storage.
 * The class also includes setter methods for configuring the fast simulation, maximum number of scatters, maximum energy, and output file name.
 */
class RunAction : public G4UserRunAction
{
  public:
    RunAction(EventAction *eventAction, GammaRayHelper *helper, NeutronHelper *helper2);
    ~RunAction();

    void BeginOfRunAction(const G4Run*) override;
    void   EndOfRunAction(const G4Run*) override;

    void InitializeNtuples();
    void DefineEventNtuple();
    void DefineCrossSectionNtuple();
    void DefineCrossSectionNtupleNeutron();
    void DefineProcessMapNtuple();
    void DefineDifferentialCrossSectionNtuple(G4double energy) const;

    void SetFastSimulation(G4bool value) { fFastSimulation = value; }
    void SetNumberOfScatters(G4int value) { fNumberOfScattersMax = value; }
    void SetMaxEnergy(G4double value) { fMaxEnergy = value; }
    void SetOutputFileName(G4String value) { fOutputFileName = value; }
    void SetSimulationMode(const G4String& mode) {simulationMode = mode;}
    
    G4String GetSimulationMode() const { return simulationMode; }

    void RecordProcessToHistogram(const G4String& processType);
    
  private:
    EventAction* fEventAction = nullptr;
    GammaRayHelper* fGammaRayHelper = nullptr;
    NeutronHelper* fNeutronHelper = nullptr;
    RunActionMessenger* fMessenger;

    int eventNtupleId = -1;
    int crossSectionNtupleId = -1;
    mutable int diffXsecNtupleId = -1;

    G4bool fFastSimulation = false;
    G4int fNumberOfScattersMax = 0;
    G4double fMaxEnergy = 0.0;
    G4String fOutputFileName = "G4Sim.root";
    G4String simulationMode = "photon";
    


    std::map<G4String, G4int> processMap;
    G4int processNtupleId;
    G4int nextBinIndex;
};

}

#endif

