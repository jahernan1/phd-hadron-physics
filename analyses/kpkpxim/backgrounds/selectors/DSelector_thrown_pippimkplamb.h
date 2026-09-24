#ifndef DSelector_thrown_pippimkplamb_h
#define DSelector_thrown_pippimkplamb_h

#include <iostream>

#include "DSelector/DSelector.h"

#include "TH1I.h"
#include "TH2I.h"

class DSelector_thrown_pippimkplamb : public DSelector
{
	public:

		DSelector_thrown_pippimkplamb(TTree* locTree = NULL) : DSelector(locTree){}
		virtual ~DSelector_thrown_pippimkplamb(){}

		void Init(TTree *tree);
		Bool_t Process(Long64_t entry);

	private:

		void Finalize(void);

		// BEAM POLARIZATION INFORMATION
		UInt_t dPreviousRunNumber;
		bool dIsPolarizedFlag; //else is AMO
		bool dIsPARAFlag; //else is PERP or AMO

		TH1I* dHist_thrown_YstarInvariantMass;
		TH1I* dHist_thrown_SigmaInvariantMass;
		TH1I* dHist_thrown_LambdaInvariantMass;
		
	ClassDef(DSelector_thrown_pippimkplamb, 0);
};

#endif // DSelector_thrown_pippimkplamb_h
