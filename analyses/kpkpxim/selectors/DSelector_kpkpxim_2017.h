#ifndef DSelector_kpkpxim_2017_h
#define DSelector_kpkpxim_2017_h

#include <iostream>

#include "DSelector/DSelector.h"
#include "DSelector/DHistogramActions.h"
#include "DSelector/DCutActions.h"

#include "TH1I.h"
#include "TH2I.h"

class DSelector_kpkpxim_2017 : public DSelector
{
	public:

		DSelector_kpkpxim_2017(TTree* locTree = NULL) : DSelector(locTree){}
		virtual ~DSelector_kpkpxim_2017(){}

		void Init(TTree *tree);
		Bool_t Process(Long64_t entry);

	private:

		void Get_ComboWrappers(void);
		void Finalize(void);

		// BEAM POLARIZATION INFORMATION
		UInt_t dPreviousRunNumber;
		bool dIsPolarizedFlag; //else is AMO
		bool dIsPARAFlag; //else is PERP or AMO

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

		//INVARIANT MASS PLOTS
		TH1I* dHist_XimInvariantMassMeasured_acc;
		TH1I* dHist_XimInvariantMassKinFit_acc;
		
		TH1I* dHist_XimInvariantMassMeasured;
		TH1I* dHist_XimMassMeasured_Uniq; 
		TH1I* dHist_XimMassMeasured_UniqCut;
		TH1I* dHist_XimInvariantMassKinFit;
		
		TH1I* dHist_LambdaInvariantMassMeasured;
		TH1I* dHist_LambdaInvariantMassKinFit;

		//PATH LENGTH PLOTS
		TH1I *dHist_XiPath_preCL;
		TH1I *dHist_XiPath_postCL;
		TH1I *dHist_LambPath_preCL;
		TH1I *dHist_LambPath_postCL;

		//PARTICLE KINEMATICS PLOTS
		TH2I *dHist_ProtonP3VsThetaMeasured;
		TH2I *dHist_ProtonP3VsTheta;
		TH2I *dHist_ProtonP3VsTheta_Uniq;
		TH2I *dHist_XimP3VsThetaMeasured;
		TH2I *dHist_XimP3VsTheta;
		TH2I *dHist_KPlus1P3VsThetaMeasured;
		TH2I *dHist_KPlus1P3VsTheta;
	        TH2I *dHist_KPlus2P3VsThetaMeasured;
		TH2I *dHist_KPlus2P3VsTheta;

		//COMPARISON PLOTS
		TH2I *dHist_XiIM_Egamma;
		TH2I *dHist_XiIM_Egamma_wacc;

		//ChiSq
		TH1I *dHist_testChiSq;
	       

	ClassDef(DSelector_kpkpxim_2017, 0);
};

void DSelector_kpkpxim_2017::Get_ComboWrappers(void)
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

#endif // DSelector_kpkpxim_2017_h
