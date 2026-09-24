#ifndef DSelector_thrown_kpkpxim_h
#define DSelector_thrown_kpkpxim_h

#include <iostream>

#include "DSelector/DSelector.h"

#include "TH1I.h"
#include "TH2I.h"

class DSelector_thrown_kpkpxim : public DSelector
{
	public:

		DSelector_thrown_kpkpxim(TTree* locTree = NULL) : DSelector(locTree){}
		virtual ~DSelector_thrown_kpkpxim(){}

		void Init(TTree *tree);
		Bool_t Process(Long64_t entry);

	private:

		void Finalize(void);

		// BEAM POLARIZATION INFORMATION
		UInt_t dPreviousRunNumber;
		bool dIsPolarizedFlag; //else is AMO
		bool dIsPARAFlag; //else is PERP or AMO


		// PUT HISTOGRAMS HERE
		
		//invariant mass distributions
		TH1I* dHist_thrown_YstarInvariantMass;
		TH1I* dHist_thrown_YstarInvariantMass_wrong;
		TH1I* dHist_thrown_XiInvariantMass;
        TH1I* dHist_thrown_XiLifetimeRestFrame_postCL;

		//True Kinematics
		TH2I* dHist_thrown_KPlus1P3VsTheta;
		TH2I* dHist_thrown_KPlus1P3VsTheta_CM;
		TH2I* dHist_thrown_KPlus2P3VsTheta;
		TH2I* dHist_thrown_KPlus2P3VsTheta_CM;
		TH2I* dHist_thrown_YstarP3VsTheta;
		TH2I* dHist_thrown_YstarP3VsTheta_CM;

		TH2I* dHist_thrown_LambdaP3VsTheta;
		TH2I* dHist_thrown_LambdaP3VsTheta_CM;
		TH2I* dHist_thrown_LambdaP3VsTheta_HF;
		TH2I* dHist_thrown_LambdaP3VsCosTheta_HF;

		TH2I* dHist_thrown_PiMinus1P3VsTheta;
		TH2I* dHist_thrown_PiMinus1P3VsTheta_HF;
		TH2I* dHist_thrown_PiMinus1P3VsCosTheta_HF;
		
		TH2I* dHist_thrown_PiMinus2P3VsTheta;
		TH2I* dHist_thrown_ProtonP3VsTheta;
		
		//For Cross section
		TH1I *dHist_thrown_Egamma; 
		TH2I *dHist_thrown_Egamma_t;
		
		//
		TH1I* dHist_thrown_ReactionTypes;
		TH1I* dHist_thrown_ReactionNumber;
		
	ClassDef(DSelector_thrown_kpkpxim, 0);
};

#endif // DSelector_thrown_kpkpxim_h
