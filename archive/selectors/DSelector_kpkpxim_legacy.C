#include "DSelector_kpkpxim.h"
#include <stdio.h>

Int_t enBins = 10;
Double_t arr_enBins[] = {6.4, 6.9, 7.4, 7.9, 8.4, 8.9, 9.4, 9.9, 10.4, 10.9, 11.4};
Double_t ximCutR[] = {1.329, 1.334, 1.34};
Double_t ximCutL[] = {1.315, 1.31, 1.30};
Double_t mm2Cut[] = {0.01, 0.02, 0.03, 0.04, 0.05, 0.06, 0.07};
Double_t chiSqNdfCut[] = {2.0, 2.5, 3.0, 3.5, 4.0, 5.0, 6.0};

void DSelector_kpkpxim::Init(TTree *locTree)
{
    // USERS: IN THIS FUNCTION, ONLY MODIFY SECTIONS WITH A "USER" OR "EXAMPLE" LABEL. LEAVE THE REST ALONE.

	// The Init() function is called when the selector needs to initialize a new tree or chain.
	// Typically here the branch addresses and branch pointers of the tree will be set.
	// Init() will be called many times when running on PROOF (once per file to be processed).

	//USERS: SET OUTPUT FILE NAME //can be overriden by user in PROOF
	dOutputFileName = "kpkpxim.root"; //"" for none
	dOutputTreeFileName = ""; //"" for none
	dFlatTreeFileName = "flatTree_kpkpxim.root"; //output flat tree (one combo per tree entry), "" for none
	dFlatTreeName = "flatTree_kpkpxim"; //if blank, default name will be chosen

	//Because this function gets called for each TTree in the TChain, we must be careful:
    //We need to re-initialize the tree interface & branch wrappers, but don't want to recreate histograms
	bool locInitializedPriorFlag = dInitializedFlag; //save whether have been initialized previously
	DSelector::Init(locTree); //This must be called to initialize wrappers for each new TTree
	//gDirectory now points to the output file with name dOutputFileName (if any)
	if(locInitializedPriorFlag)
		return; //have already created histograms, etc. below: exit

	Get_ComboWrappers();
	dPreviousRunNumber = 0;

	/*********************************** EXAMPLE USER INITIALIZATION: ANALYSIS ACTIONS **********************************/

	// EXAMPLE: Create deque for histogramming particle masses:
	// // For histogramming the phi mass in Xi- -> Lambda pi-
	// // Be sure to change this and dAnalyzeCutActions to match reaction
	std::deque<Particle_t> MyXim;
	MyXim.push_back(PiMinus); MyXim.push_back(Lambda);
	std::deque<Particle_t> MyLambda;
	MyLambda.push_back(PiMinus); MyLambda.push_back(Proton);

    //ANALYSIS ACTIONS: 
    //Executed in order if added to dAnalysisActions
	//false/true below: use measured/kinfit data
	
	//KinFit Results
	dAnalysisActions.push_back(new DHistogramAction_KinFitResults(dComboWrapper));

	//KINEMATICS 
	dAnalysisActions.push_back(new DHistogramAction_ParticleComboKinematics(dComboWrapper, false, "Measured"));
    dAnalysisActions.push_back(new DHistogramAction_ParticleComboKinematics(dComboWrapper, true, "KinFit"));
	
	//PID
	dAnalysisActions.push_back(new DHistogramAction_ParticleID(dComboWrapper, false, "Measured"));
    dAnalysisActions.push_back(new DHistogramAction_ParticleID(dComboWrapper, true, "KinFit"));
    dAnalysisActions.push_back(new DHistogramAction_PIDFOM(dComboWrapper));

	//MASSES
	dAnalysisActions.push_back(new DHistogramAction_InvariantMass(dComboWrapper,false, 1, MyXim, 1000, 1.2, 1.5, "Measured"));
	dAnalysisActions.push_back(new DHistogramAction_InvariantMass(dComboWrapper, true, 1, MyXim, 1000, 1.2, 1.5, "Kinfit"));

	dAnalysisActions.push_back(new DHistogramAction_MissingMassSquared(dComboWrapper, false, 1000, -0.4, 0.4, "Measured"));
	dAnalysisActions.push_back(new DHistogramAction_MissingMassSquared(dComboWrapper, true, 1000, -0.4, 0.4, "KinFit"));

	// //KINFIT RESULTS
    //dAnalysisActions.push_back(new DHistogramAction_KinFitResults(dComboWrapper, "KinFit"));
	//dAnalysisActions.push_back(new DHistogramAction_ParticleComboKinematics(dComboWrapper, false, "Measured"));
    // dAnalysisActions.push_back(new DHistogramAction_ParticleComboKinematics(dComboWrapper, true, "PostCut"));
	// dAnalysisActions.push_back(new DHistogramAction_KinFitResults::Create_ParticlePulls(" ", true, true, false, 1, XiMinus));	
	
	//below: value: +/- N ns, Unknown: All PIDs, SYS_NULL: all timing systems
	//NOMINAL PID TIMING CUTS (https://halldweb.jlab.org/wiki/index.php/PID_study_proposal)
	// dAnalysisActions.push_back(new DCutAction_PIDDeltaT(dComboWrapper, false, 0.75, KPlus, SYS_BCAL));
	// dAnalysisActions.push_back(new DCutAction_PIDDeltaT(dComboWrapper, false, 0.3, KPlus, SYS_TOF));
	// dAnalysisActions.push_back(new DCutAction_PIDDeltaT(dComboWrapper, false, 2.5, KPlus, SYS_FCAL));
	// dAnalysisActions.push_back(new DCutAction_PIDDeltaT(dComboWrapper, false, 2.5, KPlus, SYS_START));
	// dAnalysisActions.push_back(new DCutAction_PIDDeltaT(dComboWrapper, true, 0.75, KPlus, SYS_BCAL));
	// dAnalysisActions.push_back(new DCutAction_PIDDeltaT(dComboWrapper, true, 0.3, KPlus, SYS_TOF));
	// dAnalysisActions.push_back(new DCutAction_PIDDeltaT(dComboWrapper, true, 2.5, KPlus, SYS_FCAL));
	// dAnalysisActions.push_back(new DCutAction_PIDDeltaT(dComboWrapper, true, 2.5, KPlus, SYS_START));

	// dAnalysisActions.push_back(new DCutAction_PIDDeltaT(dComboWrapper, false, 1.0, Proton, SYS_BCAL));
	// dAnalysisActions.push_back(new DCutAction_PIDDeltaT(dComboWrapper, false, 0.6, Proton, SYS_TOF));
	// dAnalysisActions.push_back(new DCutAction_PIDDeltaT(dComboWrapper, false, 2.0, Proton, SYS_FCAL));
	// dAnalysisActions.push_back(new DCutAction_PIDDeltaT(dComboWrapper, false, 2.5, Proton, SYS_START));
	// dAnalysisActions.push_back(new DCutAction_PIDDeltaT(dComboWrapper, true, 1.0, Proton, SYS_BCAL));
	// dAnalysisActions.push_back(new DCutAction_PIDDeltaT(dComboWrapper, true, 0.6, Proton, SYS_TOF));
	// dAnalysisActions.push_back(new DCutAction_PIDDeltaT(dComboWrapper, true, 2.0, Proton, SYS_FCAL));
	// dAnalysisActions.push_back(new DCutAction_PIDDeltaT(dComboWrapper, true, 2.5, Proton, SYS_START));
	
	// dAnalysisActions.push_back(new DCutAction_PIDDeltaT(dComboWrapper, false, 1.0, PiMinus, SYS_BCAL));
	// dAnalysisActions.push_back(new DCutAction_PIDDeltaT(dComboWrapper, false, 0.5, PiMinus, SYS_TOF));
	// dAnalysisActions.push_back(new DCutAction_PIDDeltaT(dComboWrapper, false, 2.0, PiMinus, SYS_FCAL));
	// dAnalysisActions.push_back(new DCutAction_PIDDeltaT(dComboWrapper, false, 2.5, PiMinus, SYS_START));
	// dAnalysisActions.push_back(new DCutAction_PIDDeltaT(dComboWrapper, true, 1.0, PiMinus, SYS_BCAL));
	// dAnalysisActions.push_back(new DCutAction_PIDDeltaT(dComboWrapper, true, 0.5, PiMinus, SYS_TOF));
	// dAnalysisActions.push_back(new DCutAction_PIDDeltaT(dComboWrapper, true, 2.0, PiMinus, SYS_FCAL));
	// dAnalysisActions.push_back(new DCutAction_PIDDeltaT(dComboWrapper, true, 2.5, PiMinus, SYS_START));

	//BEAM ENERGY
	dAnalysisActions.push_back(new DHistogramAction_BeamEnergy(dComboWrapper, false));
	
	
	/***************************************
	  Apply Cuts to dAnalysisActions here
	***************************************/

	//CUT CHISQ
	dAnalysisActions.push_back(new DCutAction_KinFitChiSq(dComboWrapper, 4, "ChiSqNdfCut"));
	//CUT MISSING MASS
	dAnalysisActions.push_back(new DCutAction_MissingMassSquared(dComboWrapper, false, -0.02, 0.02));
    dAnalysisActions.push_back(new DCutAction_BeamEnergy(dComboWrapper, false, 6, 12));
    // dAnalysisActions.push_back(new DCutAction_InvariantMass(dComboWrapper, false, XiMinus, 1.31, 1.34));
    // dAnalysisActions.push_back(new DCutAction_InvariantMass(dComboWrapper, true, XiMinus, 1.31, 1.34));	

	/***************************************/
		
	// ANALYZE CUT ACTIONS
	// Change MyXi to match reaction
	//dAnalyzeCutActions = new DHistogramAction_AnalyzeCutActions( dAnalysisActions, dComboWrapper, false, 1, MyXim, 300, 1.2, 1.5, "CutActionEffect");
	dAnalyzeCutActions = new DHistogramAction_AnalyzeCutActions( dAnalysisActions, dComboWrapper, true, 1, MyXim, 300, 1.2, 1.5, "CutActionEffect_KinFit");

	//INITIALIZE ACTIONS
	//If you create any actions that you want to run manually (i.e. don't add to dAnalysisActions), be sure to initialize them here as well
	Initialize_Actions();
	dAnalyzeCutActions->Initialize(); // manual action, must call Initialize()

	/******************************** EXAMPLE USER INITIALIZATION: STAND-ALONE HISTOGRAMS *******************************/

	//EXAMPLE MANUAL HISTOGRAMS:
	dHist_MissingMassSquared = new TH1F("MissingMassSquared", "; (MM)^{2} (GeV/c^{2})^{2}; Counts", 200, -0.1, 0.1);
	dHist_BeamEnergy = new TH1F("BeamEnergy", ";Beam Energy (GeV)", 500, 2.0, 12.0);
	
	// Vertex Plots
	dHist_ProductionVertex_XVsY = new TH2F("ProductionVertex_XVsY", "; Vertex-X (cm); Vertex-Y (cm)", 200, -2, 2, 200, -2, 2);
	dHist_ProductionVertex_Z = new TH1F("ProductionVertex_Z", "; Vertex-Z (cm); ", 200, 30, 230);
	dHist_XiMinusVertex_XVsY = new TH2F("XiMinusVertex_XVsY", "; Vertex-X (cm); Vertex-Y (cm)", 200, -2, 2, 200, -2, 2);
	dHist_XiMinusVertex_Z = new TH1F("XiMinusVertex_Z", "; Vertex-Z (cm); ", 200, 30, 230);
	dHist_LambdaVertex_XVsY = new TH2F("LambdaVertex_XVsY", "; Vertex-X (cm); Vertex-Y (cm)", 200, -2, 2, 200, -2, 2);
	dHist_LambdaVertex_Z = new TH1F("LambdaVertex_Z", "; Vertex-Z (cm); ", 200, 30, 230);
	
	// invariant mass
	dHist_XimInvariantMassMeasured = new TH1F("XimInvariantMassMeasured", "Invariant Mass; M(#Lambda#pi^{-}) (GeV/c^{2})", 250, 1.25, 1.5); 
	dHist_XimInvariantMassKinFit = new TH1F("XimInvariantMassKinFit", "Invariant Mass; M(#Lambda#pi^{-}) (GeV/c^{2})", 250, 1.25, 1.5);
	dHist_XimInvariantMass_CM = new TH1F("XimInvariantMass_CoM", "Invariant Mass; M(#Lambda#pi^{-}) (GeV/c^{2})", 250, 1.25, 1.5);
	dHist_XimInvariantMass_HF = new TH1F("XimInvariantMassKinFit_HF", "Invariant Mass; M(#Lambda#pi^{-}) (GeV/c^{2})", 250, 1.25, 1.5);
	dHist_XimInvariantMassKinFit_FlightSignificanceCut = new TH1F("XimInvariantMassKinFit_FlightSignificanceCut", "; M(#Lambda#pi^{-}) (GeV/c^{2}); Events", 250, 1.25, 1.5);
	dHist_XimInvariantMassKinFit_ChargedTrackCut = new TH1F("XimInvariantMassKinFit_ChargedTrackCut", "Invariant Mass; M(#Lambda#pi^{-}) (GeV/c^{2})", 250, 1.25, 1.5);	 
	dHist_XimInvariantMassKinFit_NeutralTrackCut = new TH1F("XimInvariantMassKinFit_NeutralTrackCut", "Invariant Mass; M(#Lambda#pi^{-}) (GeV/c^{2})", 250, 1.25, 1.5);	 
	dHist_XimInvariantMassKF_KPlus2P3 = new TH2F("XimInvariantMassKinFit_KPlus2P3", "; M(#Lambda#pi^{-}) (GeV/c^{2})", 250, 1.25, 1.5, 100, 0, 10);
	dHist_XimInvariantMassKF_KPlusLowP3 = new TH2F("XimInvariantMassKinFit_KPlusLowP3", "; M(#Lambda#pi^{-}) (GeV/c^{2}); #vec{#bf{p}}(K^{+}_{lowp}) (GeV/c)", 250, 1.25, 1.5, 100, 0, 10); 

	// Vertex hists
	dHist_XimInvariantMass_XimVertexZ = new TH2F("XimInvariantMass_XimVertex", "; M(#Lambda #pi^{-}) (GeV/c^{2}); Vertex-Z (cm)", 250, 1.25, 1.5, 200, 40, 240);
	dHist_XimInvariantMass_XimVertexZ_SignificanceCut = new TH2F("XimInvariantMass_XimVertex_SignificanceCut", "; M(#Lambda #pi^{-}) (GeV/c^{2}); Vertex-Z (cm)", 250, 1.25, 1.5, 200, 40, 240);
	dHist_XimInvariantMass_LambdaVertexZ = new TH2F("XimInvariantMass_LambdaVertex", "; M(#Lambda #pi^{-}) (GeV/c^{2}); Vertex-Z (cm)", 250, 1.25, 1.5, 200, 40, 240);
	dHist_XimInvariantMass_LambdaVertexZ_SignificanceCut = new TH2F("XimInvariantMass_LambdaVertexZ_SignificanceCut", "; M(#Lambda #pi^{-}) (GeV/c^{2}); Vertex-Z (cm)", 250, 1.25, 1.5, 200, 40, 240);
	
	// Lambda Mass
	dHist_LambdaInvariantMassMeasured = new TH1F("LambdaInvariantMassMeasured", "Invariant Mass; p #pi^{-}", 300, 1.0, 1.3);
	dHist_LambdaInvariantMassKinFit = new TH1F("LambdaInvariantMassKinFit", "Invariant Mass; p #pi^{-}", 300, 1.0, 1.3);

	// Ystar Mass
	dHist_YstarInvariantMass_nocuts = new TH1F("YstarInvariantMass_nocuts", " ; M(K^{+}_{lowp}#Xi^{-}) (GeV/c^{2}); ", 280, 1.7, 4.5);
	dHist_YstarInvariantMass_Measured_nocuts = new TH1F("YstarInvariantMass_Measured_nocuts", " ; M(K^{+}_{lowp}#Xi^{-}) (GeV/c^{2}); ", 280, 1.7, 4.5);
	dHist_YstarInvariantMass_Kp2Xim_nocuts = new  TH1F("YstarInvariantMass_Kp2Xim_nocuts", " ; M(K^{+}_{2}#Xi^{-}) (GeV/c^{2}); ", 280, 1.7, 4.5);
	dHist_YstarInvariantMass_Kp2Xim = new  TH1F("YstarInvariantMass_Kp2Xim", " ; M(K^{+}_{2}#Xi^{-}) (GeV/c^{2}); ", 280, 1.7, 4.5);
	dHist_YstarInvariantMass_Kp2Xim_XimCut = new  TH1F("YstarInvariantMass_Kp2Xim_XimCut", " ; M(K^{+}_{2}#Xi^{-}) (GeV/c^{2}); ", 280, 1.7, 4.5);
	dHist_YstarInvariantMass_KpThetaXim_nocuts = new  TH1F("YstarInvariantMass_KpThetaXim", " ; M(K^{+}_{#theta high}#Xi^{-}) (GeV/c^{2}); ", 280, 1.7, 4.5);
	dHist_YstarInvariantMass = new TH1F("YstarInvariantMass", " ; M(K^{+}_{lowp}#Xi^{-}) (GeV/c^{2}); ", 280, 1.7, 4.5);
	dHist_YstarInvariantMass_Measured = new TH1F("YstarInvariantMass_Measured", " ; M(K^{+}_{lowp}#Xi^{-}) (GeV/c^{2}); ", 280, 1.7, 4.5);
	dHist_YstarInvariantMass_XimCut = new TH1F("YstarInvariantMass_XimCut", " ; M(K^{+}_{lowp}#Xi^{-}) (GeV/c^{2}); ", 280, 1.7, 4.5);
	dHist_YstarInvariantMass_XimCut_Measured = new TH1F("YstarInvariantMass_XimCut_Measured", " ; M(K^{+}_{lowp}#Xi^{-}) (GeV/c^{2}); ", 280, 1.7, 4.5);
	dHist_YstarInvariantMass_SigmaRegion = new TH1F("YstarInvariantMass_SigmaRegion", " ; M(K^{+}_{lowp}#Xi^{-}) (GeV/c^{2}); ", 280, 1.7, 4.5);
	dHist_YstarMass_XiMinusMass = new TH2F("YstarMass_XiMinusMass", " ; M(K^{+}_{lowp}#Xi^{-}) (GeV/c^{2}); M(#Lambda #pi^{-}) (GeV/c^{2}) ", 280, 1.7, 4.5, 250, 1.25, 1.5);

        dHist_KPlusHighpPiMinus1_PiMinus2_InvariantMass = new TH2F("KPlusHighpPiMinus1_PiMinus2_InvariantMass"," ; M(K^{+}_{highp}#pi^{-}_{1}) (GeV/c^{2}); M(K^{+}_{highp}#pi^{-}_{2}) (GeV/c^{2})", 150, 0.5, 1.5, 150, 0.5,1.5); 
	dHist_XiMinusIM_LambdaIM = new TH2F("XiMinusIM_LambdaIM"," ; M(#Lambda#pi^{-}_{1}) (GeV/c^{2}); M(p#pi^{-}_{2}) (GeV/c^{2})", 250, 1.25, 1.5, 300, 1.0,1.3); 

       	// kaon kinematics
	dHist_KPlus1P3VsThetaMeasured = new TH2F("KPlus1P3VsThetaMeasured","KPlus1 Measured Kinematic ; #Theta; 3-Momentum", 180, 0, 180, 500, 0, 10);
	dHist_KPlus1P3VsTheta = new TH2F("KPlus1P3VsTheta","KPlus1 Kinematic ; #Theta; 3-Momentum", 180, 0, 180, 500, 0, 10);
	dHist_KPlus1P3VsTheta_CM = new TH2F("KPlus1P3VsThetaCoM","KPlus1 COM Kinematic ; #Theta; 3-Momentum", 180, 0, 180, 500, 0, 5);
	dHist_KPlus1P3VsTheta_XimCutIn = new TH2F("KPlus1P3VsTheta_XimCutIn","KPlus1 Kinematic ; #Theta; 3-Momentum", 180, 0, 180, 500, 0, 10);
	dHist_KPlus1P3VsTheta_XimCutOut = new TH2F("KPlus1P3VsTheta_XimCutOut","KPlus1 Kinematic ; #Theta; 3-Momentum", 180, 0, 180, 500, 0, 10);
	
	dHist_KPlus2P3VsThetaMeasured = new TH2F("KPlus2P3VsThetaMeasured","KPlus2 Measured Kinematic ; #Theta; 3-Momentum", 180, 0, 180, 500, 0, 10);
	dHist_KPlus2P3VsTheta = new TH2F("KPlus2P3VsTheta","KPlus2 Kinematic ; #Theta; 3-Momentum", 180, 0, 180, 500, 0, 10);
	dHist_KPlus2P3VsTheta_CM = new TH2F("KPlus2P3VsThetaCoM","KPlus2 COM Kinematic ; #Theta; 3-Momentum", 180, 0, 180, 500, 0, 5);
	dHist_KPlus2P3VsTheta_XimCutIn = new TH2F("KPlus2P3VsTheta_XimCutIn","KPlus2 Kinematic ; #Theta; 3-Momentum", 180, 0, 180, 500, 0, 10);
	dHist_KPlus2P3VsTheta_XimCutOut = new TH2F("KPlus2P3VsTheta_XimCutOut","KPlus2 Kinematic ; #Theta; 3-Momentum", 180, 0, 180, 500, 0, 10);

	// seperated kaons
	dHist_KPlusHighP3_Theta_Measured = new TH2F("KPlusHighP3ThetaMeasured", " ; #Theta; 3-Momentum", 180, 0, 180, 500, 0, 10);
	dHist_KPlusHighP3_Theta = new TH2F("KPlusHighP3Theta", " ; #Theta; 3-Momentum", 180, 0, 180, 500, 0, 10);
	dHist_KPlusHighP3_Theta_CM = new TH2F("KPlusHighP3ThetaCoM", " ; #Theta; 3-Momentum", 180, 0, 180, 500, 0, 10);
	
	dHist_KPlusHighP3_Theta_XimCutIn = new TH2F("KPlusHighP3Theta_XimCutIn", " ; #Theta; 3-Momentum", 180, 0, 180, 500, 0, 10);
	dHist_KPlusHighP3_Theta_XimCutOut = new TH2F("KPlusHighP3Theta_XimCutOut", " ; #Theta; 3-Momentum", 180, 0, 180, 500, 0, 10);
	dHist_KPlusHighP3_Theta_SigmaRegion = new TH2F("KPlusHighP3Theta_SigmaRegion", " ; #Theta; 3-Momentum", 180, 0, 180, 500, 0, 10);
	dHist_KPlusHighP3Phi = new TH2F("KPlusHighP3Phi"," ; #Phi; 3-Momentum (GeV/c)", 180, 0, 180, 500, 0, 10);
	
	dHist_KPlusLowP3_Theta_Measured = new TH2F("KPlusLowP3ThetaMeasured", " ; #Theta; 3-Momentum", 180, 0, 180, 500, 0, 10);
	dHist_KPlusLowP3_Theta = new TH2F("KPlusLowP3Theta", " ; #Theta; 3-Momentum", 180, 0, 180, 500, 0, 10);
	dHist_KPlusLowP3_Theta_CM = new TH2F("KPlusLowP3ThetaCoM", " ; #Theta; 3-Momentum", 180, 0, 180, 500, 0, 10);
	dHist_KPlusLowP3_Theta_XimCutIn = new TH2F("KPlusLowP3Theta_XimCutIn", " ; #Theta; 3-Momentum", 180, 0, 180, 500, 0, 10);	    
	dHist_KPlusLowP3_Theta_XimCutOut = new TH2F("KPlusLowP3Theta_XimCutOut", " ; #Theta; 3-Momentum", 180, 0, 180, 500, 0, 10);		

	dHist_KPlus1P3VsTheta_YstarCut = new TH2F("KPlus1P3Theta_YstarCut", " ; #Theta; 3-Momentum", 180, 0, 180, 500, 0, 10);
	dHist_KPlus2P3VsTheta_YstarCut = new TH2F("KPlus2P3Theta_YstarCut", " ; #Theta; 3-Momentum", 180, 0, 180, 500, 0, 10);

	dHist_KPlusHighP3_KPlusLowP3_t = new TH3F("KPlusHighP3_KPlusLowP3_t ", "; -t; #bf{p}(K^{+}_{highp}; #bf{p}(K^{+}_{lowp} ", 100, 0, 10, 100, 0, 10, 100, 0, 10);
	dHist_KPlus1P3_KPlus2P3_t = new TH3F("KPlus1P3_KPlus2P3_t ", " ; -t; #bf{p}(K^{+}_{1}; #bf{p}(K^{+}_{2} ", 100, 0, 10, 100, 0, 10, 100, 0, 10);

	// xim kinematics
	dHist_XimP3VsThetaMeasured = new TH2F("XimP3VsThetaMeasured","Measured Xi-Kinematic ; #Theta; 3-Momentum", 180, 0, 180, 500, 0, 10);
	dHist_XimP3VsTheta = new TH2F("XimP3VsTheta","Xim Kinematic ; #Theta; 3-Momentum", 180, 0, 180, 500, 0, 10);
	dHist_XimP3VsTheta_XimCutIn = new TH2F("XimP3VsTheta_XimCutIn","Xim Kinematic ; #Theta; 3-Momentum", 180, 0, 180, 500, 0, 10);
	dHist_XimP3VsTheta_XimCutOut = new TH2F("XimP3VsTheta_XimCutOut","Xim Kinematic ; #Theta; 3-Momentum", 180, 0, 180, 500, 0, 10);

	// lambda kinematics
	dHist_LambdaP3Theta = new TH2F("LambdaP3Theta","#Lambda Kinematics; #Theta; 3-Momentum", 500,0,100,500,0,10);
	dHist_LambdaP3ThetaMeasured = new TH2F("LambdaP3ThetaMeasured","#Measured Lambda Kinematics; #Theta; 3-Momentum", 500,0,100,500,0,10);
	dHist_LambdaP3Theta_XimCutIn = new TH2F("LambdaP3Theta_XimCutIn","#Lambda Kinematics; #Theta; 3-Momentum", 500,0,100,500,0,10);
	dHist_LambdaP3Theta_XimCutOut = new TH2F("LambdaP3Theta_XimCutOut","#Lambda Kinematics; #Theta; 3-Momentum", 500,0,100,500,0,10);
	dHist_LambdaP3Theta_CM = new TH2F("LambdaP3Theta_CM","#Lambda CM Kinematics; #Theta_{CM}; 3-Momentum", 180,0,180,500,0,10);
	dHist_LambdaP3Theta_HF = new TH2F("LambdaP3Theta_HF","#Lambda HF Kinematics; #Theta_{HF}; 3-Momentum", 180,0,180,500,0,10);
	dHist_LambdaP3CosTheta_HF = new TH2F("LambdaP3CosTheta_HF","#Lambda HF Kinematics; cos(#Theta_{HF}); 3-Momentum", 200, -1, 1, 500, 0, 10);
		
	// pion kinematics
	dHist_PiMinus1P3Theta = new TH2F("PiMinus1P3Theta","#pi^{-}_{1} Kinematics; #Theta, 3-Momentum", 180, 0, 180, 500, 0, 10);
	dHist_PiMinus1P3ThetaMeasured = new TH2F("PiMinus1P3ThetaMeasured","Measured #pi^{-}_{1} Kinematics; #Theta, 3-Momentum", 180, 0, 180, 500, 0, 10);	
	dHist_PiMinus1P3Theta_XimCutIn = new TH2F("PiMinus1P3Theta_XimCutIn","#pi^{-}_{1} Kinematics; #Theta, 3-Momentum", 180, 0, 180, 500, 0, 10);
	dHist_PiMinus1P3Theta_XimCutOut = new TH2F("PiMinus1P3Theta_XimCutOut","#pi^{-}_{1} Kinematics; #Theta, 3-Momentum", 180, 0, 180, 500, 0, 10);

	dHist_PiMinus1P3Theta_HF = new TH2F("PiMinus1P3VsTheta_HF","PiMinus1 HF Kinematic ; #Theta_{HF}; 3-Momentum", 180, 0, 180, 500, 0, 10);
	dHist_PiMinus1P3CosTheta_HF = new TH2F("PiMinus1P3VsCosTheta_HF","PiMinus1 HF Kinematic ; #Cos(#Theta_{HF}); 3-Momentum", 200, -1, 1, 500, 0, 10);
		
	dHist_PiMinus2P3Theta = new TH2F("PiMinus2P3Theta","#pi^{-}_{2} Kinematics; #Theta, 3-Momentum", 180, 0, 180, 500, 0, 10);
	dHist_PiMinus2P3ThetaMeasured = new TH2F("PiMinus2P3ThetaMeasured","Measured #pi^{-}_{2} Kinematics; #Theta, 3-Momentum", 180, 0, 180, 500, 0, 10);
	dHist_PiMinus2P3Theta_XimCutIn = new TH2F("PiMinus2P3Theta_XimCutIn","#pi^{-}_{2} Kinematics; #Theta, 3-Momentum", 180, 0, 180, 500, 0, 10);
	dHist_PiMinus2P3Theta_XimCutOut = new TH2F("PiMinus2P3Theta_XimCutOut","#pi^{-}_{2} Kinematics; #Theta, 3-Momentum", 180, 0, 180, 500, 0, 10);

       	// proton kinematics
	dHist_ProtonP3VsThetaMeasured = new TH2F("ProtonP3VsThetaMeasured", "Proton Measured Kinematics; #Theta; 3-Momentum ", 180, 0, 180, 500, 0, 10);
	dHist_ProtonP3VsTheta = new TH2F("ProtonP3VsTheta", "Proton Kinematics; #Theta; 3-Momentum ", 180, 0, 180, 500, 0, 10);
	dHist_ProtonP3VsTheta_XimCutIn = new TH2F("ProtonP3VsTheta_XimCutIn", "Proton Kinematics; #Theta; 3-Momentum ", 180, 0, 180, 500, 0, 10);
	dHist_ProtonP3VsTheta_XimCutOut = new TH2F("ProtonP3VsTheta_XimCutOut", "Proton Kinematics; #Theta; 3-Momentum ", 180, 0, 180, 500, 0, 10);

	// Ystar kinematics
	dHist_YstarP3Theta = new TH2F("YstarP3VsTheta","Ystar Kinematic ; #Theta; 3-Momentum", 180, 0, 180, 500, 0, 10);
	dHist_YstarP3Theta_CM = new TH2F("YstarP3VsTheta_CM","Ystar CoM Kinematic ; #Theta_{CM}; 3-Momentum", 180, 0, 180, 500, 0, 10);
		
	// Path Length
	dHist_XiPath_preCL = new TH1F("XiPathLength_preCL", ";#Xi^{-} Path Length (cm)", 100, 0.0, 50.0);
	dHist_XiPath_postCL = new TH1F("XiPathLength_postCL", ";#Xi^{-} Path Length (cm)", 100, 0.0, 50.0);
	dHist_XiLifetime_postCL = new TH1F("XiLifetime_postCL", ";#Xi^{-} Lifetime (ns)", 100, 0.0, 10.0);
	dHist_XiLifetimeRestFrame_postCL = new TH1F("XiLifetimeRestFrame_postCL", ";#Xi^{-} Lifetime (ns)", 150, 0.0, 3.0);
	dHist_XiMass_XiPathLength = new TH2F("XiMass_XiPathLength", "; M(#Lambda#pi^{-}) (GeV/c^{2}); #Xi^{-} Pathlength (cm)", 125 ,1.25, 1.5,100, 0.0, 50.0);
	dHist_XiMass_XiPathLengthSignificance = new TH2F("XiMass_XiPathLengthSignificance", "; M(#Lambda#pi^{-}) (GeV/c^{2}); #Xi^{-} Pathlength Significance)", 125, 1.25, 1.5, 500, 0.0, 50);
	dHist_XiMass_XiPathLength_SignificanceCut = new TH2F("XiMass_XiPathLength_SignificanceCut", "; M(#Lambda#pi^{-}) (GeV/c^{2}); #Xi^{-} Pathlength Significance)", 125 ,1.25, 1.5,200, 0.0, 100.0);	
	dHist_LambPath_preCL = new TH1F("LambPathLength_preCL", ";#Lambda Path Length (cm)", 100, 0.0, 50.0);
	dHist_LambPath_postCL = new TH1F("LambPathLength_postCL", ";#Lambda Path Length (cm)", 100, 0.0, 50.0);
	dHist_LambdaLifetime_postCL = new TH1F("LambdaLifetime_postCL", ";#Lambda Lifetime (ns)", 200, 0.0, 20.0);
	dHist_LambdaLifetimeRestFrame_postCL = new TH1F("LambdaLifetimeRestFrame_postCL", ";#Lambda Lifetime (ns)", 200, 0.0, 20.0);

	// Cross Section Plots
	dHist_XiIM_Egamma = new TH2F("XiIM_Egamma", " ; E_{#gamma#}; Xi^{-}_{IM}", 500, 6.4, 11.4, 250, 1.25, 1.5);
	
	// Low Energy  
	// dHist_XiIM_Egamma = new TH2F("XiIM_Egamma", " ; E_{#gamma#}; Xi^{-}_{IM}", 300, 3.0, 6.0, 250, 1.25, 1.5);
	
	// Diff Xsec plots
	dHist_Xi_Egamma_t = new TH3F("Xi_Egamma_t","; E_{#gamma} (GeV); M(#Lambda#pi^{-}) (GeV); -t (GeV/c)^{2}", 100, 6.4, 11.4, 250, 1.25, 1.5, 400, 0.0, 20.0);
		
	dHist_t_XimCut = new TH1F("t_distribution", "; -t (GeV^{2}); Counts", 400, 0.0, 20.0);
	dHist_t_XimCut_KaonHighCut = new TH1F("t_distribution_KaonHighMomCut", "; -t (GeV^{2}); Counts", 400, 0.0, 20.0);
	
	xi_egamma_name = new char[100];
	char *xi_egamma_name = new char[100];

	for(Int_t i=0; i < enBins; i++)
	  {
	    sprintf(xi_egamma_name, "XiIM_Egamma_arr_%.1f%.1f", arr_enBins[i], arr_enBins[i+1]);
	    dHist_XiIM_Egamma_arr[i] = new  TH1F( xi_egamma_name, "Counts / 2 MeV  ; M[#Xi^{-}(1320)] (Gev)", 125, 1.25, 1.5);
	  }

	//chisq
	dHist_ChiSqNDF = new TH1F("ChiSqNDF", " ; ChiSq; ", 750, 0, 150);
	dHist_ChiSqNDF_XimCut = new TH1F("ChiSq_XimCut", "; Fit #chi^{2}/NDF; Counts", 750, 0, 150);
	dHist_ChiSqNDF_XimSignal = new TH1F("ChiSq_XimSignal", "; Fit #chi^{2}/NDF; Counts", 750, 0, 150);
	dHist_ChiSqNDF_XimSideband = new TH1F("ChiSq_XimSideband", "; Fit #chi^{2}/NDF; Counts", 750, 0, 150);
	dHist_XimInvariantMassKF_ChiSqNdf = new TH2F("XimInvariantMassKF_ChiSqNdf", " ; M(#Lambda#pi^{-}) (GeV/c^{2}) ; Fit #chi^{2}/NDF", 250, 1.25, 1.5, 750, 0, 150);
	dHist_XimInvariantMassKF_ChiSqNdf_MissingMassSq = new TH3F("XimInvariantMassKF_ChiSqNdf_MissingMassSq", " ; M(#Lambda#pi^{-}) (GeV/c^{2}) ; Fit #chi^{2}/NDF", 125, 1.25, 1.5, 750, 0, 150, 100, -0.1, 0.1);
	
	//beam bunches
	dHist_BeamBunches = new TH1F("Beam_Bunches", "; RFtime (ns); Counts", 400, -20.0, 20.0);

	// number of tracks
	dHist_NumChargedTracksPerEvent = new TH1F("NumChargedTracksPerEvent", " ; Num Charged Tracks; Events", 10, 0, 10);
	dHist_NumNeutralTracksPerEvent = new TH1F("NumNeutralTracksPerEvent", " ; Num Neutral Tracks; Events", 10, 0, 10);
	dHist_NumCombosPerEvent = new TH1F("NumCombosPerEvent", " ; Num Combos; Events", 15, 0, 15);
	dHist_NumCombosSurviveCut = new TH1F("NumCombosSurviveCut", " ; Num Combos; Events", 10, 0, 10);
	dHist_NumUnusedChargedTracksPerCombo = new TH1F("NumUnusedChargedTracksPerCombo", " ; Num Charged Tracks; Combos", 10, 0, 10);
	dHist_NumUnusedNeutralTracksPerCombo = new TH1F("NumUnusedNeutralTracksPerCombo", " ; Num Neutral Tracks; Combos", 15, 0, 15);
	// dHist_NumUsedChargedTracksPerCombo = new TH1F("NumUsedChargedTracksPerCombo", " ; Num Charged Tracks; Combos", 10, 0, 10);
    // dHist_NumUsedNeutralTracksPerCombo = new TH1F("NumUsedNeutralTracksPerCombo", " ; Num Neutral Tracks; Combos", 10, 0, 10);
	dHist_NumReconChargedTracksPerEvent = new TH1F("NumUsedChargedTracksPerEvent", " ; Num Charged Tracks; Events", 15, 0, 15);
    dHist_NumReconNeutralTracksPerEvent = new TH1F("NumUsedNeutralTracksPerEvent", " ; Num Neutral Tracks; Events", 15, 0, 15);


	// Thrown Stuff
	dHist_KPlus1P3VsTruth = new TH2F("KPlus1P3VsTruth", " ; Truth #bf{#vec{p}}(K^{+}_{1}) (GeV/c); #bf{#vec{p}}(K^{+}_{1}) (GeV/c)", 100, 0, 10, 100, 0, 10);
	dHist_KPlus2P3VsTruth = new TH2F("KPlus2P3VsTruth", " ; Truth #bf{#vec{p}}(K^{+}_{2}) (GeV/c); #bf{#vec{p}}(K^{+}_{2}) (GeV/c)", 100, 0, 10, 100, 0, 10);
	dHist_KPlusHighP3VsTruth = new TH2F("KPlusHighP3VsTruth", " ; Truth #bf{#vec{p}}(K^{+}_{high}) (GeV/c); #bf{#vec{p}}(K^{+}_{high}) (GeV/c)", 100, 0, 10, 100, 0, 10);
	dHist_KPlusLowP3VsTruth = new TH2F("KPlusLowP3VsTruth", " ; Truth #bf{#vec{p}}(K^{+}_{low}) (GeV/c); #bf{#vec{p}}(K^{+}_{low}) (GeV/c)", 100, 0, 10, 100, 0, 10);
      
	//output files
	//kaonfile = new ofstream("kaon_kinematics.txt");
	//*kaonfile << "K1.p\t" << "K2.p\t" << "K1.theta\t" << "K2.theta" << endl;
	
	/************************** EXAMPLE USER INITIALIZATION: CUSTOM OUTPUT BRANCHES - MAIN TREE *************************/

	//EXAMPLE MAIN TREE CUSTOM BRANCHES (OUTPUT ROOT FILE NAME MUST FIRST BE GIVEN!!!! (ABOVE: TOP)):
	//The type for the branch must be included in the brackets
	//1st function argument is the name of the branch
	//2nd function argument is the name of the branch that contains the size of the array (for fundamentals only)
	/*
	dTreeInterface->Create_Branch_Fundamental<Int_t>("my_int"); //fundamental = char, int, float, double, etc.
	dTreeInterface->Create_Branch_FundamentalArray<Int_t>("my_int_array", "my_int");
	dTreeInterface->Create_Branch_FundamentalArray<Float_t>("my_combo_array", "NumCombos");
	dTreeInterface->Create_Branch_NoSplitTObject<TLorentzVector>("my_p4");
	dTreeInterface->Create_Branch_ClonesArray<TLorentzVector>("my_p4_array");
	*/

	/************************** EXAMPLE USER INITIALIZATION: CUSTOM OUTPUT BRANCHES - FLAT TREE *************************/

	//EXAMPLE FLAT TREE CUSTOM BRANCHES (OUTPUT ROOT FILE NAME MUST FIRST BE GIVEN!!!! (ABOVE: TOP)):
	//The type for the branch must be included in the brackets
	//1st function argument is the name of the branch
	//2nd function argument is the name of the branch that contains the size of the array (for fundamentals only)
	/* Examples
	dFlatTreeInterface->Create_Branch_Fundamental<Int_t>("flat_my_int"); //fundamental = char, int, float, double, etc.
	dFlatTreeInterface->Create_Branch_FundamentalArray<Int_t>("flat_my_int_array", "flat_my_int");
	dFlatTreeInterface->Create_Branch_NoSplitTObject<TLorentzVector>("flat_my_p4");
	dFlatTreeInterface->Create_Branch_ClonesArray<TLorentzVector>("flat_my_p4_array");
	*/
	
	// All measurements are kinfit unless specified by _meas
    // 4-vectors
    dFlatTreeInterface->Create_Branch_NoSplitTObject<TLorentzVector>("beam_p4");
    dFlatTreeInterface->Create_Branch_NoSplitTObject<TLorentzVector>("ystar_p4");    
    dFlatTreeInterface->Create_Branch_NoSplitTObject<TLorentzVector>("kphigh_fcal_p4");
    dFlatTreeInterface->Create_Branch_NoSplitTObject<TLorentzVector>("kphigh_p4");
    dFlatTreeInterface->Create_Branch_NoSplitTObject<TLorentzVector>("kplow_p4");
    dFlatTreeInterface->Create_Branch_NoSplitTObject<TLorentzVector>("kplow_p4_ystar_hf");
    dFlatTreeInterface->Create_Branch_NoSplitTObject<TLorentzVector>("kplow_bcal_p4");
    dFlatTreeInterface->Create_Branch_NoSplitTObject<TLorentzVector>("decayxim_p4");
    dFlatTreeInterface->Create_Branch_NoSplitTObject<TLorentzVector>("lambda_p4");
    
    // beam stuff
 	dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("beam_E");
    dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("beam_rfbunches");
	dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("beam_vertexX");
	dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("beam_vertexY");
	dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("beam_vertexZ");
	// kp1
    dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("kp1_P3");
    dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("kp1_P3_Truth");
    dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("kp1_CosTheta");
	dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("kp1_Phi");
	dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("kp_highp_P3");
    dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("kp_highp_Theta");
    dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("kp_highp_CosTheta");
	dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("kp_highp_Phi");
	// kp2
    dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("kp2_P3");	
	dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("kp2_P3_Truth");	
	dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("kp_lowp_P3");
    dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("kp_lowp_Theta");
    dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("kp2_YstarRest_CosTheta");
	dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("kp2_YstarRest_Phi");
    dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("kplow_costheta_hf");
	dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("kplow_phi_hf");
    // ystar
	dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("ystar_M");
	dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("ystar_P3");
	dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("ystar_Theta");
    // ximinus
	dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("decayxim_M"); 
    dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("xim_P3");
    dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("xim_vertexX");
	dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("xim_vertexY");
	dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("xim_vertexZ");
	dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("xim_pathlen");
	dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("xim_pathlensig");
	dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("xim_lifetime");
	dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("xim_lifetime_restframe");
	// lambda
    dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("decaylamb_M_meas");
    dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("lambda_P3");
	dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("lambda_vertexX");
	dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("lambda_vertexY");
	dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("lambda_vertexZ");
	dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("lambda_pathlen");
	dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("lambda_pathlensig");
	dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("lambda_lifetime");
	dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("lambda_lifetime_restframe");
    // pions
	dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("pim1_P3");
    dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("pim1_costheta_hf");
	dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("pim2_P3");
	// proton
    dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("proton_P3");
	// other
    dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("chisqndf");
	dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("confidencelvl");
    dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("total_mm2");
    dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("t_dist");
	dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("acc_weight");
	dFlatTreeInterface->Create_Branch_Fundamental<ULong64_t>("evnt_num");
    dFlatTreeInterface->Create_Branch_Fundamental<UInt_t>("combo_num");
    dFlatTreeInterface->Create_Branch_Fundamental<Int_t>("combos_survived");
    dFlatTreeInterface->Create_Branch_Fundamental<Int_t>("best_combo");
    dFlatTreeInterface->Create_Branch_Fundamental<Int_t>("best_combo_1");
    
	/************************************* ADVANCED EXAMPLE: CHOOSE BRANCHES TO READ ************************************/

	//TO SAVE PROCESSING TIME
		//If you know you don't need all of the branches/data, but just a subset of it, you can speed things up
		//By default, for each event, the data is retrieved for all branches
		//If you know you only need data for some branches, you can skip grabbing data from the branches you don't need
		//Do this by doing something similar to the commented code below

	//dTreeInterface->Clear_GetEntryBranches(); //now get none
	//dTreeInterface->Register_GetEntryBranch("Proton__P4"); //manually set the branches you want
}

Bool_t DSelector_kpkpxim::Process(Long64_t locEntry)
{
	// The Process() function is called for each entry in the tree. The entry argument
	// specifies which entry in the currently loaded tree is to be processed.
	//
	// This function should contain the "body" of the analysis. It can contain
	// simple or elaborate selection criteria, run algorithms on the data
	// of the event and typically fill histograms.
	//
	// The processing can be stopped by calling Abort().
	// Use fStatus to set the return value of TTree::Process().
	// The return value is currently not used.

	//CALL THIS FIRST
	DSelector::Process(locEntry); //Gets the data from the tree for the entry
	//cout << "RUN " << Get_RunNumber() << ", EVENT " << Get_EventNumber() << endl;
	//TLorentzVector locProductionX4 = Get_X4_Production();

	/******************************************** GET POLARIZATION ORIENTATION ******************************************/

	//Only if the run number changes
	//RCDB environment must be setup in order for this to work! (Will return false otherwise)
	UInt_t locRunNumber = Get_RunNumber();
	if(locRunNumber != dPreviousRunNumber)
	{
		dIsPolarizedFlag = dAnalysisUtilities.Get_IsPolarizedBeam(locRunNumber, dIsPARAFlag);
		dPreviousRunNumber = locRunNumber;
	}

	/********************************************* SETUP UNIQUENESS TRACKING ********************************************/

	//ANALYSIS ACTIONS: Reset uniqueness tracking for each action
	//For any actions that you are executing manually, be sure to call Reset_NewEvent() on them here
	Reset_Actions_NewEvent();
	dAnalyzeCutActions->Reset_NewEvent(); // manual action, must call Reset_NewEvent()

	//PREVENT-DOUBLE COUNTING WHEN HISTOGRAMMING
		//Sometimes, some content is the exact same between one combo and the next
			//e.g. maybe two combos have different beam particles, but the same data for the final-state
		//When histogramming, you don't want to double-count when this happens: artificially inflates your signal (or background)
		//So, for each quantity you histogram, keep track of what particles you used (for a given combo)
		//Then for each combo, just compare to what you used before, and make sure it's unique

	//EXAMPLE 1: Particle-specific info:
	set<Int_t> locUsedSoFar_BeamEnergy; //Int_t: Unique ID for beam particles. set: easy to use, fast to search

	//EXAMPLE 2: Combo-specific info:
		//In general: Could have multiple particles with the same PID: Use a set of Int_t's
		//In general: Multiple PIDs, so multiple sets: Contain within a map
		//Multiple combos: Contain maps within a set (easier, faster to search)
	set<map<Particle_t, set<Int_t> > > locUsedSoFar_MissingMass;
	set<map<Particle_t, set<Int_t> > > locUsedSoFar_FinalStateParticles;
	//INSERT USER ANALYSIS UNIQUENESS TRACKING HERE

	set<map<Particle_t, set<Int_t> > > locUsedSoFar_gXi;
	set<map<Particle_t, set<Int_t> > > locUsedSoFar_LambdaInvariantMass;
	set<map<Particle_t, set<Int_t> > > locUsedSoFar_ProtonKinematics;


 	/******************************************* LOOP OVER THROWN DATA (OPTIONAL) ***************************************/

	//Thrown beam: just use directly
	if(dThrownBeam != NULL)
		Double_t locEnergy = dThrownBeam->Get_P4().E();

	TLorentzVector locKPlus1P4_Truth;
	TLorentzVector locKPlus2P4_Truth;
	TLorentzVector locXiMinusP4_Truth;
	TLorentzVector locPiMinus1P4_Truth;
	TLorentzVector locPiMinus2P4_Truth;
	TLorentzVector locProtonP4_Truth;
	TLorentzVector locLambdaP4_Truth;
	
	//Loop over throwns
    for(UInt_t loc_j = 0; loc_j < Get_NumThrown(); ++loc_j)
	{
        //Set branch array indices corresponding to this particle
		dThrownWrapper->Set_ArrayIndex(loc_j);
		
		Particle_t locPID = dThrownWrapper->Get_PID();
		Int_t locParentIndex = dThrownWrapper->Get_ParentIndex();
		TLorentzVector locThrownP4 = dThrownWrapper->Get_P4();

		//cout << "Thrown " << loc_j << ": " << locPID << ", " << locThrownP4.Px() << ", " << locThrownP4.Py() << ", " << locThrownP4.Pz() << ", " << locThrownP4.E() << endl;
		//cout << "\t Parent Particle Index: " << locParentIndex << endl;

		if(locPID == 11) {
		  if(loc_j == 0) {locKPlus1P4_Truth = locThrownP4; }
		  if(loc_j == 1) {locKPlus2P4_Truth = locThrownP4; }
		}
		if(locPID == 9){
		  if(locParentIndex == 5) locPiMinus1P4_Truth = locThrownP4;
		  if(locParentIndex == 6) locPiMinus2P4_Truth = locThrownP4;
		}
		if(locPID == 14) locProtonP4_Truth = locThrownP4;		  
		if(locPID == 23) locXiMinusP4_Truth = locThrownP4; 
		if(locPID == 18) locLambdaP4_Truth = locThrownP4; 
		
		//Do stuff with the wrapper here ...
	}
	
	/**************************************** EXAMPLE: FILL CUSTOM OUTPUT BRANCHES **************************************/

	/*
	Int_t locMyInt = 7;
	dTreeInterface->Fill_Fundamental<Int_t>("my_int", locMyInt);

	TLorentzVector locMyP4(4.0, 3.0, 2.0, 1.0);
	dTreeInterface->Fill_TObject<TLorentzVector>("my_p4", locMyP4);

	for(int loc_i = 0; loc_i < locMyInt; ++loc_i)
		dTreeInterface->Fill_Fundamental<Int_t>("my_int_array", 3*loc_i, loc_i); //2nd argument = value, 3rd = array index
	*/

	/************************************************* LOOP OVER COMBOS *************************************************/

	// Set up tacking plots
	Int_t locNumChargedHypos = Get_NumChargedHypos();
	Int_t locNumNeutralHypos = Get_NumNeutralHypos();
	Int_t locNumCombos = Get_NumCombos();
	Int_t locNumComboSurvivedCut = 0;
    Double_t locBestChiSqNdf = 0;//best chisq is closest to 1
    UInt_t locBestChiSqNdfComboNum = 0;
    Double_t locBestChiSqNdf_1 = 0;
    UInt_t locBestChiSqNdfComboNum_1 = 0;
    ULong64_t locEventNum = Get_EventNumber();

	// Plots for events
	dHist_NumChargedTracksPerEvent->Fill(locNumChargedHypos);
	dHist_NumNeutralTracksPerEvent->Fill(locNumNeutralHypos);
	dHist_NumCombosPerEvent->Fill(locNumCombos);

	//Loop over combos for cuts
	for(UInt_t loc_i = 0; loc_i < Get_NumCombos(); ++loc_i)
	{
		//Set branch array indices for combo and all combo particles
		dComboWrapper->Set_ComboIndex(loc_i);

		// Is used to indicate when combos have been cut
		if(dComboWrapper->Get_IsComboCut()) // Is false when tree originally created
		        continue; // Combo has been cut previously
		

		/********************************************** GET PARTICLE INDICES *********************************************/

		//Used for tracking uniqueness when filling histograms, and for determining unused particles

		//Step 0
		Int_t locBeamID = dComboBeamWrapper->Get_BeamID();
		Int_t locKPlus1TrackID = dKPlus1Wrapper->Get_TrackID();
		Int_t locKPlus2TrackID = dKPlus2Wrapper->Get_TrackID();

		//Step 1
		Int_t locPiMinus1TrackID = dPiMinus1Wrapper->Get_TrackID();

		//Step 2
		Int_t locPiMinus2TrackID = dPiMinus2Wrapper->Get_TrackID();
		Int_t locProtonTrackID = dProtonWrapper->Get_TrackID();

		/*********************************************** GET FOUR-MOMENTUM **********************************************/

		// Get P4's: //is kinfit if kinfit performed, else is measured
		//dTargetP4 is target p4
		//Step 0
		TLorentzVector locBeamP4 = dComboBeamWrapper->Get_P4();
		TLorentzVector locKPlus1P4 = dKPlus1Wrapper->Get_P4();
		TLorentzVector locKPlus2P4 = dKPlus2Wrapper->Get_P4();
		//Step 1
		TLorentzVector locPiMinus1P4 = dPiMinus1Wrapper->Get_P4();
		//Step 2
		TLorentzVector locDecayingLambdaP4 = dDecayingLambdaWrapper->Get_P4();
		TLorentzVector locPiMinus2P4 = dPiMinus2Wrapper->Get_P4();
		TLorentzVector locProtonP4 = dProtonWrapper->Get_P4();

		// Get Measured P4's:
		//Step 0
		TLorentzVector locBeamP4_Measured = dComboBeamWrapper->Get_P4_Measured();
		TLorentzVector locKPlus1P4_Measured = dKPlus1Wrapper->Get_P4_Measured();
		TLorentzVector locKPlus2P4_Measured = dKPlus2Wrapper->Get_P4_Measured();
		//Step 1
		TLorentzVector locPiMinus1P4_Measured = dPiMinus1Wrapper->Get_P4_Measured();
		//Step 2
		TLorentzVector locPiMinus2P4_Measured = dPiMinus2Wrapper->Get_P4_Measured();
		TLorentzVector locProtonP4_Measured = dProtonWrapper->Get_P4_Measured();

		/********************************************* GET COMBO RF TIMING INFO *****************************************/

        TLorentzVector locBeamX4_Measured = dComboBeamWrapper->Get_X4_Measured();
        Double_t locBunchPeriod = dAnalysisUtilities.Get_BeamBunchPeriod(Get_RunNumber());
        Double_t locDeltaT_RF = dAnalysisUtilities.Get_DeltaT_RF(Get_RunNumber(), locBeamX4_Measured, dComboWrapper);
        Int_t locRelBeamBucket = dAnalysisUtilities.Get_RelativeBeamBucket(Get_RunNumber(), locBeamX4_Measured, dComboWrapper); // 0 for in-time events, non-zero integer for out-of-time photons
        Int_t locNumOutOfTimeBunchesInTree = 1; //YOU need to specify this number
        //Number of out-of-time beam bunches in tree (on a single side, so that total number out-of-time bunches accepted is 2 times this number for left + right bunches) 
                
        Bool_t locSkipNearestOutOfTimeBunch = false; // True: skip events from nearest out-of-time bunch on either side (recommended).
        Int_t locNumOutOfTimeBunchesToUse = locNumOutOfTimeBunchesInTree>1 ? locNumOutOfTimeBunchesInTree-1:locNumOutOfTimeBunchesInTree;
        Double_t locAccidentalScalingFactor = dAnalysisUtilities.Get_AccidentalScalingFactor(Get_RunNumber(), locBeamP4.E()); // Ideal value would be 1, but deviations observed: need added factor.
        Double_t locAccidentalScalingFactorError = dAnalysisUtilities.Get_AccidentalScalingFactorError(Get_RunNumber(), locBeamP4.E()); // Ideal value would be 1, but deviations observed, need added factor.
        Double_t locHistAccidWeightFactor = locRelBeamBucket==0 ? 1 : -locAccidentalScalingFactor/(2*locNumOutOfTimeBunchesToUse) ; // Weight by 1 for in-time events, ScalingFactor*(1/NBunches) for out-of-time
        //if(locSkipNearestOutOfTimeBunch && abs(locRelBeamBucket)==1 && locNumOutOfTimeBunchesInTree>1) continue; // Skip nearest out-of-time bunch: tails of in-time distribution also leak in
		
        // skip all out of time beam bunches for best combo
        if(fabs(locRelBeamBucket)>0) continue;

		/********************************************* COMBINE FOUR-MOMENTUM ********************************************/

		// DO YOUR STUFF HERE

		// Accidental Scale Factor
		//double scaling_factor = dAnalysisUtilities.Get_AccidentalScalingFactor(locRunNumber, locBeamP4.E());
		//double scaling_factor_err = dAnalysisUtilities.Get_AccidentalScalingFactorError(locRunNumber, locBeamP4.E());
		
		// Combine 4-vectors
		// Kaon Determincation
		TLorentzVector locKPlusP4_lowp;
		TLorentzVector locKPlusP4_highp;
		TLorentzVector locKPlusP4_lowp_Measured;
		TLorentzVector locKPlusP4_highp_Measured;
		TLorentzVector locKPlusP4_hightheta;
		TLorentzVector locKPlusP4_lowtheta;
		TLorentzVector locKPlusP4_hightheta_Measured;
		TLorentzVector locKPlusP4_lowtheta_Measured;
		Int_t locKhighTrackID;
		Int_t locKlowTrackID;
		
		//Kaon Seperation with only angle and momentum
		if(locKPlus1P4.P() > locKPlus2P4.P() )
		  {
		    locKPlusP4_highp = locKPlus1P4; 
		    locKPlusP4_lowp = locKPlus2P4;
		    locKhighTrackID = locKPlus1TrackID;
		    locKlowTrackID = locKPlus2TrackID;
		    locKPlusP4_highp_Measured = locKPlus1P4_Measured; 
		    locKPlusP4_lowp_Measured = locKPlus2P4_Measured;
		  }
		else
		  {
		    locKPlusP4_highp = locKPlus2P4; 
		    locKPlusP4_lowp = locKPlus1P4;
		    locKhighTrackID = locKPlus2TrackID;
		    locKlowTrackID = locKPlus1TrackID;
		    locKPlusP4_highp_Measured = locKPlus2P4_Measured; 
		    locKPlusP4_lowp_Measured = locKPlus1P4_Measured;
		  }
		
		//Kaon Seperation with only angle
		if(locKPlus1P4.Theta() < locKPlus2P4.Theta() && 180/TMath::Pi()*locKPlus1P4.Theta() < 40  )
		  {
		    locKPlusP4_lowtheta = locKPlus1P4; 
		    locKPlusP4_hightheta = locKPlus2P4;
		    locKhighTrackID = locKPlus1TrackID;
		    locKlowTrackID = locKPlus2TrackID;
		    locKPlusP4_lowtheta_Measured = locKPlus1P4_Measured; 
		    locKPlusP4_hightheta_Measured = locKPlus2P4_Measured;
		  }
		else if(locKPlus1P4.Theta() > locKPlus2P4.Theta())
		  {
		    locKPlusP4_lowtheta = locKPlus2P4; 
		    locKPlusP4_hightheta = locKPlus1P4;
		    locKhighTrackID = locKPlus2TrackID;
		    locKlowTrackID = locKPlus1TrackID;
		    locKPlusP4_lowtheta_Measured = locKPlus2P4_Measured; 
		    locKPlusP4_hightheta_Measured = locKPlus1P4_Measured;
		  }


		Double_t locT = (locBeamP4-locKPlusP4_highp).M2();
		Double_t locT_K1 = (locBeamP4-locKPlus1P4).M2();

		TLorentzVector locMissingP4_Measured = locBeamP4_Measured + dTargetP4;
		TLorentzVector locDecayingLambdaP4_Measured = locProtonP4_Measured + locPiMinus2P4_Measured;
	  		
		TLorentzVector locXiMinusP4 = locDecayingLambdaP4 + locPiMinus1P4;
		TLorentzVector locXiMinusP4_Measured = locDecayingLambdaP4_Measured + locPiMinus1P4_Measured;  
		  
		TLorentzVector locYstar_Kp2Xim_P4 = locKPlus2P4 + locXiMinusP4;
		TLorentzVector locYstarP4 = locKPlusP4_lowp + locXiMinusP4;
		TLorentzVector locYstar_KpThetaXim_P4 = locKPlusP4_hightheta + locXiMinusP4;
		TLorentzVector locYstarP4_Measured = locKPlusP4_lowp_Measured + locXiMinusP4_Measured;

		TLorentzVector locKPlusHigh_PiMinus1_P4 = locKPlusP4_highp + locPiMinus1P4;
		TLorentzVector locKPlusHigh_PiMinus2_P4 = locKPlusP4_highp + locPiMinus2P4;

		TLorentzVector locKPlusLow_PiMinus1_P4 = locKPlusP4_lowp + locPiMinus1P4;
		TLorentzVector locKPlusLow_PiMinus2_P4 = locKPlusP4_lowp + locPiMinus2P4;        

		locMissingP4_Measured -= locKPlus1P4_Measured + locKPlus2P4_Measured + locPiMinus1P4_Measured + locPiMinus2P4_Measured + locProtonP4_Measured;

		/**************************************************
                          Boost into different frames
		***************************************************/
		//BoostVector in COM frame
		TLorentzVector locCoMP4 = locBeamP4 + dTargetP4;
		TVector3 boostCoM = locCoMP4.BoostVector();
		//Initalize 4Vectors in COM frame
		//step 1
		TLorentzVector locBeamP4_CM = locBeamP4;
		//step 2
		TLorentzVector locKPlus1P4_CM = locKPlus1P4;
		TLorentzVector locKPlusP4_highp_CM = locKPlusP4_highp;
		TLorentzVector locKPlusP4_lowtheta_CM = locKPlusP4_lowtheta;
		TLorentzVector locYstarP4_CM = locYstarP4;
		//step 3
		TLorentzVector locXiP4_CM = locXiMinusP4;
		TLorentzVector locKPlus2P4_CM = locKPlus2P4;
		TLorentzVector locKPlusP4_lowp_CM = locKPlusP4_lowp;
		TLorentzVector locKPlusP4_hightheta_CM = locKPlusP4_hightheta;
		TLorentzVector locPiMinus1P4_CM = locPiMinus1P4;
		//step 4
		TLorentzVector locDecayingLambdaP4_CM = locDecayingLambdaP4;
		//Boost 4vectors in Com frame
		locBeamP4_CM.Boost(-boostCoM);
		locYstarP4_CM.Boost(-boostCoM);
		locKPlus1P4_CM.Boost(-boostCoM);
		locKPlusP4_highp_CM.Boost(-boostCoM);
		locKPlusP4_lowtheta_CM.Boost(-boostCoM);
		locKPlus2P4_CM.Boost(-boostCoM);
		locKPlusP4_lowp_CM.Boost(-boostCoM);
		locKPlusP4_hightheta_CM.Boost(-boostCoM);
		locXiP4_CM.Boost(-boostCoM);
		locPiMinus1P4_CM.Boost(-boostCoM);
		locDecayingLambdaP4_CM.Boost(-boostCoM);
		
		//Boost 4vector into rest frame of ystar from CoM 
		TVector3 boostYstar_Rest = locYstarP4_CM.BoostVector();
		//Initialize 4vectors in ystar rest frame
		//step 1
		TLorentzVector locBeamP4_YstarRest = locBeamP4_CM;
		//step 2
		TLorentzVector locKPlus1P4_YstarRest = locKPlus1P4_CM;
		TLorentzVector locKPlusP4_highp_YstarRest = locKPlusP4_highp_CM;
		TLorentzVector locKPlusP4_lowtheta_YstarRest = locKPlusP4_lowtheta_CM;
		TLorentzVector locYstarP4_YstarRest = locYstarP4_CM;
		//step 3
		TLorentzVector locXiP4_YstarRest = locXiP4_CM;
		TLorentzVector locKPlus2P4_YstarRest = locKPlus2P4_CM;
		TLorentzVector locKPlusP4_lowp_YstarRest = locKPlusP4_lowp_CM;
		TLorentzVector locKPlusP4_hightheta_YstarRest = locKPlusP4_hightheta_CM;
		TLorentzVector locPiMinus1P4_YstarRest = locPiMinus1P4_CM;
		//step 4
		TLorentzVector locDecayingLambdaP4_YstarRest = locDecayingLambdaP4_CM;
		//Boost 4vectos to rest frame of ystar
		locBeamP4_YstarRest.Boost(-boostYstar_Rest);
		locYstarP4_YstarRest.Boost(-boostYstar_Rest);
		locKPlus1P4_YstarRest.Boost(-boostYstar_Rest);
		locKPlusP4_highp_YstarRest.Boost(-boostYstar_Rest);
		locKPlusP4_lowtheta_YstarRest.Boost(-boostYstar_Rest);
		locKPlus2P4_YstarRest.Boost(-boostYstar_Rest);
		locKPlusP4_lowp_YstarRest.Boost(-boostYstar_Rest);
		locKPlusP4_hightheta_YstarRest.Boost(-boostYstar_Rest);
		locXiP4_YstarRest.Boost(-boostYstar_Rest);
		locPiMinus1P4_YstarRest.Boost(-boostYstar_Rest);
		locDecayingLambdaP4_YstarRest.Boost(-boostYstar_Rest);

        // Set helicity frame for ystar
        TVector3 z_hat_Ystar_HF = locYstarP4_CM.Vect().Unit();
        TVector3 y_hat_Ystar_HF = locBeamP4_CM.Vect().Cross(locYstarP4_CM.Vect()).Unit();//y direction normal to production plane gammap
        TVector3 x_hat_Ystar_HF = y_hat_Ystar_HF.Cross(z_hat_Ystar_HF);//maintain right handed coordinate system		
        TLorentzVector locKPlusP4_slow_HF(locKPlusP4_lowp_YstarRest.E(),locKPlusP4_lowp_YstarRest.Vect().Dot(x_hat_Ystar_HF), locKPlusP4_lowp_YstarRest.Vect().Dot(y_hat_Ystar_HF), locKPlusP4_lowp_YstarRest.Vect().Dot(z_hat_Ystar_HF)); 
        Double_t KPlusLow_CosTheta_HF = locKPlusP4_lowp_YstarRest.Pz() / locKPlusP4_lowp_YstarRest.Vect().Mag();
        Double_t KPlusLow_Phi_HF = TMath::ATan(locKPlusP4_lowp_YstarRest.Py() / locKPlusP4_lowp_YstarRest.Px());

		//BoostVector in Xim rest frame from ystar rest frame
		TVector3 boostXim_HF = locXiP4_YstarRest.BoostVector();
		//Initalize 4Vectors in Xim Rest frame
		//step 1
		TLorentzVector locBeamP4_HF = locBeamP4_YstarRest;
		//step 2
		TLorentzVector locKPlus1P4_HF = locKPlus1P4_YstarRest;
		TLorentzVector locKPlusP4_highp_HF = locKPlusP4_highp_YstarRest;
		TLorentzVector locYstarP4_HF = locYstarP4_YstarRest;
		//step 3
		TLorentzVector locXiP4_HF = locXiP4_YstarRest;
		TLorentzVector locKPlus2P4_HF = locKPlus2P4_YstarRest;
		TLorentzVector locKPlusP4_lowp_HF = locKPlusP4_lowp_YstarRest;
		TLorentzVector locPiMinus1P4_HF = locPiMinus1P4_YstarRest;
		//step 4
		TLorentzVector locDecayingLambdaP4_HF = locDecayingLambdaP4_YstarRest;
		//Boost 4vectors in Xim Rest frame
		locBeamP4_HF.Boost(-boostXim_HF);
		locKPlus1P4_HF.Boost(-boostXim_HF);
		locKPlusP4_highp_HF.Boost(-boostXim_HF);
		locKPlus2P4_HF.Boost(-boostXim_HF);
		locKPlusP4_lowp_HF.Boost(-boostXim_HF);
		locXiP4_HF.Boost(-boostXim_HF);
		locPiMinus1P4_HF.Boost(-boostXim_HF);
		locDecayingLambdaP4_HF.Boost(-boostXim_HF);
		//Create the GF reference frame
		TVector3 z_hat_HF = locXiP4_YstarRest.Vect().Unit();//z direction is opposite direction of the boost or Ystar in rest frame
        TVector3 y_hat_HF = locXiP4_YstarRest.Vect().Cross(locPiMinus1P4_YstarRest.Vect()).Unit();//y direction normal to production plane
        TVector3 x_hat_HF = y_hat_HF.Cross(z_hat_HF);//maintain right handed coordinate system		
        //Define angle of Klus2 and XiMinus in HF
        TVector3 locPiMinus1P3_HF(locPiMinus1P4_HF.Vect().Dot(x_hat_HF), locPiMinus1P4_HF.Vect().Dot(y_hat_HF), locPiMinus1P4_HF.Vect().Dot(z_hat_HF)); 
        double PiMinus1_Angle_HF = z_hat_HF.Angle(locPiMinus1P4_HF.Vect());
        double PiMinus1_CosTheta_HF = locPiMinus1P3_HF.Z() / locPiMinus1P3_HF.Mag();
		Double_t Lambda_Angle_HF = z_hat_HF.Angle(locDecayingLambdaP4_HF.Vect());
		Double_t Lambda_CosAngle_HF = TMath::Cos(z_hat_HF.Angle(locPiMinus1P4_HF.Vect()));


		/*******************************
                    Path Length Calculations
		*******************************/
		//TLorentzVector locDecayingLambdaP4 = locPiMinus2P4 +	locProtonP4; //Decaying Lambda for M18 is manually calculated
		TLorentzVector locProdSpacetimeVertex = dComboBeamWrapper->Get_X4();//Get production vertex
		// Xi Path Length
		TLorentzVector locDecayingXiX4 = dTreeInterface->Get_TObject<TLorentzVector>("DecayingXiMinus__X4",loc_i);
		TLorentzVector locDeltaSpacetimeXi = locProdSpacetimeVertex - locDecayingXiX4;//vertex difference
		Double_t locPathLengthXi = locDeltaSpacetimeXi.Vect().Mag();//pathlength is just the magnitude
		Float_t locPathLengthSigmaXi = Get_Fundamental<Float_t>("DecayingXiMinus__PathLengthSigma", loc_i);
		Double_t locPathLengthSignificanceXi = locPathLengthXi/locPathLengthSigmaXi;
        Double_t locLifetimeXi = locDeltaSpacetimeXi.T();//lifetime of xi in lab
		Double_t locLifetimeRestFrameXi = locPathLengthXi*locXiMinusP4.M() / (29.9792458 * locXiMinusP4.P());//lifetime of xi in restframe t = (travel distance)*(xi mass)/(xi mom) // add 1/c[cm/ns] to correct dimensionality

		// Lambda Path Length
		TLorentzVector locDecayingLambdaX4 = dDecayingLambdaWrapper->Get_X4(); //Doesn't exist for M18
		TLorentzVector locDeltaSpacetimeLambda = locDecayingXiX4 - locDecayingLambdaX4;//vertex difference
		Double_t locPathLengthLambda = locDeltaSpacetimeLambda.Vect().Mag();//pathlength is just the magnitude		
		Float_t locPathLengthSigmaLambda = dDecayingLambdaWrapper->Get_PathLengthSigma();
		Double_t locPathLengthSignificanceLambda = locPathLengthLambda/locPathLengthSigmaLambda;
		Double_t locLifetimeLambda = locDeltaSpacetimeLambda.T();//lifetime of lamb in lab
		Double_t locLifetimeRestFrameLambda = locPathLengthLambda*locDecayingLambdaP4.M() / (29.9792458 * locDecayingLambdaP4.P());//lifetime of lamb in restframe 

		/******************************************** Get FOUR-POSITION **************************************************/

		//TLorentzVector loc_beamX4 = dComboBeamWrapper-> GetX4();


		/**************************************** EXAMPLE: FILL CUSTOM OUTPUT BRANCHES **************************************/

		/*
		TLorentzVector locMyComboP4(8.0, 7.0, 6.0, 5.0);
		//for arrays below: 2nd argument is value, 3rd is array index
		//NOTE: By filling here, AFTER the cuts above, some indices won't be updated (and will be whatever th   were from the last event)
			//So, when you draw the branch, be sure to cut on "IsComboCut" to avoid these.
		dTreeInterface->Fill_Fundamental<Float_t>("my_combo_array", -2*loc_i, loc_i);
		dTreeInterface->Fill_TObject<TLorentzVector>("my_p4_array", locMyComboP4, loc_i);
		*/
		
		//<----
		
		/**************************************** EXAMPLE: HISTOGRAM BEAM ENERGY *****************************************/

		//Histogram beam energy (if haven't already)
		if(locUsedSoFar_BeamEnergy.find(locBeamID) == locUsedSoFar_BeamEnergy.end())
          {
			dHist_BeamEnergy->Fill(locBeamP4.E());
			dHist_BeamBunches->Fill(locDeltaT_RF);
			locUsedSoFar_BeamEnergy.insert(locBeamID);
          }

		/************************************ EXAMPLE: HISTOGRAM MISSING MASS SQUARED ************************************/

        //Uniqueness tracking: Build the map of particles used for the missing mass
        //For beam: Don't want to group with final-state photons. Instead use "Unknown" PID (not ideal, but it's easy).

		map<Particle_t, set<Int_t> > locUsedThisCombo_MissingMass;
		locUsedThisCombo_MissingMass[Unknown].insert(locBeamID); //beam
		locUsedThisCombo_MissingMass[KPlus].insert(locKPlus1TrackID);
		locUsedThisCombo_MissingMass[KPlus].insert(locKPlus2TrackID);
		locUsedThisCombo_MissingMass[PiMinus].insert(locPiMinus1TrackID);
		locUsedThisCombo_MissingMass[PiMinus].insert(locPiMinus2TrackID);
		locUsedThisCombo_MissingMass[Proton].insert(locProtonTrackID);

		map<Particle_t, set<Int_t> > locUsedThisCombo_FinalStateParticles;
		locUsedThisCombo_FinalStateParticles[Unknown].insert(locBeamID); //beam
		locUsedThisCombo_FinalStateParticles[KPlus].insert(locKPlus1TrackID);
		locUsedThisCombo_FinalStateParticles[KPlus].insert(locKPlus2TrackID);
		locUsedThisCombo_FinalStateParticles[PiMinus].insert(locPiMinus1TrackID);
		locUsedThisCombo_FinalStateParticles[PiMinus].insert(locPiMinus2TrackID);
		locUsedThisCombo_FinalStateParticles[Proton].insert(locProtonTrackID);

		map<Particle_t, set<Int_t> > locUsedThisCombo_gXi;
		locUsedThisCombo_gXi[Unknown].insert(locBeamID); //beam
		locUsedThisCombo_gXi[PiMinus].insert(locPiMinus1TrackID);
		locUsedThisCombo_gXi[PiMinus].insert(locPiMinus2TrackID);
		locUsedThisCombo_gXi[Proton].insert(locProtonTrackID);

		//Missing Mass Squared
		Double_t locMissingMassSquared = locMissingP4_Measured.M2();

		//ChiSqNDF
		Double_t locChiSqNdf = dComboWrapper->Get_ChiSq_KinFit("") / dComboWrapper->Get_NDF_KinFit("");
		//Double_t locNDF = dComboWrapper->Get_NDF_KinFit("");
		//Double_t locChiSqNdf = locChiSq/locNDF;
        Double_t locConfidenceLvl = dComboWrapper->Get_ConfidenceLevel_KinFit("");

        // Pre-cut isnan chisqndf cut
        if( std::isnan(locChiSqNdf) )
		{
          dComboWrapper->Set_IsComboCut(true);
          locUsedSoFar_gXi.insert(locUsedThisCombo_gXi);
          locUsedSoFar_MissingMass.insert(locUsedThisCombo_MissingMass);
          locUsedSoFar_FinalStateParticles.insert(locUsedThisCombo_FinalStateParticles);
          continue;
		}

		// Pre-cut histograms
		if(locUsedSoFar_gXi.find(locUsedThisCombo_gXi) == locUsedSoFar_gXi.end())
		  {
            //unique missing mass combo: histogram it, and register this combo of particles		  
            if(locXiMinusP4.M() > 1.308 && locXiMinusP4.M() < 1.334)
              {
                dHist_XiPath_preCL->Fill(locPathLengthXi, locHistAccidWeightFactor);
                dHist_ChiSqNDF_XimSignal->Fill(locChiSqNdf, locHistAccidWeightFactor);
              }
			
            dHist_LambPath_preCL->Fill(locPathLengthLambda, locHistAccidWeightFactor);
            dHist_XimInvariantMassKF_ChiSqNdf->Fill(locXiMinusP4.M(), locChiSqNdf, locHistAccidWeightFactor);
			dHist_XimInvariantMassKF_ChiSqNdf_MissingMassSq->Fill(locXiMinusP4.M(), locChiSqNdf, locMissingMassSquared, locHistAccidWeightFactor);
		  }
 
		if(locUsedSoFar_MissingMass.find(locUsedThisCombo_MissingMass) == locUsedSoFar_MissingMass.end())
		  {
            dHist_ChiSqNDF->Fill(locChiSqNdf);
            //plot missing mass with chisqcut
            if(locChiSqNdf < 4.0)
              {
                dHist_MissingMassSquared->Fill(locMissingMassSquared, locHistAccidWeightFactor);
              }
            
			dHist_YstarInvariantMass_Kp2Xim_nocuts->Fill(locYstar_Kp2Xim_P4.M(), locHistAccidWeightFactor);
			dHist_YstarInvariantMass_KpThetaXim_nocuts->Fill(locYstar_KpThetaXim_P4.M(), locHistAccidWeightFactor);
			dHist_YstarInvariantMass_nocuts->Fill(locYstarP4.M(), locHistAccidWeightFactor);
			dHist_YstarInvariantMass_Measured_nocuts->Fill(locYstarP4_Measured.M(), locHistAccidWeightFactor);
		    
			//Plot chisq fro xisignal and sideband region
			if(fabs(locMissingMassSquared) < 0.02)
			  {
			    //signal regionxb
			    if(locXiMinusP4.M() < 1.338 && locXiMinusP4.M() > 1.308)
			      {
                    dHist_ChiSqNDF_XimCut->Fill(locChiSqNdf, locHistAccidWeightFactor);
			      }
			    //sideband region
			    else if((locXiMinusP4.M() < 1.302 && locXiMinusP4.M() > 1.270) || (locXiMinusP4.M() < 1.462 && locXiMinusP4.M() > 1.430))
			      {
			        dHist_ChiSqNDF_XimSideband->Fill(locChiSqNdf, locHistAccidWeightFactor);
			      }
			  }
		  }
        
		/*****************************************************
                             Perform Universal Cuts
        *****************************************************/
		//E.g. Cut
		if(locChiSqNdf > 4 )
		{
          dComboWrapper->Set_IsComboCut(true);
          locUsedSoFar_gXi.insert(locUsedThisCombo_gXi);
          locUsedSoFar_MissingMass.insert(locUsedThisCombo_MissingMass);
          //locUsedSoFar_FinalStateParticles.insert(locUsedThisCombo_FinalStateParticles);
          continue;
		}
		//E.g. Cut
		if(fabs(locMissingMassSquared) > 0.02 )
		{
          dComboWrapper->Set_IsComboCut(true);
          locUsedSoFar_gXi.insert(locUsedThisCombo_gXi);
          locUsedSoFar_MissingMass.insert(locUsedThisCombo_MissingMass);			
          //locUsedSoFar_FinalStateParticles.insert(locUsedThisCombo_FinalStateParticles);
          continue;
		}
		// Beam Energy 
		if(locBeamP4.E() < 6.4 && locBeamP4.E() > 11.4)
		{
          dComboWrapper->Set_IsComboCut(true);
          locUsedSoFar_gXi.insert(locUsedThisCombo_gXi);
          locUsedSoFar_MissingMass.insert(locUsedThisCombo_MissingMass);			
          //locUsedSoFar_FinalStateParticles.insert(locUsedThisCombo_FinalStateParticles);
          continue;
		}
        // vertex cuts
		if(locProdSpacetimeVertex.Z() > 50.4 && locProdSpacetimeVertex.Z() < 79.1 && locDecayingXiX4.Z() > locDecayingLambdaX4.Z())
		{
          dComboWrapper->Set_IsComboCut(true);
          locUsedSoFar_gXi.insert(locUsedThisCombo_gXi);
          locUsedSoFar_MissingMass.insert(locUsedThisCombo_MissingMass);			
          //locUsedSoFar_FinalStateParticles.insert(locUsedThisCombo_FinalStateParticles);
          continue;
		}

        // kaon momentum cut
		// if(locKPlusP4_highp.P() < 3.0 || locKPlusP4_lowp.P() >= 3.0)
		//   {
		//     dComboWrapper->Set_IsComboCut(true); 
        //     locUsedSoFar_gXi.insert(locUsedThisCombo_gXi);
        //     locUsedSoFar_MissingMass.insert(locUsedThisCombo_MissingMass);			
        //     locUsedSoFar_FinalStateParticles.insert(locUsedThisCombo_FinalStateParticles);
		//     continue;
		//   }
		
		// get the number of tracks used per survived combo
		Int_t locNumUnusedChargedTracks = dComboWrapper->Get_NumUnusedTracks();
		Int_t locNumUnusedNeutralTracks = dComboWrapper->Get_NumUnusedShowers();
		Int_t locNumReconChargedTracks = locNumChargedHypos + locNumUnusedChargedTracks; 
		Int_t locNumReconNeutralTracks = locNumNeutralHypos + locNumUnusedNeutralTracks; 
		
		dHist_NumUnusedChargedTracksPerCombo->Fill(locNumUnusedChargedTracks);
		dHist_NumUnusedNeutralTracksPerCombo->Fill(locNumUnusedNeutralTracks);

		// only keep 1 combo per event
		if(locNumComboSurvivedCut == 1)
            {
              dHist_NumReconChargedTracksPerEvent->Fill(locNumReconChargedTracks); 
              dHist_NumReconNeutralTracksPerEvent->Fill(locNumReconNeutralTracks); 
            }
	   
        // find the best combo in this event (chisqndf closest to 1) 
        if(locNumComboSurvivedCut<1) {locBestChiSqNdf = locChiSqNdf, locBestChiSqNdf_1 = locChiSqNdf; cout << "Event " << locEventNum << endl;}
        if(locNumComboSurvivedCut>=1 )
          {
            if(locBestChiSqNdf > locChiSqNdf)
              locBestChiSqNdf = locChiSqNdf, locBestChiSqNdfComboNum = loc_i;
            
            if(fabs(1.0-locBestChiSqNdf) > fabs(1.0-locChiSqNdf)) 
              locBestChiSqNdf_1 = locChiSqNdf, locBestChiSqNdfComboNum_1 = loc_i;
          }  
        locNumComboSurvivedCut += 1;
                
        /***************************************************************************************************************************************/
        
        //Fill User histograms
		//compare to what's been used so far
		if(locUsedSoFar_MissingMass.find(locUsedThisCombo_MissingMass) == locUsedSoFar_MissingMass.end())
		{
		    //unique missing mass combo: histogram it, and register this combo of particles
            //kinematic distributions
		  
		    //Vertex hists
		    dHist_ProductionVertex_XVsY->Fill(locProdSpacetimeVertex.X(), locProdSpacetimeVertex.Y(), locHistAccidWeightFactor);
            dHist_ProductionVertex_Z->Fill(locProdSpacetimeVertex.Z(), locHistAccidWeightFactor);
            dHist_XiMinusVertex_XVsY->Fill(locDecayingXiX4.X(), locDecayingXiX4.Y(), locHistAccidWeightFactor);
            dHist_XiMinusVertex_Z->Fill(locDecayingXiX4.Z(),locHistAccidWeightFactor);
            dHist_LambdaVertex_XVsY->Fill(locDecayingLambdaX4.X(), locDecayingLambdaX4.Y(), locHistAccidWeightFactor);
            dHist_LambdaVertex_Z->Fill(locDecayingLambdaX4.Z(),locHistAccidWeightFactor);
		  
            // KPlus1
            dHist_KPlusHighP3_Theta_Measured->Fill(180/TMath::Pi()*locKPlusP4_highp_Measured.Theta(), locKPlusP4_highp_Measured.P(), locHistAccidWeightFactor);
            dHist_KPlusHighP3_Theta->Fill(180/TMath::Pi()*locKPlusP4_highp.Theta(), locKPlusP4_highp.P(), locHistAccidWeightFactor);
            dHist_KPlusHighP3_Theta_CM->Fill(180/TMath::Pi()*locKPlusP4_highp_CM.Theta(), locKPlusP4_highp_CM.P(), locHistAccidWeightFactor);
            dHist_KPlus1P3VsThetaMeasured->Fill(180/TMath::Pi()*locKPlus1P4_Measured.Theta(),locKPlus1P4_Measured.P(), locHistAccidWeightFactor);
            dHist_KPlus1P3VsTheta->Fill(180/TMath::Pi()*locKPlus1P4.Theta(),locKPlus1P4.P(), locHistAccidWeightFactor);
            dHist_KPlus1P3VsTheta_CM->Fill(180/TMath::Pi()*locKPlus1P4_CM.Theta(),locKPlus1P4_CM.P(), locHistAccidWeightFactor);
			      
            dHist_KPlus1P3VsTruth->Fill(locKPlus1P4.P(), locKPlus1P4_Truth.P(), locHistAccidWeightFactor);
            dHist_KPlus2P3VsTruth->Fill(locKPlus2P4.P(), locKPlus2P4_Truth.P(), locHistAccidWeightFactor);
            dHist_KPlusHighP3VsTruth->Fill( locKPlusP4_highp.P(), locKPlus1P4_Truth.P(), locHistAccidWeightFactor);
            dHist_KPlusLowP3VsTruth->Fill(locKPlusP4_lowp.P(), locKPlus2P4_Truth.P(), locHistAccidWeightFactor);
            dHist_XimInvariantMassKF_KPlus2P3->Fill(locXiMinusP4.M(),locKPlus2P4.P(), locHistAccidWeightFactor);
            dHist_XimInvariantMassKF_KPlusLowP3->Fill(locXiMinusP4.M(),locKPlusP4_lowp.P(), locHistAccidWeightFactor);

            // Ystar
            dHist_YstarInvariantMass->Fill( locYstarP4.M(), locHistAccidWeightFactor);
            dHist_YstarInvariantMass_Measured->Fill(locYstarP4_Measured.M() , locHistAccidWeightFactor);
            dHist_YstarInvariantMass_Kp2Xim->Fill(locYstar_Kp2Xim_P4.M(), locHistAccidWeightFactor);
            dHist_YstarP3Theta->Fill(180/TMath::Pi()*locYstarP4.Theta(), locYstarP4.P(), locHistAccidWeightFactor);
            dHist_YstarP3Theta_CM->Fill(180/TMath::Pi()*locYstarP4_CM.Theta(), locYstarP4_CM.P(), locHistAccidWeightFactor);

            dHist_KPlus2P3VsTheta->Fill(180/TMath::Pi()*locKPlus2P4.Theta(),locKPlus2P4.P(), locHistAccidWeightFactor);
			  
            dHist_XimP3VsThetaMeasured->Fill((180/TMath::Pi())*(locXiMinusP4_Measured.Theta()),locXiMinusP4_Measured.P(), locHistAccidWeightFactor);
            dHist_XimP3VsTheta->Fill((180/TMath::Pi())*(locXiMinusP4.Theta()),locXiMinusP4.P(), locHistAccidWeightFactor);

            dHist_KPlus2P3VsThetaMeasured->Fill(180/TMath::Pi()*locKPlus2P4_Measured.Theta(),locKPlus2P4_Measured.P(), locHistAccidWeightFactor);
            dHist_KPlus2P3VsTheta->Fill(180/TMath::Pi()*locKPlus2P4.Theta(),locKPlus2P4.P(), locHistAccidWeightFactor);
            dHist_KPlus2P3VsTheta_CM->Fill(180/TMath::Pi()*locKPlus2P4_CM.Theta(),locKPlus2P4_CM.P(), locHistAccidWeightFactor);
            dHist_KPlusLowP3_Theta_Measured->Fill(180/TMath::Pi()*locKPlusP4_lowp_Measured.Theta(), locKPlusP4_lowp_Measured.P(), locHistAccidWeightFactor);
            dHist_KPlusLowP3_Theta->Fill(180/TMath::Pi()*locKPlusP4_lowp.Theta(), locKPlusP4_lowp.P(), locHistAccidWeightFactor);
            dHist_KPlusLowP3_Theta_CM->Fill(180/TMath::Pi()*locKPlusP4_lowp_CM.Theta(), locKPlusP4_lowp_CM.P(), locHistAccidWeightFactor);
		       
            dHist_LambdaP3Theta->Fill((180/TMath::Pi())*(locDecayingLambdaP4.Theta()),locDecayingLambdaP4.P(), locHistAccidWeightFactor);
            dHist_PiMinus1P3Theta->Fill((180/TMath::Pi())*(locPiMinus1P4.Theta()),locPiMinus1P4.P(), locHistAccidWeightFactor);
            dHist_PiMinus2P3Theta->Fill((180/TMath::Pi())*(locPiMinus2P4.Theta()),locPiMinus2P4.P(), locHistAccidWeightFactor);
            dHist_ProtonP3VsTheta->Fill((180/TMath::Pi())*(locProtonP4.Theta()),locProtonP4.P(), locHistAccidWeightFactor);

            dHist_KPlusHighpPiMinus1_PiMinus2_InvariantMass->Fill(locKPlusHigh_PiMinus1_P4.M(), locKPlusHigh_PiMinus2_P4.M(), locHistAccidWeightFactor);

            dHist_XiMinusIM_LambdaIM->Fill(locXiMinusP4.M(),locDecayingLambdaP4_Measured.M(), locHistAccidWeightFactor);
		      
            dHist_YstarMass_XiMinusMass->Fill(locYstarP4.M(), locXiMinusP4.M(), locHistAccidWeightFactor);

            dHist_XimInvariantMass_XimVertexZ->Fill(locXiMinusP4.M(), locDecayingXiX4.Z(), locHistAccidWeightFactor);
            dHist_XimInvariantMass_LambdaVertexZ->Fill(locXiMinusP4.M(), locDecayingLambdaX4.Z(), locHistAccidWeightFactor);
		      
            if(locPathLengthSignificanceXi >= 3.0)
              {
                dHist_XimInvariantMass_XimVertexZ_SignificanceCut->Fill(locXiMinusP4.M(), locDecayingXiX4.Z(), locHistAccidWeightFactor);
                dHist_XimInvariantMass_LambdaVertexZ_SignificanceCut->Fill(locXiMinusP4.M(), locDecayingLambdaX4.Z(), locHistAccidWeightFactor);
              }
		      
            if(locXiMinusP4.M() >= 1.345)
              {
                dHist_YstarInvariantMass_SigmaRegion->Fill(locYstarP4.M(), locHistAccidWeightFactor);
                dHist_KPlusHighP3_Theta_SigmaRegion->Fill((180/TMath::Pi())*locKPlusP4_highp.Theta(), locKPlusP4_highp.P(), locHistAccidWeightFactor);
              }
		      

            // Xim Mass Cut
            if(locXiMinusP4.M() >= 1.308 && locXiMinusP4.M() <= 1.334 )
              {
                // KPlus1
                dHist_KPlus1P3VsTheta_XimCutIn->Fill(180/TMath::Pi()*locKPlus1P4.Theta(),locKPlus1P4.P(), locHistAccidWeightFactor);
                dHist_KPlusHighP3_Theta_XimCutIn->Fill(180/TMath::Pi()*locKPlusP4_highp.Theta(), locKPlusP4_highp.P(), locHistAccidWeightFactor);
                dHist_KPlusHighP3Phi->Fill(180/TMath::Pi()*locKPlusP4_highp.Phi(),locKPlusP4_highp.P(), locHistAccidWeightFactor);
			  		  		  
                // YStar 
                dHist_YstarInvariantMass_XimCut->Fill(locYstarP4.M(), locHistAccidWeightFactor);
                dHist_YstarInvariantMass_Kp2Xim_XimCut->Fill(locYstar_Kp2Xim_P4.M(), locHistAccidWeightFactor);
                dHist_YstarInvariantMass_XimCut->Fill(locYstarP4.M() , locHistAccidWeightFactor);
                dHist_YstarInvariantMass_XimCut_Measured->Fill(locYstarP4_Measured.M() , locHistAccidWeightFactor);
	
                // XiMinus
                dHist_XimP3VsTheta_XimCutIn->Fill((180/TMath::Pi())*(locXiMinusP4.Theta()),locXiMinusP4.P(), locHistAccidWeightFactor);
                dHist_t_XimCut->Fill(-1.0*locT, locHistAccidWeightFactor);

                if(locKPlusP4_highp.P() > 3)
                  dHist_t_XimCut_KaonHighCut->Fill(-1.0*locT, locHistAccidWeightFactor);
			  
                // KPlus2
                dHist_KPlus2P3VsTheta_XimCutIn->Fill(180/TMath::Pi()*locKPlus2P4.Theta(),locKPlus2P4.P(), locHistAccidWeightFactor);

                dHist_KPlusLowP3_Theta_XimCutIn->Fill(180/TMath::Pi()*locKPlusP4_lowp.Theta(), locKPlusP4_lowp.P(), locHistAccidWeightFactor);
			  
                // Lambda
                dHist_LambdaP3ThetaMeasured->Fill((180/TMath::Pi())*(locDecayingLambdaP4_Measured.Theta()),locDecayingLambdaP4_Measured.P(), locHistAccidWeightFactor);
                dHist_LambdaP3Theta_XimCutIn->Fill((180/TMath::Pi())*(locDecayingLambdaP4.Theta()),locDecayingLambdaP4.P(), locHistAccidWeightFactor);
                dHist_LambdaP3Theta_CM->Fill((180/TMath::Pi())*(locDecayingLambdaP4_CM.Theta()),locDecayingLambdaP4_CM.P(), locHistAccidWeightFactor);
                dHist_LambdaP3Theta_HF->Fill((180/TMath::Pi())*(Lambda_Angle_HF),locDecayingLambdaP4_HF.P(), locHistAccidWeightFactor);
                dHist_LambdaP3CosTheta_HF->Fill(Lambda_CosAngle_HF,locDecayingLambdaP4_HF.P(), locHistAccidWeightFactor);
                // PiMinus1
                dHist_PiMinus1P3ThetaMeasured->Fill((180/TMath::Pi())*(locPiMinus1P4_Measured.Theta()),locPiMinus1P4_Measured.P(), locHistAccidWeightFactor);
                dHist_PiMinus1P3Theta_XimCutIn->Fill((180/TMath::Pi())*(locPiMinus1P4.Theta()),locPiMinus1P4.P(), locHistAccidWeightFactor);
                dHist_PiMinus1P3Theta_HF->Fill(180/TMath::Pi()*PiMinus1_Angle_HF,locPiMinus1P4_HF.P(), locHistAccidWeightFactor);
                dHist_PiMinus1P3CosTheta_HF->Fill(PiMinus1_CosTheta_HF,locPiMinus1P4_HF.P(), locHistAccidWeightFactor);
                // PiMinus2
                dHist_PiMinus2P3ThetaMeasured->Fill((180/TMath::Pi())*(locPiMinus2P4_Measured.Theta()),locPiMinus2P4_Measured.P(), locHistAccidWeightFactor);
                dHist_PiMinus2P3Theta_XimCutIn->Fill((180/TMath::Pi())*(locPiMinus2P4.Theta()),locPiMinus2P4.P(), locHistAccidWeightFactor);
                // Proton
                dHist_ProtonP3VsThetaMeasured->Fill((180/TMath::Pi())*(locProtonP4_Measured.Theta()),locProtonP4_Measured.P(), locHistAccidWeightFactor);
                dHist_ProtonP3VsTheta_XimCutIn->Fill((180/TMath::Pi())*(locProtonP4.Theta()),locProtonP4.P(), locHistAccidWeightFactor);	        
                dHist_KPlusHighP3_KPlusLowP3_t->Fill(-1.0*locT, locKPlusP4_highp.P(), locKPlusP4_lowp.P(), locHistAccidWeightFactor);
                dHist_KPlus1P3_KPlus2P3_t->Fill(-1.0*locT_K1, locKPlus1P4.P(), locKPlus2P4.P(), locHistAccidWeightFactor);
					  
                if(locYstarP4.M() <= 2.5)
                  {
                    dHist_KPlus1P3VsTheta_YstarCut->Fill(180/TMath::Pi()*locKPlus1P4.Theta(), locKPlus1P4.P(), locHistAccidWeightFactor);
                  }
                else if(locYstarP4.M() > 2.5)
                  {
                    dHist_KPlus2P3VsTheta_YstarCut->Fill(180/TMath::Pi()*locKPlus2P4.Theta(), locKPlus2P4.P(), locHistAccidWeightFactor);
                  }
              }
            else
              {
                dHist_KPlus1P3VsTheta_XimCutOut->Fill(180/TMath::Pi()*locKPlus1P4.Theta(),locKPlus1P4.P(), locHistAccidWeightFactor);
                dHist_KPlusHighP3_Theta_XimCutOut->Fill(180/TMath::Pi()*locKPlusP4_highp.Theta(), locKPlusP4_highp.P(), locHistAccidWeightFactor);
                dHist_KPlusHighP3Phi->Fill(180/TMath::Pi()*locKPlusP4_highp.Phi(),locKPlusP4_highp.P(), locHistAccidWeightFactor);
                dHist_KPlus2P3VsTheta_XimCutOut->Fill(180/TMath::Pi()*locKPlus2P4.Theta(),locKPlus2P4.P(), locHistAccidWeightFactor);
                dHist_KPlusLowP3_Theta_XimCutOut->Fill(180/TMath::Pi()*locKPlusP4_lowp.Theta(), locKPlusP4_lowp.P(), locHistAccidWeightFactor);
                dHist_XimP3VsTheta_XimCutOut->Fill((180/TMath::Pi())*(locXiMinusP4.Theta()),locXiMinusP4.P(), locHistAccidWeightFactor);
                dHist_LambdaP3Theta_XimCutOut->Fill((180/TMath::Pi())*(locDecayingLambdaP4.Theta()),locDecayingLambdaP4.P(), locHistAccidWeightFactor);
                dHist_PiMinus1P3Theta_XimCutOut->Fill((180/TMath::Pi())*(locPiMinus1P4.Theta()),locPiMinus1P4.P(), locHistAccidWeightFactor);
                dHist_PiMinus2P3Theta_XimCutOut->Fill((180/TMath::Pi())*(locPiMinus2P4.Theta()),locPiMinus2P4.P(), locHistAccidWeightFactor);
                dHist_ProtonP3VsTheta_XimCutOut->Fill((180/TMath::Pi())*(locProtonP4.Theta()),locProtonP4.P(), locHistAccidWeightFactor);        
              }
				
            //insert combo into list for uniquiness tracking
            locUsedSoFar_MissingMass.insert(locUsedThisCombo_MissingMass);
		}

		//compare to what's been used so far
		if(locUsedSoFar_gXi.find(locUsedThisCombo_gXi) == locUsedSoFar_gXi.end())
		{
            //unique missing mass combo: histogram it, and register this combo of particles
            dHist_XimInvariantMassMeasured->Fill(locXiMinusP4_Measured.M(), locHistAccidWeightFactor);
		      
            dHist_LambdaInvariantMassKinFit->Fill(locDecayingLambdaP4.M(), locHistAccidWeightFactor);		           
            dHist_LambdaInvariantMassMeasured->Fill(locDecayingLambdaP4_Measured.M(), locHistAccidWeightFactor);
			     
            //dHist_XimP3VsThetaMeasured->Fill(TMath::Cos(locXiMinusP4_Measured.Theta()),locXiMinusP4_Measured.P());
            if(locXiMinusP4.M() > 1.308 && locXiMinusP4.M() > 1.334)
              {
                dHist_XiPath_postCL->Fill(locPathLengthXi, locHistAccidWeightFactor);
                dHist_XiLifetime_postCL->Fill(locLifetimeXi, locHistAccidWeightFactor);
                dHist_XiLifetimeRestFrame_postCL->Fill(locLifetimeRestFrameXi, locHistAccidWeightFactor);
                  
                dHist_LambPath_postCL->Fill(locPathLengthLambda, locHistAccidWeightFactor);
                dHist_LambdaLifetime_postCL->Fill(locLifetimeLambda, locHistAccidWeightFactor);
                dHist_LambdaLifetimeRestFrame_postCL->Fill(locLifetimeRestFrameLambda, locHistAccidWeightFactor);
              }
              
            dHist_XiMass_XiPathLength->Fill(locXiMinusP4.M(), locPathLengthXi, locHistAccidWeightFactor);
            dHist_XiMass_XiPathLengthSignificance->Fill(locXiMinusP4.M(), locPathLengthSignificanceXi, locHistAccidWeightFactor);
			  
            if(locPathLengthSignificanceXi >= 3.0)
              {
                dHist_XiMass_XiPathLength_SignificanceCut->Fill(locXiMinusP4.M(), locPathLengthXi, locHistAccidWeightFactor);
                dHist_XimInvariantMassKinFit_FlightSignificanceCut->Fill(locXiMinusP4.M(), locHistAccidWeightFactor);
              }
		      
            dHist_XiIM_Egamma->Fill( locBeamP4.E(), locXiMinusP4.M() , locHistAccidWeightFactor);  
            dHist_Xi_Egamma_t->Fill(locBeamP4.E(), locXiMinusP4.M(), -1.0*locT, locHistAccidWeightFactor);
		      
            dHist_XimInvariantMassKinFit->Fill(locXiMinusP4.M(), locHistAccidWeightFactor);
            dHist_XimInvariantMass_CM->Fill(locXiP4_CM.M(), locHistAccidWeightFactor);
            dHist_XimInvariantMass_HF->Fill(locXiP4_HF.M(), locHistAccidWeightFactor);
			  
            if ( locNumReconChargedTracks == 5 )
              {
                dHist_XimInvariantMassKinFit_ChargedTrackCut->Fill(locXiMinusP4.M(), locHistAccidWeightFactor);
              }
              
            if ( locNumReconNeutralTracks == 0 )
              {
                dHist_XimInvariantMassKinFit_NeutralTrackCut->Fill(locXiMinusP4.M(), locHistAccidWeightFactor);
              }
			  
            // Get Xi events per bin
            for(Int_t j=0; j < enBins; j++)
              {			      
                if ( locBeamP4.E() >= arr_enBins[j] && locBeamP4.E() < arr_enBins[j+1])
                  {
                    dHist_XiIM_Egamma_arr[j]->Fill( locXiMinusP4.M() , locHistAccidWeightFactor);  
                  }
              }
		      
            locUsedSoFar_gXi.insert(locUsedThisCombo_gXi);
		}
		
		// map<Particle_t, set<Int_t> > locUsedThisCombo_ProtonKinematics;
		// locUsedThisCombo_ProtonKinematics[Proton].insert(locProtonTrackID);

		// if(locUsedSoFar_ProtonKinematics.find(locUsedThisCombo_ProtonKinematics) == locUsedSoFar_ProtonKinematics.end())
		// {
		  
		  
		//   locUsedSoFar_ProtonKinematics.insert(locUsedThisCombo_ProtonKinematics);
		// }

		/******************************************** EXECUTE ANALYSIS ACTIONS *******************************************/

		// Loop through the analysis actions, executing them in order for the active particle combo
		dAnalyzeCutActions->Perform_ActionWeight(locHistAccidWeightFactor);
		
		// Loop through the analysis actions, executing them in order for the active particle combo
		
		//dAnalyzeCutActions->Perform_Action(); // Must be executed before Execute_Actions()
		if(!Execute_Actions()) //if the active combo fails a cut, IsComboCutFlag automatically set
		  continue;
		
		//if you manually execute any actions, and it fails a cut, be sure to call:
		//dComboWrapper->Set_IsComboCut(true);

		/*****************************************************************************************************************/

    } // end of combo loop for cuts

    //FILL HISTOGRAMS: Num combos / events surviving actions
	dHist_NumCombosSurviveCut->Fill(locNumComboSurvivedCut);
	Fill_NumCombosSurvivedHists();
	

    /*********************************************************************************************************************/
    // begin combo loop to fill flat trees
    for(UInt_t loc_i = 0; loc_i < Get_NumCombos(); ++loc_i)
	{
		//Set branch array indices for combo and all combo particles
		dComboWrapper->Set_ComboIndex(loc_i);
        // only keep best combo event for flat trees 
        //if(locBestChiSqNdfComboNum == loc_i) continue;
		
        // Is used to indicate when combos have been cut
		// if(dComboWrapper->Get_IsComboCut()) // Is false when tree originally created
		//         continue; // Combo has been cut previously
		

		/********************************************** GET PARTICLE INDICES *********************************************/

		//Used for tracking uniqueness when filling histograms, and for determining unused particles

		//Step 0
		Int_t locBeamID = dComboBeamWrapper->Get_BeamID();
		Int_t locKPlus1TrackID = dKPlus1Wrapper->Get_TrackID();
		Int_t locKPlus2TrackID = dKPlus2Wrapper->Get_TrackID();

		//Step 1
		Int_t locPiMinus1TrackID = dPiMinus1Wrapper->Get_TrackID();

		//Step 2
		Int_t locPiMinus2TrackID = dPiMinus2Wrapper->Get_TrackID();
		Int_t locProtonTrackID = dProtonWrapper->Get_TrackID();

		/*********************************************** GET FOUR-MOMENTUM **********************************************/

		// Get P4's: //is kinfit if kinfit performed, else is measured
		//dTargetP4 is target p4
		//Step 0
		TLorentzVector locBeamP4 = dComboBeamWrapper->Get_P4();
		TLorentzVector locKPlus1P4 = dKPlus1Wrapper->Get_P4();
		TLorentzVector locKPlus2P4 = dKPlus2Wrapper->Get_P4();
		//Step 1
		TLorentzVector locPiMinus1P4 = dPiMinus1Wrapper->Get_P4();
		//Step 2
		TLorentzVector locDecayingLambdaP4 = dDecayingLambdaWrapper->Get_P4();
		TLorentzVector locPiMinus2P4 = dPiMinus2Wrapper->Get_P4();
		TLorentzVector locProtonP4 = dProtonWrapper->Get_P4();

		// Get Measured P4's:
		//Step 0
		TLorentzVector locBeamP4_Measured = dComboBeamWrapper->Get_P4_Measured();
		TLorentzVector locKPlus1P4_Measured = dKPlus1Wrapper->Get_P4_Measured();
		TLorentzVector locKPlus2P4_Measured = dKPlus2Wrapper->Get_P4_Measured();
		//Step 1
		TLorentzVector locPiMinus1P4_Measured = dPiMinus1Wrapper->Get_P4_Measured();
		//Step 2
		TLorentzVector locPiMinus2P4_Measured = dPiMinus2Wrapper->Get_P4_Measured();
		TLorentzVector locProtonP4_Measured = dProtonWrapper->Get_P4_Measured();

		/********************************************* GET COMBO RF TIMING INFO *****************************************/

        TLorentzVector locBeamX4_Measured = dComboBeamWrapper->Get_X4_Measured();
        Double_t locBunchPeriod = dAnalysisUtilities.Get_BeamBunchPeriod(Get_RunNumber());
        Double_t locDeltaT_RF = dAnalysisUtilities.Get_DeltaT_RF(Get_RunNumber(), locBeamX4_Measured, dComboWrapper);
        Int_t locRelBeamBucket = dAnalysisUtilities.Get_RelativeBeamBucket(Get_RunNumber(), locBeamX4_Measured, dComboWrapper); // 0 for in-time events, non-zero integer for out-of-time photons
        Int_t locNumOutOfTimeBunchesInTree = 1; //YOU need to specify this number
        //Number of out-of-time beam bunches in tree (on a single side, so that total number out-of-time bunches accepted is 2 times this number for left + right bunches) 
                
        Bool_t locSkipNearestOutOfTimeBunch = false; // True: skip events from nearest out-of-time bunch on either side (recommended).
        Int_t locNumOutOfTimeBunchesToUse = locNumOutOfTimeBunchesInTree>1 ? locNumOutOfTimeBunchesInTree-1:locNumOutOfTimeBunchesInTree;
        Double_t locAccidentalScalingFactor = dAnalysisUtilities.Get_AccidentalScalingFactor(Get_RunNumber(), locBeamP4.E()); // Ideal value would be 1, but deviations observed: need added factor.
        Double_t locAccidentalScalingFactorError = dAnalysisUtilities.Get_AccidentalScalingFactorError(Get_RunNumber(), locBeamP4.E()); // Ideal value would be 1, but deviations observed, need added factor.
        Double_t locHistAccidWeightFactor = locRelBeamBucket==0 ? 1 : -locAccidentalScalingFactor/(2*locNumOutOfTimeBunchesToUse) ; // Weight by 1 for in-time events, ScalingFactor*(1/NBunches) for out-of-time
        //if(locSkipNearestOutOfTimeBunch && abs(locRelBeamBucket)==1 && locNumOutOfTimeBunchesInTree>1) continue; // Skip nearest out-of-time bunch: tails of in-time distribution also leak in
		
		/********************************************* COMBINE FOUR-MOMENTUM ********************************************/

		// DO YOUR STUFF HERE

		// Accidental Scale Factor
		//double scaling_factor = dAnalysisUtilities.Get_AccidentalScalingFactor(locRunNumber, locBeamP4.E());
		//double scaling_factor_err = dAnalysisUtilities.Get_AccidentalScalingFactorError(locRunNumber, locBeamP4.E());
		
		// Combine 4-vectors
		// Kaon Determincation
		TLorentzVector locKPlusP4_lowp;
		TLorentzVector locKPlusP4_highp;
		TLorentzVector locKPlusP4_lowp_Measured;
		TLorentzVector locKPlusP4_highp_Measured;
		Int_t locKhighTrackID;
		Int_t locKlowTrackID;

        // Kaon determined from fcal bcal detectors
        TLorentzVector locKPlusLowP4_Bcal;
        TLorentzVector locKPlusHighP4_Fcal;
        Double_t kp1_efcal = dKPlus1Wrapper->Get_Energy_FCAL();
        Double_t kp1_ebcal = dKPlus1Wrapper->Get_Energy_BCAL();
        Double_t kp2_efcal = dKPlus2Wrapper->Get_Energy_FCAL();
        Double_t kp2_ebcal = dKPlus2Wrapper->Get_Energy_BCAL();
        
        // pick only events that are bcal(K2) and fcal(K1)
        if(kp1_efcal > 1e-1 && kp2_ebcal > 1e-1) {locKPlusHighP4_Fcal = locKPlus1P4, locKPlusLowP4_Bcal = locKPlus2P4;}
        else if(kp1_ebcal > 1e-1 && kp2_efcal > 1e-1) {locKPlusLowP4_Bcal = locKPlus1P4, locKPlusHighP4_Fcal = locKPlus2P4;}
        else continue;

		//Kaon Seperation with only angle and momentum
		if(locKPlus1P4.P() > locKPlus2P4.P() )
		  {
		    locKPlusP4_highp = locKPlus1P4; 
		    locKPlusP4_lowp = locKPlus2P4;
		    locKhighTrackID = locKPlus1TrackID;
		    locKlowTrackID = locKPlus2TrackID;
		    locKPlusP4_highp_Measured = locKPlus1P4_Measured; 
		    locKPlusP4_lowp_Measured = locKPlus2P4_Measured;
		  }
		else
		  {
		    locKPlusP4_highp = locKPlus2P4; 
		    locKPlusP4_lowp = locKPlus1P4;
		    locKhighTrackID = locKPlus2TrackID;
		    locKlowTrackID = locKPlus1TrackID;
		    locKPlusP4_highp_Measured = locKPlus2P4_Measured; 
		    locKPlusP4_lowp_Measured = locKPlus1P4_Measured;
		  }

		Double_t locT = (locBeamP4-locKPlusP4_highp).M2();
		Double_t locT_K1 = (locBeamP4-locKPlus1P4).M2();

		TLorentzVector locMissingP4_Measured = locBeamP4_Measured + dTargetP4;
		TLorentzVector locDecayingLambdaP4_Measured = locProtonP4_Measured + locPiMinus2P4_Measured;
	  		
		TLorentzVector locXiMinusP4 = locDecayingLambdaP4 + locPiMinus1P4;
		TLorentzVector locXiMinusP4_Measured = locDecayingLambdaP4_Measured + locPiMinus1P4_Measured;  
		  
		TLorentzVector locYstar_Kp2Xim_P4 = locKPlus2P4 + locXiMinusP4;
		TLorentzVector locYstarP4 = locKPlusP4_lowp + locXiMinusP4;
        TLorentzVector locYstarP4_Measured = locKPlusP4_lowp_Measured + locXiMinusP4_Measured;

		TLorentzVector locKPlusHigh_PiMinus1_P4 = locKPlusP4_highp + locPiMinus1P4;
		TLorentzVector locKPlusHigh_PiMinus2_P4 = locKPlusP4_highp + locPiMinus2P4;

		TLorentzVector locKPlusLow_PiMinus1_P4 = locKPlusP4_lowp + locPiMinus1P4;
		TLorentzVector locKPlusLow_PiMinus2_P4 = locKPlusP4_lowp + locPiMinus2P4;        

		locMissingP4_Measured -= locKPlus1P4_Measured + locKPlus2P4_Measured + locPiMinus1P4_Measured + locPiMinus2P4_Measured + locProtonP4_Measured;

		/**************************************************
                          Boost into different frames
		***************************************************/
		//BoostVector in COM frame
		TLorentzVector locCoMP4 = locBeamP4 + dTargetP4;
		TVector3 boostCoM = locCoMP4.BoostVector();
		//Initalize 4Vectors in COM frame
		//step 1
		TLorentzVector locBeamP4_CM = locBeamP4;
		//step 2
		TLorentzVector locKPlus1P4_CM = locKPlus1P4;
		TLorentzVector locKPlusP4_highp_CM = locKPlusP4_highp;
        TLorentzVector locYstarP4_CM = locYstarP4;
		//step 3
		TLorentzVector locXiP4_CM = locXiMinusP4;
		TLorentzVector locKPlus2P4_CM = locKPlus2P4;
		TLorentzVector locKPlusP4_lowp_CM = locKPlusP4_lowp;
        TLorentzVector locPiMinus1P4_CM = locPiMinus1P4;
		//step 4
		TLorentzVector locDecayingLambdaP4_CM = locDecayingLambdaP4;
		//Boost 4vectors in Com frame
		locBeamP4_CM.Boost(-boostCoM);
		locYstarP4_CM.Boost(-boostCoM);
		locKPlus1P4_CM.Boost(-boostCoM);
		locKPlusP4_highp_CM.Boost(-boostCoM);
        locKPlus2P4_CM.Boost(-boostCoM);
		locKPlusP4_lowp_CM.Boost(-boostCoM);
        locXiP4_CM.Boost(-boostCoM);
		locPiMinus1P4_CM.Boost(-boostCoM);
		locDecayingLambdaP4_CM.Boost(-boostCoM);
		
		//Boost 4vector into rest frame of ystar from CoM 
		TVector3 boostYstar_Rest = locYstarP4_CM.BoostVector();
		//Initialize 4vectors in ystar rest frame
		//step 1
		TLorentzVector locBeamP4_YstarRest = locBeamP4_CM;
		//step 2
		TLorentzVector locKPlus1P4_YstarRest = locKPlus1P4_CM;
		TLorentzVector locKPlusP4_highp_YstarRest = locKPlusP4_highp_CM;
        TLorentzVector locYstarP4_YstarRest = locYstarP4_CM;
		//step 3
		TLorentzVector locXiP4_YstarRest = locXiP4_CM;
		TLorentzVector locKPlus2P4_YstarRest = locKPlus2P4_CM;
		TLorentzVector locKPlusP4_lowp_YstarRest = locKPlusP4_lowp_CM;
        TLorentzVector locPiMinus1P4_YstarRest = locPiMinus1P4_CM;
		//step 4
		TLorentzVector locDecayingLambdaP4_YstarRest = locDecayingLambdaP4_CM;
		//Boost 4vectos to rest frame of ystar
		locBeamP4_YstarRest.Boost(-boostYstar_Rest);
		locYstarP4_YstarRest.Boost(-boostYstar_Rest);
		locKPlus1P4_YstarRest.Boost(-boostYstar_Rest);
		locKPlusP4_highp_YstarRest.Boost(-boostYstar_Rest);
        locKPlus2P4_YstarRest.Boost(-boostYstar_Rest);
		locKPlusP4_lowp_YstarRest.Boost(-boostYstar_Rest);
        locXiP4_YstarRest.Boost(-boostYstar_Rest);
		locPiMinus1P4_YstarRest.Boost(-boostYstar_Rest);
		locDecayingLambdaP4_YstarRest.Boost(-boostYstar_Rest);

        // Set helicity frame for ystar
        TVector3 z_hat_Ystar_HF = locYstarP4_CM.Vect().Unit();
        TVector3 y_hat_Ystar_HF = locBeamP4_CM.Vect().Cross(locYstarP4_CM.Vect()).Unit();//y direction normal to production plane gammap
        TVector3 x_hat_Ystar_HF = y_hat_Ystar_HF.Cross(z_hat_Ystar_HF);//maintain right handed coordinate system		
        TLorentzVector locKPlusP4_slow_HF(locKPlusP4_lowp_YstarRest.E(),locKPlusP4_lowp_YstarRest.Vect().Dot(x_hat_Ystar_HF), locKPlusP4_lowp_YstarRest.Vect().Dot(y_hat_Ystar_HF), locKPlusP4_lowp_YstarRest.Vect().Dot(z_hat_Ystar_HF)); 
        Double_t KPlusLow_CosTheta_HF = locKPlusP4_lowp_YstarRest.Pz() / locKPlusP4_lowp_YstarRest.Vect().Mag();
        Double_t KPlusLow_Phi_HF = TMath::ATan(locKPlusP4_lowp_YstarRest.Py() / locKPlusP4_lowp_YstarRest.Px());

		//BoostVector in Xim rest frame from ystar rest frame
		TVector3 boostXim_HF = locXiP4_YstarRest.BoostVector();
		//Initalize 4Vectors in Xim Rest frame
		//step 1
		TLorentzVector locBeamP4_HF = locBeamP4_YstarRest;
		//step 2
		TLorentzVector locKPlus1P4_HF = locKPlus1P4_YstarRest;
		TLorentzVector locKPlusP4_highp_HF = locKPlusP4_highp_YstarRest;
		TLorentzVector locYstarP4_HF = locYstarP4_YstarRest;
		//step 3
		TLorentzVector locXiP4_HF = locXiP4_YstarRest;
		TLorentzVector locKPlus2P4_HF = locKPlus2P4_YstarRest;
		TLorentzVector locKPlusP4_lowp_HF = locKPlusP4_lowp_YstarRest;
		TLorentzVector locPiMinus1P4_HF = locPiMinus1P4_YstarRest;
		//step 4
		TLorentzVector locDecayingLambdaP4_HF = locDecayingLambdaP4_YstarRest;
		//Boost 4vectors in Xim Rest frame
		locBeamP4_HF.Boost(-boostXim_HF);
		locKPlus1P4_HF.Boost(-boostXim_HF);
		locKPlusP4_highp_HF.Boost(-boostXim_HF);
		locKPlus2P4_HF.Boost(-boostXim_HF);
		locKPlusP4_lowp_HF.Boost(-boostXim_HF);
		locXiP4_HF.Boost(-boostXim_HF);
		locPiMinus1P4_HF.Boost(-boostXim_HF);
		locDecayingLambdaP4_HF.Boost(-boostXim_HF);
		//Create the GF reference frame
		TVector3 z_hat_HF = locXiP4_YstarRest.Vect().Unit();//z direction is opposite direction of the boost or Ystar in rest frame
        TVector3 y_hat_HF = locXiP4_YstarRest.Vect().Cross(locPiMinus1P4_YstarRest.Vect()).Unit();//y direction normal to production plane
        TVector3 x_hat_HF = y_hat_HF.Cross(z_hat_HF);//maintain right handed coordinate system		
        //Define angle of Klus2 and XiMinus in HF
        TVector3 locPiMinus1P3_HF(locPiMinus1P4_HF.Vect().Dot(x_hat_HF), locPiMinus1P4_HF.Vect().Dot(y_hat_HF), locPiMinus1P4_HF.Vect().Dot(z_hat_HF)); 
        double PiMinus1_Angle_HF = z_hat_HF.Angle(locPiMinus1P4_HF.Vect());
        double PiMinus1_CosTheta_HF = locPiMinus1P3_HF.Z() / locPiMinus1P3_HF.Mag();
		Double_t Lambda_Angle_HF = z_hat_HF.Angle(locDecayingLambdaP4_HF.Vect());
		Double_t Lambda_CosAngle_HF = TMath::Cos(z_hat_HF.Angle(locPiMinus1P4_HF.Vect()));


		/*******************************
                    Path Length Calculations
		*******************************/
		//TLorentzVector locDecayingLambdaP4 = locPiMinus2P4 +	locProtonP4; //Decaying Lambda for M18 is manually calculated
		TLorentzVector locProdSpacetimeVertex = dComboBeamWrapper->Get_X4();//Get production vertex
		// Xi Path Length
		TLorentzVector locDecayingXiX4 = dTreeInterface->Get_TObject<TLorentzVector>("DecayingXiMinus__X4",loc_i);
		TLorentzVector locDeltaSpacetimeXi = locProdSpacetimeVertex - locDecayingXiX4;//vertex difference
		Double_t locPathLengthXi = locDeltaSpacetimeXi.Vect().Mag();//pathlength is just the magnitude
		Float_t locPathLengthSigmaXi = Get_Fundamental<Float_t>("DecayingXiMinus__PathLengthSigma", loc_i);
		Double_t locPathLengthSignificanceXi = locPathLengthXi/locPathLengthSigmaXi;
        Double_t locLifetimeXi = locDeltaSpacetimeXi.T();//lifetime of xi in lab
		Double_t locLifetimeRestFrameXi = locPathLengthXi*locXiMinusP4.M() / (29.9792458 * locXiMinusP4.P());//lifetime of xi in restframe t = (travel distance)*(xi mass)/(xi mom) // add 1/c[cm/ns] to correct dimensionality

		// Lambda Path Length
		TLorentzVector locDecayingLambdaX4 = dDecayingLambdaWrapper->Get_X4(); //Doesn't exist for M18
		TLorentzVector locDeltaSpacetimeLambda = locDecayingXiX4 - locDecayingLambdaX4;//vertex difference
		Double_t locPathLengthLambda = locDeltaSpacetimeLambda.Vect().Mag();//pathlength is just the magnitude		
		Float_t locPathLengthSigmaLambda = dDecayingLambdaWrapper->Get_PathLengthSigma();
		Double_t locPathLengthSignificanceLambda = locPathLengthLambda/locPathLengthSigmaLambda;
		Double_t locLifetimeLambda = locDeltaSpacetimeLambda.T();//lifetime of lamb in lab
		Double_t locLifetimeRestFrameLambda = locPathLengthLambda*locDecayingLambdaP4.M() / (29.9792458 * locDecayingLambdaP4.P());//lifetime of lamb in restframe 
       
        
		//Missing Mass Squared
		Double_t locMissingMassSquared = locMissingP4_Measured.M2();

		//ChiSqNDF
		Double_t locChiSqNdf = dComboWrapper->Get_ChiSq_KinFit("") / dComboWrapper->Get_NDF_KinFit("");
		//Double_t locNDF = dComboWrapper->Get_NDF_KinFit("");
		//Double_t locChiSqNdf = locChiSq/locNDF;
        Double_t locConfidenceLvl = dComboWrapper->Get_ConfidenceLevel_KinFit("");


		/****************************************** FILL FLAT TREE (IF DESIRED) ******************************************/

		/*
		//FILL ANY CUSTOM BRANCHES FIRST!!
		Int_t locMyInt_Flat = 7;
		dFlatTreeInterface->Fill_Fundamental<Int_t>("flat_my_int", locMyInt_Flat);

		TLorentzVector locMyP4_Flat(4.0, 3.0, 2.0, 1.0);
		dFlatTreeInterface->Fill_TObject<TLorentzVector>("flat_my_p4", locMyP4_Flat);

		for(int loc_j = 0; loc_j < locMyInt_Flat; ++loc_j)
		{
			dFlatTreeInterface->Fill_Fundamental<Int_t>("flat_my_int_array", 3*loc_j, loc_j); //2nd argument = value, 3rd = array index
			TLorentzVector locMyComboP4_Flat(8.0, 7.0, 6.0, 5.0);
			dFlatTreeInterface->Fill_TObject<TLorentzVector>("flat_my_p4_array", locMyComboP4_Flat, loc_j);
		}
		*/

        // 4-vectors
        dFlatTreeInterface->Fill_TObject<TLorentzVector>("beam_p4", locBeamP4);
        dFlatTreeInterface->Fill_TObject<TLorentzVector>("ystar_p4", locYstarP4);
        dFlatTreeInterface->Fill_TObject<TLorentzVector>("kplow_p4", locKPlusP4_lowp);
        dFlatTreeInterface->Fill_TObject<TLorentzVector>("kplow_bcal_p4", locKPlusLowP4_Bcal);
        dFlatTreeInterface->Fill_TObject<TLorentzVector>("kphigh_p4", locKPlusP4_highp);
        dFlatTreeInterface->Fill_TObject<TLorentzVector>("kplow_p4_ystar_hf", locKPlusP4_slow_HF);
        dFlatTreeInterface->Fill_TObject<TLorentzVector>("kphigh_fcal_p4", locKPlusHighP4_Fcal);
        dFlatTreeInterface->Fill_TObject<TLorentzVector>("decayxim_p4", locXiMinusP4);
        dFlatTreeInterface->Fill_TObject<TLorentzVector>("lambda_p4", locDecayingLambdaP4);
        // beam stuff
        dFlatTreeInterface->Fill_Fundamental<Double_t>("beam_E", locBeamP4.E());
        dFlatTreeInterface->Fill_Fundamental<Double_t>("beam_rfbunches", locDeltaT_RF);
        dFlatTreeInterface->Fill_Fundamental<Double_t>("beam_vertexX", locProdSpacetimeVertex.X());
        dFlatTreeInterface->Fill_Fundamental<Double_t>("beam_vertexY", locProdSpacetimeVertex.Y());
        dFlatTreeInterface->Fill_Fundamental<Double_t>("beam_vertexZ", locProdSpacetimeVertex.Z());
        // kp1
        dFlatTreeInterface->Fill_Fundamental<Double_t>("kp1_P3", locKPlus1P4.P() );
        dFlatTreeInterface->Fill_Fundamental<Double_t>("kp1_P3_Truth", locKPlus1P4_Truth.P() );
        dFlatTreeInterface->Fill_Fundamental<Double_t>("kp1_CosTheta", locKPlus1P4.CosTheta() );
        dFlatTreeInterface->Fill_Fundamental<Double_t>("kp1_Phi", locKPlus1P4.Phi() );
        dFlatTreeInterface->Fill_Fundamental<Double_t>("kp_highp_P3", locKPlusP4_highp.P() );
        dFlatTreeInterface->Fill_Fundamental<Double_t>("kp_highp_Theta", locKPlusP4_highp.Theta() );
        dFlatTreeInterface->Fill_Fundamental<Double_t>("kp_highp_CosTheta", locKPlusP4_highp.CosTheta() );
        dFlatTreeInterface->Fill_Fundamental<Double_t>("kp_highp_Phi", locKPlusP4_highp.Phi() );
        // kp2
        dFlatTreeInterface->Fill_Fundamental<Double_t>("kp2_P3", locKPlus2P4.P() );
        dFlatTreeInterface->Fill_Fundamental<Double_t>("kp2_P3_Truth", locKPlus2P4_Truth.P() );
        dFlatTreeInterface->Fill_Fundamental<Double_t>("kp_lowp_Theta", locKPlusP4_lowp.Theta() );
        dFlatTreeInterface->Fill_Fundamental<Double_t>("kp2_YstarRest_CosTheta", locKPlus2P4_YstarRest.CosTheta() );
        dFlatTreeInterface->Fill_Fundamental<Double_t>("kp2_YstarRest_Phi", locKPlus2P4_YstarRest.Phi() );
        dFlatTreeInterface->Fill_Fundamental<Double_t>("kp_lowp_P3", locKPlusP4_lowp.P() );
        dFlatTreeInterface->Fill_Fundamental<Double_t>("kplow_costheta_hf",  KPlusLow_CosTheta_HF);
        dFlatTreeInterface->Fill_Fundamental<Double_t>("kplow_phi_hf",  KPlusLow_Phi_HF);
        // ystar
        dFlatTreeInterface->Fill_Fundamental<Double_t>("ystar_M", locYstarP4.M());			  
        dFlatTreeInterface->Fill_Fundamental<Double_t>("ystar_P3", locYstarP4.P() );
        dFlatTreeInterface->Fill_Fundamental<Double_t>("ystar_Theta", locYstarP4.CosTheta() );
        // ximinus
        dFlatTreeInterface->Fill_Fundamental<Double_t>("decayxim_M", locXiMinusP4.M());
        dFlatTreeInterface->Fill_Fundamental<Double_t>("xim_P3", locXiMinusP4.P() );
        dFlatTreeInterface->Fill_Fundamental<Double_t>("xim_vertexX", locDecayingXiX4.X());
        dFlatTreeInterface->Fill_Fundamental<Double_t>("xim_vertexY", locDecayingXiX4.Y());
        dFlatTreeInterface->Fill_Fundamental<Double_t>("xim_vertexZ", locDecayingXiX4.Z());
        dFlatTreeInterface->Fill_Fundamental<Double_t>("xim_pathlen", locPathLengthXi);
        dFlatTreeInterface->Fill_Fundamental<Double_t>("xim_pathlensig", locPathLengthSignificanceXi);
        dFlatTreeInterface->Fill_Fundamental<Double_t>("xim_lifetime", locLifetimeXi);
        dFlatTreeInterface->Fill_Fundamental<Double_t>("xim_lifetime_restframe", locLifetimeRestFrameXi);
        // lambda
        dFlatTreeInterface->Fill_Fundamental<Double_t>("decaylamb_M_meas", locDecayingLambdaP4_Measured.M());
        dFlatTreeInterface->Fill_Fundamental<Double_t>("lambda_P3", locDecayingLambdaP4.P() );
        dFlatTreeInterface->Fill_Fundamental<Double_t>("lambda_vertexX", locDecayingLambdaX4.X());
        dFlatTreeInterface->Fill_Fundamental<Double_t>("lambda_vertexY", locDecayingLambdaX4.Y());
        dFlatTreeInterface->Fill_Fundamental<Double_t>("lambda_vertexZ", locDecayingLambdaX4.Z());
        dFlatTreeInterface->Fill_Fundamental<Double_t>("lambda_pathlen", locPathLengthLambda);
        dFlatTreeInterface->Fill_Fundamental<Double_t>("lambda_pathlensig", locPathLengthSignificanceLambda);
        dFlatTreeInterface->Fill_Fundamental<Double_t>("lambda_lifetime", locLifetimeLambda);
        dFlatTreeInterface->Fill_Fundamental<Double_t>("lambda_lifetime_restframe", locLifetimeRestFrameLambda);
        // final state particles
        dFlatTreeInterface->Fill_Fundamental<Double_t>("pim1_P3", locPiMinus1P4.P() );
        dFlatTreeInterface->Fill_Fundamental<Double_t>("pim1_costheta_hf", PiMinus1_CosTheta_HF );     
        dFlatTreeInterface->Fill_Fundamental<Double_t>("pim2_P3", locPiMinus2P4.P() );
        dFlatTreeInterface->Fill_Fundamental<Double_t>("proton_P3", locProtonP4.P() );
        // other	    		    
        dFlatTreeInterface->Fill_Fundamental<Double_t>("acc_weight", locHistAccidWeightFactor);
        dFlatTreeInterface->Fill_Fundamental<Double_t>("chisqndf", locChiSqNdf);
        dFlatTreeInterface->Fill_Fundamental<Double_t>("confidencelvl", locConfidenceLvl);
        dFlatTreeInterface->Fill_Fundamental<Double_t>("total_mm2", locMissingMassSquared);
        dFlatTreeInterface->Fill_Fundamental<Double_t>("t_dist", -1.0*locT);
        dFlatTreeInterface->Fill_Fundamental<ULong64_t>("evnt_num", locEventNum);
        dFlatTreeInterface->Fill_Fundamental<UInt_t>("combo_num", loc_i);
        
        if(locBestChiSqNdfComboNum == loc_i) dFlatTreeInterface->Fill_Fundamental<Int_t>("best_combo", 1);
        else dFlatTreeInterface->Fill_Fundamental<Int_t>("best_combo", 0);
        
        if(locBestChiSqNdfComboNum_1 == loc_i) dFlatTreeInterface->Fill_Fundamental<Int_t>("best_combo_1", 1);
        else dFlatTreeInterface->Fill_Fundamental<Int_t>("best_combo_1", 0);
                
        dFlatTreeInterface->Fill_Fundamental<Int_t>("combos_survived", locNumComboSurvivedCut);

        cout << "ChiSqNdf: " << locChiSqNdf << "\nXiPathLen: " << locPathLengthXi << "\nXiPathLenSigma: " << locPathLengthSigmaXi << "\nXiPathLenSignificance: " << locPathLengthSignificanceXi << "\n" << endl;
			  
        //FILL FLAT TREE
        Fill_FlatTree(); //for the active combo		   
    }// end combo loop for flat trees
	
	//kaonfile->close();
	
	/****************************************** LOOP OVER OTHER ARRAYS (OPTIONAL) ***************************************/
/*
	//Loop over beam particles (note, only those appearing in combos are present)
	for(UInt_t loc_i = 0; loc_i < Get_NumBeam(); ++loc_i)
	{
		//Set branch array indices corresponding to this particle
		dBeamWrapper->Set_ArrayIndex(loc_i);

		//Do stuff with the wrapper here ...
	}

	//Loop over charged track hypotheses
	for(UInt_t loc_i = 0; loc_i < Get_NumChargedHypos(); ++loc_i)
	{
		//Set branch array indices corresponding to this particle
		dChargedHypoWrapper->Set_ArrayIndex(loc_i);

		//Do stuff with the wrapper here ...
	}

	//Loop over neutral particle hypotheses
	for(UInt_t loc_i = 0; loc_i < Get_NumNeutralHypos(); ++loc_i)
	{
		//Set branch array indices corresponding to this particle
		dNeutralHypoWrapper->Set_ArrayIndex(loc_i);

		//Do stuff with the wrapper here ...
	}
*/

	/************************************ EXAMPLE: FILL CLONE OF TTREE HERE WITH CUTS APPLIED ************************************/
/*
	Bool_t locIsEventCut = true;
	for(UInt_t loc_i = 0; loc_i < Get_NumCombos(); ++loc_i) {
		//Set branch array indices for combo and all combo particles
		dComboWrapper->Set_ComboIndex(loc_i);
		// Is used to indicate when combos have been cut
		if(dComboWrapper->Get_IsComboCut())
			continue;
		locIsEventCut = false; // At least one combo succeeded
		break;
	}
	if(!locIsEventCut && dOutputTreeFileName != "")
		Fill_OutputTree();
*/

	return kTRUE;
}

void DSelector_kpkpxim::Finalize(void)
{
	//Save anything to output here that you do not want to be in the default DSelector output ROOT file.

	//Otherwise, don't do anything else (especially if you are using PROOF).
		//If you are using PROOF, this function is called on each thread,
		//so anything you do will not have the combined infSormation from the various threads.
		//Besides, it is best-practice to do post-processing (e.g. fitting) separately, in case there is a problem.

 	//DO YOUR STUFF HERE
        //kaonfile->close(); 
	//delete kaonfile;
  
	//CALL THIS LAST
	DSelector::Finalize(); //Saves results to the output file
}
