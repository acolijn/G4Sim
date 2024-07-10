//
// ********************************************************************
// * License and Disclaimer                                           *
// *                                                                  *
// * The  Geant4 software  is  copyright of the Copyright Holders  of *
// * the Geant4 Collaboration.  It is provided  under  the terms  and *
// * conditions of the Geant4 Software License,  included in the file *
// * LICENSE and available at  http://cern.ch/geant4/license .  These *
// * include a list of copyright holders.                             *
// *                                                                  *
// * Neither the authors of this software system, nor their employing *
// * institutes,nor the agencies providing financial support for this *
// * work  make  any representation or  warranty, express or implied, *
// * regarding  this  software system or assume any liability for its *
// * use.  Please see the license in the file  LICENSE  and URL above *
// * for the full disclaimer and the limitation of liability.         *
// *                                                                  *
// * This  code  implementation is the result of  the  scientific and *
// * technical work of the GEANT4 collaboration.                      *
// * By using,  copying,  modifying or  distributing the software (or *
// * any work based  on the software)  you  agree  to acknowledge its *
// * use  in  resulting  scientific  publications,  and indicate your *
// * acceptance of all terms of the Geant4 Software license.          *
// ********************************************************************
//
//
/// \file B1/include/RunAction.hh
/// \brief Definition of the B1::RunAction class

#ifndef B1RunAction_h
#define B1RunAction_h 1

#include "G4UserEventAction.hh"
#include "G4UserRunAction.hh"
#include "G4Accumulable.hh"
#include "G4AnalysisManager.hh"
#include "globals.hh"
#include "GammaRayHelper.hh"
#include "RunActionMessenger.hh"

class G4Run;

/// Run action class

namespace G4FastSim
{

class EventAction;
class RunActionMessenger;

class RunAction : public G4UserRunAction
{
  public:
    RunAction(EventAction *eventAction, GammaRayHelper *helper);
    ~RunAction();

    void BeginOfRunAction(const G4Run*) override;
    void   EndOfRunAction(const G4Run*) override;

    void InitializeNtuples();
    void DefineEventNtuple();
    void DefineCrossSectionNtuple();
    void DefineDifferentialCrossSectionNtuple(G4double energy) const;

    void SetFastSimulation(G4bool value) { fFastSimulation = value; }
    void SetNumberOfScatters(G4int value) { fNumberOfScattersMax = value; }
    void SetMaxEnergy(G4double value) { fMaxEnergy = value; }


  private:
    EventAction* fEventAction = nullptr;
    GammaRayHelper* fGammaRayHelper = nullptr;
    RunActionMessenger* fMessenger;

    int eventNtupleId = -1;
    int crossSectionNtupleId = -1;
    mutable int diffXsecNtupleId = -1;

    G4bool fFastSimulation = false;
    G4int fNumberOfScattersMax = 0;
    G4double fMaxEnergy = 0.0;
};

}

#endif

