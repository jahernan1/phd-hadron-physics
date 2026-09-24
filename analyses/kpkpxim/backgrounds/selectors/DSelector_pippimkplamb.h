#ifndef DSelector_pippimkplamb_h
#define DSelector_pippimkplamb_h

#include <iostream>

#include "DSelector/DSelector.h"
#include "DSelector/DHistogramActions.h"
#include "DSelector/DCutActions.h"

#include "TH1I.h"
#include "TH2I.h"

class DSelector_pippimkplamb : public DSelector
{
	public:

		DSelector_pippimkplamb(TTree* locTree = NULL) : DSelector(locTree){}
		virtual ~DSelector_pippimkplamb(){}

		void Init(TTree *tree);
		Bool_t Process(Long64_t entry);

	private:

		void Get_ComboWrappers(void);
		void Finalize(void);

		// BEAM POLARIZATION INFORMATION
		UInt_t dPreviousRunNumber;
		bool dIsPolarizedFlag; //else is AMO
		bool dIsPARAFlag; //else is PERP or AMO

		bool dIsMC;

		// ANALYZE CUT ACTIONS
		// // Automatically makes mass histograms where one cut is missing
		DHistogramAction_AnalyzeCutActions* dAnalyzeCutActions;

		//CREATE REACTION-SPECIFIC PARTICLE ARRAYS

		//Step 0
		DParticleComboStep* dStep0Wrapper;
		DBeamParticle* dComboBeamWrapper;
		DChargedTrackHypothesis* dPiPlusWrapper;
		DChargedTrackHypothesis* dPiMinus1Wrapper;
		DChargedTrackHypothesis* dKPlusWrapper;

		//Step 1
		DParticleComboStep* dStep1Wrapper;
		DChargedTrackHypothesis* dPiMinus2Wrapper;
		DChargedTrackHypothesis* dProtonWrapper;

		// DEFINE YOUR HISTOGRAMS HERE
		// EXAMPLES:
		TH1I* dHist_MissingMassSquared;
		TH1I* dHist_BeamEnergy;
		TH1I* dHist_ChiSqNdf;

		TH1I* dHist_LambdaInvariantMass;
		TH1I* dHist_SigmaInvariantMass;
		TH1I* dHist_YstarInvariantMass;
		
	ClassDef(DSelector_pippimkplamb, 0);
};

void DSelector_pippimkplamb::Get_ComboWrappers(void)
{
	//Step 0
	dStep0Wrapper = dComboWrapper->Get_ParticleComboStep(0);
	dComboBeamWrapper = static_cast<DBeamParticle*>(dStep0Wrapper->Get_InitialParticle());
	dPiPlusWrapper = static_cast<DChargedTrackHypothesis*>(dStep0Wrapper->Get_FinalParticle(0));
	dPiMinus1Wrapper = static_cast<DChargedTrackHypothesis*>(dStep0Wrapper->Get_FinalParticle(1));
	dKPlusWrapper = static_cast<DChargedTrackHypothesis*>(dStep0Wrapper->Get_FinalParticle(2));

	//Step 1
	dStep1Wrapper = dComboWrapper->Get_ParticleComboStep(1);
	dPiMinus2Wrapper = static_cast<DChargedTrackHypothesis*>(dStep1Wrapper->Get_FinalParticle(0));
	dProtonWrapper = static_cast<DChargedTrackHypothesis*>(dStep1Wrapper->Get_FinalParticle(1));
}

#endif // DSelector_pippimkplamb_h
