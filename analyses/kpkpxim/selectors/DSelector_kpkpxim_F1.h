#ifndef DSelector_kpkpxim_F1_h
#define DSelector_kpkpxim_F1_h

#include <iostream>

#include "DSelector/DSelector.h"
#include "DSelector/DHistogramActions.h"
#include "DSelector/DCutActions.h"

#include "TH1I.h"
#include "TH2I.h"

class DSelector_kpkpxim_F1 : public DSelector
{
	public:

		DSelector_kpkpxim_F1(TTree* locTree = NULL) : DSelector(locTree){}
		virtual ~DSelector_kpkpxim_F1(){}

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
		DChargedTrackHypothesis* dPiMinus1Wrapper;

		//Step 2
		DParticleComboStep* dStep2Wrapper;
		DKinematicData* dDecayingLambdaWrapper;
		DChargedTrackHypothesis* dPiMinus2Wrapper;
		DChargedTrackHypothesis* dProtonWrapper;

		// DEFINE YOUR HISTOGRAMS HERE
		// EXAMPLES:
		TH1I* dHist_MissingMassSquared;
		TH1I* dHist_BeamEnergy;

	ClassDef(DSelector_kpkpxim_F1, 0);
};

void DSelector_kpkpxim_F1::Get_ComboWrappers(void)
{
	//Step 0
	dStep0Wrapper = dComboWrapper->Get_ParticleComboStep(0);
	dComboBeamWrapper = static_cast<DBeamParticle*>(dStep0Wrapper->Get_InitialParticle());
	dKPlus1Wrapper = static_cast<DChargedTrackHypothesis*>(dStep0Wrapper->Get_FinalParticle(0));
	dKPlus2Wrapper = static_cast<DChargedTrackHypothesis*>(dStep0Wrapper->Get_FinalParticle(1));

	//Step 1
	dStep1Wrapper = dComboWrapper->Get_ParticleComboStep(1);
	dPiMinus1Wrapper = static_cast<DChargedTrackHypothesis*>(dStep1Wrapper->Get_FinalParticle(0));

	//Step 2
	dStep2Wrapper = dComboWrapper->Get_ParticleComboStep(2);
	dDecayingLambdaWrapper = dStep2Wrapper->Get_InitialParticle();
	dPiMinus2Wrapper = static_cast<DChargedTrackHypothesis*>(dStep2Wrapper->Get_FinalParticle(0));
	dProtonWrapper = static_cast<DChargedTrackHypothesis*>(dStep2Wrapper->Get_FinalParticle(1));
}

#endif // DSelector_kpkpxim_F1_h
