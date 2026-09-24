#ifndef DSelector_kpkpxim_h
#define DSelector_kpkpxim_h

#include <iostream>
#include <fstream>
#include <stdio.h>
#include <string.h>

#include "DSelector/DSelector.h"
#include "DSelector/DHistogramActions.h"
#include "DSelector/DCutActions.h"

#include "TLorentzRotation.h"
#include "TH1I.h"
#include "TH2I.h"
#include "TH3I.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TH3F.h"

class DSelector_kpkpxim : public DSelector
{
	public:

		DSelector_kpkpxim(TTree* locTree = NULL) : DSelector(locTree){}
		virtual ~DSelector_kpkpxim(){}

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
		TH1F* dHist_MissingMassSquared;
		TH1F* dHist_BeamEnergy;
		//ChiSq Plots
		TH1F* dHist_ChiSqNDF;
		// Beam Bunch
		TH1F* dHist_BeamBunches;
		// Vertex Histos
		TH2F* dHist_ProductionVertex_XVsY;
		TH1F* dHist_ProductionVertex_Z;
		TH2F* dHist_XiMinusVertex_XVsY;
		TH1F* dHist_XiMinusVertex_Z;
		TH2F* dHist_LambdaVertex_XVsY;
		TH1F* dHist_LambdaVertex_Z;

		//INVARIANT MASS PLOTS
		TH1F* dHist_YstarInvariantMass_nocuts;
		TH1F* dHist_YstarInvariantMass_Measured_nocuts;
		TH1F* dHist_YstarInvariantMass;
		TH1F* dHist_YstarInvariantMass_Measured;
		TH1F* dHist_YstarInvariantMass_Kp2Xim_nocuts;
		TH1F* dHist_YstarInvariantMass_Kp2Xim;
		TH1F* dHist_YstarInvariantMass_Kp2Xim_XimCut;
		TH1F* dHist_YstarInvariantMass_KpThetaXim_nocuts;
		TH1F* dHist_YstarInvariantMass_XimCut;
		TH1F* dHist_YstarInvariantMass_XimCut_Measured;
		TH1F* dHist_YstarInvariantMass_SigmaRegion;
		TH2F* dHist_YstarMass_XiMinusMass;
		
		TH1F* dHist_XimInvariantMassMeasured;
		TH1F* dHist_XimInvariantMassKinFit;
		TH1F* dHist_XimInvariantMassKinFit_FlightSignificanceCut;
		TH1F* dHist_XimInvariantMassKinFit_MomCut;
		TH1F* dHist_XimInvariantMassKinFit_MomCut_FlightSignificanceCut;
		TH1F* dHist_XimInvariantMassKinFit_ChargedTrackCut;
		TH1F* dHist_XimInvariantMassKinFit_NeutralTrackCut;
		TH2F* dHist_XimInvariantMassKF_KPlus2P3;
		TH2F* dHist_XimInvariantMassKF_KPlusLowP3;
		
		TH1F* dHist_XimInvariantMass_CM;
		TH1F* dHist_XimInvariantMass_HF;
		TH2F* dHist_XimInvariantMass_XimVertexZ;
		TH2F* dHist_XimInvariantMass_XimVertexZ_SignificanceCut;
		TH2F* dHist_XimInvariantMass_LambdaVertexZ;
		TH2F* dHist_XimInvariantMass_LambdaVertexZ_SignificanceCut;

		TH1F* dHist_LambdaInvariantMassMeasured;
		TH1F* dHist_LambdaInvariantMassKinFit;

		TH2F* dHist_KPlusHighpPiMinus1_PiMinus2_InvariantMass;
		TH2F* dHist_XiMinusIM_LambdaIM;
		
		//PATH LENGTH PLOTS
		TH1F* dHist_XiPath_preCL;
		TH1F* dHist_XiPath_postCL;
		TH1F* dHist_XiLifetime_postCL;
		TH1F* dHist_XiLifetimeRestFrame_postCL;
		TH2F* dHist_XiMass_XiPathLength;
		TH2F* dHist_XiMass_XiPathLength_SignificanceCut;
		TH2F* dHist_XiMass_XiPathLengthSignificance;
		TH2F* dHist_XiMass_XiLifetime;
		TH2F* dHist_XiMass_XiLifetimeRestframe;
		TH1F* dHist_LambPath_preCL;
		TH1F* dHist_LambPath_postCL;
		TH1F* dHist_LambdaLifetime_postCL;
		TH1F* dHist_LambdaLifetimeRestFrame_postCL;
				
		//PARTICLE KINEMATICS PLOTS
		TH2F* dHist_ProtonP3VsThetaMeasured;
		TH2F* dHist_ProtonP3VsTheta;
		TH2F* dHist_ProtonP3VsTheta_CM;
		TH2F* dHist_ProtonP3VsTheta_XimCutIn;
		TH2F* dHist_ProtonP3VsTheta_XimCutOut;

		TH2F* dHist_XimP3VsThetaMeasured;
		TH2F* dHist_XimP3VsTheta;
		TH2F* dHist_XimP3VsTheta_CM;
		TH2F* dHist_XimP3VsTheta_XimCutIn;
		TH2F* dHist_XimP3VsTheta_XimCutOut;

		TH2F* dHist_KPlus1P3VsThetaMeasured;
		TH2F* dHist_KPlus1P3VsTheta;
		TH2F* dHist_KPlus1P3VsTheta_CM;
		TH2F* dHist_KPlus1P3VsTheta_XimCutIn;
		TH2F* dHist_KPlus1P3VsTheta_XimCutOut;

        TH2F* dHist_KPlus2P3VsThetaMeasured;
		TH2F* dHist_KPlus2P3VsTheta;
		TH2F* dHist_KPlus2P3VsTheta_CM;
		TH2F* dHist_KPlus2P3VsTheta_XimCutIn;
		TH2F* dHist_KPlus2P3VsTheta_XimCutOut;

		TH2F* dHist_KPlusHighP3_Theta_Measured;
		TH2F* dHist_KPlusHighP3_Theta;
		TH2F* dHist_KPlusHighP3_Theta_CM;
		TH2F* dHist_KPlusHighP3_Theta_XimCutIn;
		TH2F* dHist_KPlusHighP3_Theta_XimCutOut;
		TH2F* dHist_KPlusHighP3_Theta_SigmaRegion;
	      
		TH2F* dHist_KPlusHighP3Phi;

		TH2F* dHist_KPlusLowP3_Theta_Measured;
		TH2F* dHist_KPlusLowP3_Theta;	
		TH2F* dHist_KPlusLowP3_Theta_CM;	
		TH2F* dHist_KPlusLowP3_Theta_XimCutIn;
		TH2F* dHist_KPlusLowP3_Theta_XimCutOut;	

		TH2F* dHist_KPlus1P3VsTheta_YstarCut;
		TH2F* dHist_KPlus2P3VsTheta_YstarCut;

		TH3F* dHist_KPlusHighP3_KPlusLowP3_t;
		TH3F* dHist_KPlus1P3_KPlus2P3_t;

		TH2F* dHist_LambdaP3Theta ;
		TH2F* dHist_LambdaP3Theta_CM ;
		TH2F* dHist_LambdaP3Theta_HF ;
		TH2F* dHist_LambdaP3CosTheta_HF ;
		TH2F* dHist_LambdaP3CosTheta_HF_wacc ;
		TH2F* dHist_LambdaP3ThetaMeasured ;
		TH2F* dHist_LambdaP3Theta_XimCutIn ;
		TH2F* dHist_LambdaP3Theta_XimCutIn_wacc ;
		TH2F* dHist_LambdaP3Theta_XimCutOut ;
		
		TH2F* dHist_PiMinus1P3Theta;
		TH2F* dHist_PiMinus1P3Theta_CM;
		TH2F* dHist_PiMinus1P3ThetaMeasured;
		TH2F* dHist_PiMinus1P3Theta_XimCutIn;
		TH2F* dHist_PiMinus1P3Theta_XimCutOut;
		TH2F* dHist_PiMinus1P3Theta_HF;
		TH2F* dHist_PiMinus1P3CosTheta_HF;
		TH2F* dHist_PiMinus2P3Theta;
		TH2F* dHist_PiMinus2P3Theta_CM;
		TH2F* dHist_PiMinus2P3ThetaMeasured;
		TH2F* dHist_PiMinus2P3Theta_XimCutIn;
		TH2F* dHist_PiMinus2P3Theta_XimCutOut;
		
		TH2F* dHist_YstarP3Theta;
		TH2F* dHist_YstarP3Theta_CM;
	       
        //XSECTION PLOTS
		TH1F* dHist_XiIM_Egamma_arr[10];
        TH2F* dHist_XiIM_Egamma;
        TH2F* dHist_XiIM_Egamma_woacc;
        TH2F* dHist_XiIM_Egamma_wacc;
        TH3F* dHist_Xi_Egamma_t;
		TH3F* dHist_Xi_Egamma_t_woacc;
        TH3F* dHist_Xi_Egamma_t_wacc;

		TH1F* dHist_t_XimCut;
		TH1F* dHist_t_XimCut_KaonHighCut;

		//ChiSqNDF
		TH1F* dHist_ChiSqNDF_XimCut;
		TH1F* dHist_ChiSqNDF_XimSignal;
		TH1F* dHist_ChiSqNDF_XimSideband;
		TH2F* dHist_XimInvariantMassKF_ChiSqNdf;
		TH3F* dHist_XimInvariantMassKF_ChiSqNdf_MissingMassSq;
		
		
		// Number of tracks
		TH1F* dHist_NumChargedTracksPerEvent;
		TH1F* dHist_NumNeutralTracksPerEvent;
		TH1F* dHist_NumCombosPerEvent;
		TH1F* dHist_NumCombosSurviveCut;
		TH1F* dHist_NumUnusedChargedTracksPerCombo;
		TH1F* dHist_NumUnusedNeutralTracksPerCombo;
		TH1F* dHist_NumReconChargedTracksPerEvent;
		TH1F* dHist_NumReconNeutralTracksPerEvent;

		// Thrown Histos
		TH2F* dHist_KPlus1P3VsTruth; 
		TH2F* dHist_KPlus2P3VsTruth;
		TH2F* dHist_KPlusHighP3VsTruth; 
		TH2F* dHist_KPlusLowP3VsTruth; 
		char* xi_egamma_name;

	ClassDef(DSelector_kpkpxim, 0);
};

void DSelector_kpkpxim::Get_ComboWrappers(void)
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

#endif // DSelector_kpkpxim_h
