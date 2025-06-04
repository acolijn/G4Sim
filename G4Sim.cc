#include "DetectorConstruction.hh"
#include "DetectorConstructionMessenger.hh"
#include "ActionInitialization.hh"

#include "G4RunManagerFactory.hh"
#include "G4SteppingVerbose.hh"
#include "G4UImanager.hh"
#include "QBBC.hh"
#include "FTFP_BERT_HP.hh"

#include "G4VisExecutive.hh"
#include "G4UIExecutive.hh"
#include "G4PhysListFactory.hh"
#include "G4EmLivermorePhysics.hh"

#include "Randomize.hh"
#include "GammaRayHelper.hh"
#include "NeutronHelper.hh"
#include "CustomEmPhysics.hh"
#include "PhysicsListManager.hh"
#include "PhysicsMessenger.hh"

using namespace G4Sim;

/**
 * @brief The main function of the program.
 *
 * This function is the entry point of the program. It initializes the necessary components,
 * sets up the run manager, initializes the visualization, and processes the macro or starts
 * the UI session based on the command line arguments. After the execution, it frees the memory
 * allocated for the visualization manager and the run manager.
 *
 * @param argc The number of command line arguments.
 * @param argv An array of command line arguments.
 * @return An integer representing the exit status of the program.
 */
int main(int argc,char** argv)
{
  // Detect interactive mode (if no arguments) and define UI session

  G4UIExecutive* ui = nullptr;
  if ( argc == 1 ) { ui = new G4UIExecutive(argc, argv); }

  // Optionally: choose a different Random engine...
  // G4Random::setTheEngine(new CLHEP::MTwistEngine);

  //use G4SteppingVerboseWithUnits
  G4int precision = 4;
  G4SteppingVerbose::UseBestUnit(precision);

  // Construct the default run manager
  //
  auto* runManager =
    G4RunManagerFactory::CreateRunManager(G4RunManagerType::Serial);


  // create a new messenger for the custom physics 
 
  auto* physicsMessenger = new PhysicsMessenger();
  //runManager->SetNumberOfThreads(1);

  // Set mandatory initialization classes
  //
  // Detector construction




  // WITH BREM ###################################################
  GammaRayHelper* helper = &GammaRayHelper::Instance();
  NeutronHelper* helper2 = &NeutronHelper::Instance();
  runManager->SetUserInitialization(new DetectorConstruction());
  
  


  
  // Initialize physics using the new PhysicsListManager
  PhysicsListManager physicsManager;
  auto* defaultList = physicsManager.CreatePhysicsList();
  runManager->SetUserInitialization(defaultList);
  runManager->SetUserInitialization(new ActionInitialization(helper,helper2));



  //G4PhysListFactory factory;
  //G4VModularPhysicsList* physicsList = factory.GetReferencePhysList("FTFP_BERT_HP");
  //physicsList->ReplacePhysics(new G4EmLivermorePhysics());
  ////if you want to mess with the Em physics list ...... physicsList->ReplacePhysics(new CustomEmPhysics());
  //runManager->SetUserInitialization(physicsList);
  // User action initialization

  // WITH BREM ###################################################

  // Initialize visualization

  G4VisManager* visManager = new G4VisExecutive;
  // G4VisExecutive can take a verbosity argument - see /vis/verbose guidance.
  // G4VisManager* visManager = new G4VisExecutive("Quiet");
  visManager->Initialize();

  // Get the pointer to the User Interface manager
  G4UImanager* UImanager = G4UImanager::GetUIpointer();

  // Process macro or start UI session
  //

  if (!ui) {
    if (argc >= 3) {
      // 1) parse "preinit.mac"
      G4String preinitFile = argv[1];
      UImanager->ApplyCommand("/control/execute " + preinitFile);

      // 2) Now read physics toggles
      bool brem = physicsMessenger->IsBremEnabled();
      bool pair = physicsMessenger->IsPairEnabled();
      bool rayl = physicsMessenger->IsRayleighEnabled();
      bool elas = physicsMessenger->IsNeutronElasticEnabled();
      bool inelas = physicsMessenger->IsNeutronInelasticEnabled();
      bool ncap = physicsMessenger->IsNeutronCaptureEnabled();
      
      G4cout << "Physics toggles: brem=" << brem << " pair=" << pair << " rayl=" << rayl << " elas=" << elas << " inelas=" << inelas << " ncap=" << ncap << G4endl;
      
      bool needCustom = (!brem || !pair || !rayl || !elas || !inelas || !ncap);
      if (needCustom) {
          G4cout << "Switching to CustomEmPhysics" << G4endl;
          auto* customEm = new CustomEmPhysics(physicsMessenger);
          defaultList->ReplacePhysics(customEm);
      }
      
      // 3) parse "main.mac"
      G4String mainFile = argv[2];
      UImanager->ApplyCommand("/control/execute " + mainFile);
      }
      else if (argc == 2) {
        // single macro
        G4String macroFile = argv[1];
        UImanager->ApplyCommand("/control/execute " + macroFile);
      }
  }
  else {
      // interactive
      UImanager->ApplyCommand("/control/execute vis.mac");
      ui->SessionStart();
      delete ui;
  }
 

  // Job termination
  // Free the store: user actions, physics_list and detector_description are
  // owned and deleted by the run manager, so they should not be deleted
  // in the main() program !

  delete visManager;
  delete runManager;

  return 0;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo.....