#ifndef DSelector_thrown_kpkpxim_F1_h
#define DSelector_thrown_kpkpxim_F1_h

#include <iostream>

#include "DSelector/DSelector.h"

#include "TH1I.h"
#include "TH2I.h"

class DSelector_thrown_kpkpxim_F1 : public DSelector
{
	public:

		DSelector_thrown_kpkpxim_F1(TTree* locTree = NULL) : DSelector(locTree){}
		virtual ~DSelector_thrown_kpkpxim_F1(){}

		void Init(TTree *tree);
		Bool_t Process(Long64_t entry);

	private:

		void Finalize(void);

		// BEAM POLARIZATION INFORMATION
		UInt_t dPreviousRunNumber;
		bool dIsPolarizedFlag; //else is AMO
		bool dIsPARAFlag; //else is PERP or AMO

	ClassDef(DSelector_thrown_kpkpxim_F1, 0);
};

#endif // DSelector_thrown_kpkpxim_F1_h
