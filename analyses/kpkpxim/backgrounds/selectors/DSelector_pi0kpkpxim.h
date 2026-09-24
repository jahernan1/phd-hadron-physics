#ifndef DSelector_pi0kpkpxim_h
#define DSelector_pi0kpkpxim_h

#include <iostream>

#include "DSelector/DSelector.h"
#include "DSelector/DHistogramActions.h"
#include "DSelector/DCutActions.h"

#include "TH1F.h"
#include "TH2F.h"

class DSelector_pi0kpkpxim : public DSelector
{
	public:

		DSelector_pi0kpkpxim(TTree* locTree = NULL) : DSelector(locTree){}
		virtual ~DSelector_pi0kpkpxim(){}

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
		DChargedTrackHypothesis* dKPlus1Wrapper;
		DChargedTrackHypothesis* dKPlus2Wrapper;

		//Step 1
		DParticleComboStep* dStep1Wrapper;
		DKinematicData* dDecayingPi0Wrapper;
		DNeutralParticleHypothesis* dPhoton1Wrapper;
		DNeutralParticleHypothesis* dPhoton2Wrapper;

		//Step 2
		DParticleComboStep* dStep2Wrapper;
		DChargedTrackHypothesis* dPiMinus1Wrapper;

		//Step 3
		DParticleComboStep* dStep3Wrapper;
		DKinematicData* dDecayingLambdaWrapper;
		DChargedTrackHypothesis* dPiMinus2Wrapper;
		DChargedTrackHypothesis* dProtonWrapper;

		// DEFINE YOUR HISTOGRAMS HERE
		// EXAMPLES:
		TH1F* dHist_MissingMassSquared;
		TH1F* dHist_BeamEnergy;

	ClassDef(DSelector_pi0kpkpxim, 0);
};

void DSelector_pi0kpkpxim::Get_ComboWrappers(void)
{
	//Step 0
	dStep0Wrapper = dComboWrapper->Get_ParticleComboStep(0);
	dComboBeamWrapper = static_cast<DBeamParticle*>(dStep0Wrapper->Get_InitialParticle());
	dKPlus1Wrapper = static_cast<DChargedTrackHypothesis*>(dStep0Wrapper->Get_FinalParticle(1));
	dKPlus2Wrapper = static_cast<DChargedTrackHypothesis*>(dStep0Wrapper->Get_FinalParticle(2));

	//Step 1
	dStep1Wrapper = dComboWrapper->Get_ParticleComboStep(1);
	dDecayingPi0Wrapper = dStep1Wrapper->Get_InitialParticle();
	dPhoton1Wrapper = static_cast<DNeutralParticleHypothesis*>(dStep1Wrapper->Get_FinalParticle(0));
	dPhoton2Wrapper = static_cast<DNeutralParticleHypothesis*>(dStep1Wrapper->Get_FinalParticle(1));

	//Step 2
	dStep2Wrapper = dComboWrapper->Get_ParticleComboStep(2);
	dPiMinus1Wrapper = static_cast<DChargedTrackHypothesis*>(dStep2Wrapper->Get_FinalParticle(0));

	//Step 3
	dStep3Wrapper = dComboWrapper->Get_ParticleComboStep(3);
	dDecayingLambdaWrapper = dStep3Wrapper->Get_InitialParticle();
	dPiMinus2Wrapper = static_cast<DChargedTrackHypothesis*>(dStep3Wrapper->Get_FinalParticle(0));
	dProtonWrapper = static_cast<DChargedTrackHypothesis*>(dStep3Wrapper->Get_FinalParticle(1));
}

#endif // DSelector_pi0kpkpxim_h
