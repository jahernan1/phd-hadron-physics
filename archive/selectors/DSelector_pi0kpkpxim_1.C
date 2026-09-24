#include "DSelector_pi0kpkpxim.h"
#include <stdio.h>

void DSelector_kpkpxim::Init(TTree *locTree)
{
    // USERS: IN THIS FUNCTION, ONLY MODIFY SECTIONS WITH A "USER" OR "EXAMPLE" LABEL. LEAVE THE REST ALONE.

	// The Init() function is called when the selector needs to initialize a new tree or chain.
	// Typically here the branch addresses and branch pointers of the tree will be set.
	// Init() will be called many times when running on PROOF (once per file to be processed).

	//USERS: SET OUTPUT FILE NAME //can be overriden by user in PROOF
	dOutputFileName = "kpkpxim.root"; //"" for none
	dOutputTreeFileName = ""; //"" for none
	dFlatTreeFileName = "flatTree_pi0kpkpxim.root"; //output flat tre (one combo per tree entry), "" for none
	dFlatTreeName = "flatTree_pi0kpkpxim"; //if blank, default name will be chosen

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
	std::deque<Particle_t> MyXimStar;
	MyXimStar.push_back(Pi); MyXimStar.push_back(XiMinus);

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
    
	dAnalysisActions.push_back(new DHistogramAction_InvariantMass(dComboWrapper,false, 1, MuXimStar, 1000, 1.2, 1.5, "Measured"));
	dAnalysisActions.push_back(new DHistogramAction_InvariantMass(dComboWrapper, true, 1, MuXimStar, 1000, 1.2, 1.5, "Kinfit"));

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
	dAnalysisActions.push_back(new DCutAction_KinFitChiSq(dComboWrapper, 8, "ChiSqNdfCut"));
	//CUT MISSING MASS
	dAnalysisActions.push_back(new DCutAction_MissingMassSquared(dComboWrapper, false, -0.02, 0.02));
    dAnalysisActions.push_back(new DCutAction_BeamEnergy(dComboWrapper, false, 6, 12));
    dAnalysisActions.push_back(new DCutAction_InvariantMass(dComboWrapper, false, XiMinus, 1.31, 1.34));
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
	dHist_BeamEnergy = new TH1F("BeamEnergy", ";Beam Energy (GeV)", 500, 6.0, 12.0);
	
	
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
    dFlatTreeInterface->Create_Branch_NoSplitTObject<TLorentzVector>("beam_p4_truth");
    dFlatTreeInterface->Create_Branch_NoSplitTObject<TLorentzVector>("kp1_p4_truth");
    dFlatTreeInterface->Create_Branch_NoSplitTObject<TLorentzVector>("kp1_p4_com");
    dFlatTreeInterface->Create_Branch_NoSplitTObject<TLorentzVector>("kp2_p4_com");
    dFlatTreeInterface->Create_Branch_NoSplitTObject<TLorentzVector>("kp1kp2_p4_meas");
    
    dFlatTreeInterface->Create_Branch_NoSplitTObject<TLorentzVector>("ystar_p4");
    dFlatTreeInterface->Create_Branch_NoSplitTObject<TLorentzVector>("ystar_p4_com");
    dFlatTreeInterface->Create_Branch_NoSplitTObject<TLorentzVector>("ystar_angle_p4");
    dFlatTreeInterface->Create_Branch_NoSplitTObject<TLorentzVector>("ystar_angle_p4_com");
    dFlatTreeInterface->Create_Branch_NoSplitTObject<TLorentzVector>("ystar_ellipse_p4");
    
    dFlatTreeInterface->Create_Branch_NoSplitTObject<TLorentzVector>("kplow_ellipse_p4");
    dFlatTreeInterface->Create_Branch_NoSplitTObject<TLorentzVector>("kplow_p4");
    dFlatTreeInterface->Create_Branch_NoSplitTObject<TLorentzVector>("kplow_p4_com");
    dFlatTreeInterface->Create_Branch_NoSplitTObject<TLorentzVector>("kplow_angle_p4");
    dFlatTreeInterface->Create_Branch_NoSplitTObject<TLorentzVector>("kplow_angle_p4_com");
    dFlatTreeInterface->Create_Branch_NoSplitTObject<TLorentzVector>("kplow_p4_ystar_hf");
    dFlatTreeInterface->Create_Branch_NoSplitTObject<TLorentzVector>("kplow_bcal_p4");
    
    dFlatTreeInterface->Create_Branch_NoSplitTObject<TLorentzVector>("kphigh_ellipse_p4");
    dFlatTreeInterface->Create_Branch_NoSplitTObject<TLorentzVector>("kphigh_p4");
    dFlatTreeInterface->Create_Branch_NoSplitTObject<TLorentzVector>("kphigh_p4_com");
    dFlatTreeInterface->Create_Branch_NoSplitTObject<TLorentzVector>("kphigh_fcal_p4");
    dFlatTreeInterface->Create_Branch_NoSplitTObject<TLorentzVector>("kphigh_angle_p4");
    dFlatTreeInterface->Create_Branch_NoSplitTObject<TLorentzVector>("kphigh_angle_p4_com");

    dFlatTreeInterface->Create_Branch_NoSplitTObject<TLorentzVector>("decayxim_p4");
    dFlatTreeInterface->Create_Branch_NoSplitTObject<TLorentzVector>("decayxim_p4_com");
    dFlatTreeInterface->Create_Branch_NoSplitTObject<TLorentzVector>("decayxim_p4_meas");
    dFlatTreeInterface->Create_Branch_NoSplitTObject<TLorentzVector>("lambda_p4");
    dFlatTreeInterface->Create_Branch_NoSplitTObject<TLorentzVector>("lambda_p4_meas");
    dFlatTreeInterface->Create_Branch_NoSplitTObject<TLorentzVector>("mm_kp1_p4");
    dFlatTreeInterface->Create_Branch_NoSplitTObject<TLorentzVector>("mm_kp2_p4");
    dFlatTreeInterface->Create_Branch_NoSplitTObject<TLorentzVector>("kphigh_pim1_p4");
    
    //get vertex position
    dFlatTreeInterface->Create_Branch_NoSplitTObject<TLorentzVector>("vertex0_x4");
    dFlatTreeInterface->Create_Branch_NoSplitTObject<TLorentzVector>("lambda_x4_meas");
    dFlatTreeInterface->Create_Branch_NoSplitTObject<TLorentzVector>("combobeam_x4");
    dFlatTreeInterface->Create_Branch_NoSplitTObject<TLorentzVector>("vertex1_x4");
    dFlatTreeInterface->Create_Branch_NoSplitTObject<TLorentzVector>("vertex2_x4");
        
    // beam stuff
 	dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("beam_E");
    dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("beam_E_Truth");
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
    dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("xim_costheta_hf");
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
    dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("t_ellipse_dist");
    dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("t_dist_fcal");
	dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("acc_weight");
	dFlatTreeInterface->Create_Branch_Fundamental<ULong64_t>("evnt_num");
    dFlatTreeInterface->Create_Branch_Fundamental<UInt_t>("combo_num");
    dFlatTreeInterface->Create_Branch_Fundamental<Int_t>("combos_survived");
    dFlatTreeInterface->Create_Branch_Fundamental<Int_t>("best_combo");
    dFlatTreeInterface->Create_Branch_Fundamental<Int_t>("best_combo_1");
    dFlatTreeInterface->Create_Branch_Fundamental<Int_t>("best_combo_rf");
    dFlatTreeInterface->Create_Branch_Fundamental<Int_t>("rf_intime_weight");
    dFlatTreeInterface->Create_Branch_Fundamental<Int_t>("num_unused_showers");
    dFlatTreeInterface->Create_Branch_Fundamental<UInt_t>("l1_trig_bit");
    dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("l1_bcal_en");
    dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("l1_fcal_en");
    
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
    //INSERT USER ANALYSIS UNIQUENESS TRACKING HERE

 	/******************************************* LOOP OVER THROWN DATA (OPTIONAL) ***************************************/

	//Thrown beam: just use directly
    Double_t locThrownBeamE=0;
    TLorentzVector locTruthBeamP4;
	if(dThrownBeam != NULL){
        locThrownBeamE = dThrownBeam->Get_P4().E();
        locTruthBeamP4 = dThrownBeam->Get_P4();
    }
	TLorentzVector locKPlus1P4_Truth;
	TLorentzVector locKPlus2P4_Truth;
    TLorentzVector locPi0P4_Truth;
	TLorentzVector locXiMinusP4_Truth;
	TLorentzVector locXiMinusX4_Truth;
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
        TLorentzVector locThrownX4 = dThrownWrapper->Get_X4();

		cout << "Thrown " << loc_j << ": " << locPID << ", " << locThrownP4.Px() << ", " << locThrownP4.Py() << ", " << locThrownP4.Pz() << ", " << locThrownP4.E() << endl;
		//cout << "\t Parent Particle Index: " << locParentIndex << endl;

		if(locPID == 11) {
		  if(loc_j == 0) {locKPlus1P4_Truth = locThrownP4; }
		  if(loc_j == 1) {locKPlus2P4_Truth = locThrownP4; }
		}
        if(locPID == 7) locPi0P4_Truth = locThrownP4;
          
		if(locPID == 9){
		  if(loc_j == 3) locPiMinus1P4_Truth = locThrownP4;
		  if(loc_j == 4) locPiMinus2P4_Truth = locThrownP4;
		}
        if(locPID == 14) locProtonP4_Truth = locThrownP4;		  
		if(locPID == 23) {locXiMinusP4_Truth = locThrownP4; locXiMinusX4_Truth = locThrownX4; }
		if(locPID == 18) {locLambdaP4_Truth = locThrownP4; }
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

	// Set up event level variables
	Int_t locNumChargedHypos = Get_NumChargedHypos();
	Int_t locNumNeutralHypos = Get_NumNeutralHypos();
	Int_t locNumCombos = Get_NumCombos();
    UInt_t locL1TriggerBits = Get_L1TriggerBits();
    // Set up best combo variables
	Int_t locNumComboSurvivedCut = 0;
    Double_t locBestChiSqNdf = 0;
    Double_t locBestConfidenceLvl = 0;
    UInt_t locBestChiSqNdfComboNum = 999;
    Double_t locBestChiSqNdf_1 = 0;
    UInt_t locBestChiSqNdfComboNum_1 = 999;
    ULong64_t locEventNum = Get_EventNumber();
    //Initialize vectors for combo tracking
    std::vector<std::vector<Int_t>> locVVComboBestCombo;
    std::vector<double> locVectChiSq;
    
    
    cout << "EVENT NUMBER: " << locEventNum <<"****************************************************" << endl;
    //Loop over combos for cuts
	for(UInt_t loc_i = 0; loc_i < Get_NumCombos(); ++loc_i)
	{
		//Set branch array indices for combo and all combo particles
		dComboWrapper->Set_ComboIndex(loc_i);
        cout << "COMBO NUMBER: " << loc_i << endl;
        cout << "************* " << loc_i << endl;
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
		Int_t locPhoton1NeutralID = dPhoton1Wrapper->Get_NeutralID();
		Int_t locPhoton2NeutralID = dPhoton2Wrapper->Get_NeutralID();

		//Step 2
		Int_t locPiMinus1TrackID = dPiMinus1Wrapper->Get_TrackID();

		//Step 3
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
		TLorentzVector locDecayingPi0P4 = dDecayingPi0Wrapper->Get_P4();
		TLorentzVector locPhoton1P4 = dPhoton1Wrapper->Get_P4();
		TLorentzVector locPhoton2P4 = dPhoton2Wrapper->Get_P4();
		//Step 2
		TLorentzVector locPiMinus1P4 = dPiMinus1Wrapper->Get_P4();
		//Step 3
		TLorentzVector locDecayingLambdaP4 = dDecayingLambdaWrapper->Get_P4();
		TLorentzVector locPiMinus2P4 = dPiMinus2Wrapper->Get_P4();
		TLorentzVector locProtonP4 = dProtonWrapper->Get_P4();

		// Get Measured P4's:
		//Step 0
		TLorentzVector locBeamP4_Measured = dComboBeamWrapper->Get_P4_Measured();
		TLorentzVector locKPlus1P4_Measured = dKPlus1Wrapper->Get_P4_Measured();
		TLorentzVector locKPlus2P4_Measured = dKPlus2Wrapper->Get_P4_Measured();
		//Step 1
		TLorentzVector locPhoton1P4_Measured = dPhoton1Wrapper->Get_P4_Measured();
		TLorentzVector locPhoton2P4_Measured = dPhoton2Wrapper->Get_P4_Measured();
		//Step 2
		TLorentzVector locPiMinus1P4_Measured = dPiMinus1Wrapper->Get_P4_Measured();
		//Step 3
		TLorentzVector locPiMinus2P4_Measured = dPiMinus2Wrapper->Get_P4_Measured();
		TLorentzVector locProtonP4_Measured = dProtonWrapper->Get_P4_Measured();

		/********************************************* GET COMBO RF TIMING INFO *****************************************/
        Int_t locNumOutOfTimeBunchesInTree;
        TLorentzVector locBeamX4_Measured = dComboBeamWrapper->Get_X4_Measured();
        Double_t locBunchPeriod = dAnalysisUtilities.Get_BeamBunchPeriod(Get_RunNumber());
        Double_t locDeltaT_RF = dAnalysisUtilities.Get_DeltaT_RF(Get_RunNumber(), locBeamX4_Measured, dComboWrapper);
        Int_t locRelBeamBucket = dAnalysisUtilities.Get_RelativeBeamBucket(Get_RunNumber(), locBeamX4_Measured, dComboWrapper); // 0 for in-time events, non-zero integer for out-of-time photons
        if(locRunNumber>30000 && locRunNumber<40000)//2017 data 1 beam bunch on each side
          locNumOutOfTimeBunchesInTree = 1;
        else
          locNumOutOfTimeBunchesInTree = 4; //2018-01 & 2018-08 has 4 bunched on each side
        //Number of out-of-time beam bunches in tree (on a single side, so that total number out-of-time bunches accepted is 2 times this number for left + right bunches)
        Bool_t locSkipNearestOutOfTimeBunch = false; // True: skip events from nearest out-of-time bunch on either side (recommended).
        Int_t locNumOutOfTimeBunchesToUse = locNumOutOfTimeBunchesInTree>1 ? locNumOutOfTimeBunchesInTree-1:locNumOutOfTimeBunchesInTree;
        Double_t locAccidentalScalingFactor = dAnalysisUtilities.Get_AccidentalScalingFactor(Get_RunNumber(), locBeamP4.E()); // Ideal value would be 1, but deviations observed: need added factor.
        Double_t locAccidentalScalingFactorError = dAnalysisUtilities.Get_AccidentalScalingFactorError(Get_RunNumber(), locBeamP4.E()); // Ideal value would be 1, but deviations observed, need added factor.
        Double_t locHistAccidWeightFactor = locRelBeamBucket==0 ? 1 : -locAccidentalScalingFactor/(2*locNumOutOfTimeBunchesToUse) ; // Weight by 1 for in-time events, ScalingFactor*(1/NBunches) for out-of-time
        if(locSkipNearestOutOfTimeBunch && abs(locRelBeamBucket)==1 && locNumOutOfTimeBunchesInTree>1)
          continue; // Skip nearest out-of-time bunch: tails of in-time distribution also leak in
        Int_t locInTimeBunch = locRelBeamBucket==0 ? 1:0;
        
		/********************************************* COMBINE FOUR-MOMENTUM ********************************************/

		// DO YOUR STUFF HERE

        TLorentzVector locMissingP4_Measured = locBeamP4_Measured + dTargetP4;
		locMissingP4_Measured -= locKPlus1P4_Measured + locKPlus2P4_Measured + locPhoton1P4_Measured + locPhoton2P4_Measured + locPiMinus1P4_Measured + locPiMinus2P4_Measured + locProtonP4_Measured;

        // TLorentzVector locMissingKpKpP4_Measured = locBeamP4_Measured + dTargetP4 - (locKPlus1P4_Measured + locKPlus2P4_Measured);
        TLorentzVector locDecayingLambdaP4_Measured = locProtonP4_Measured + locPiMinus2P4_Measured;
        TLorentzVector locXiMinusP4 = locDecayingLambdaP4 + locPiMinus1P4;
        TLorentzVector locXiMinusP4_Measured = locDecayingLambdaP4_Measured + locPiMinus1P4_Measured;  
        TLorentzVector locXiMinusStarP4 = locDecayingPi0P4 + locXiMinusP4;
        TLorentzVector locXiMinusStarP4_Measured = locDecayingPi0P4_Measured + locPhoton1P4_Measured + locPhoton1P4_Measured;  
        
        /*******************************
            Path Length Calculations
		*******************************/
		// TLorentzVector locDecayingLambdaP4 = locPiMinus2P4 +	locProtonP4; //Decaying Lambda for M18 is manually calculated
		TLorentzVector locProdSpacetimeVertex = dComboBeamWrapper->Get_X4();//Get production vertex
        TLorentzVector locVertex0X4 = dStep0Wrapper->Get_X4();
        // Xi Path Length
        TLorentzVector locVertex1X4 = dStep1Wrapper->Get_X4();
        TLorentzVector locDecayingXiX4 = dTreeInterface->Get_TObject<TLorentzVector>("DecayingXiMinus__X4",loc_i);
		TLorentzVector locDeltaSpacetimeXi = locDecayingXiX4 - locProdSpacetimeVertex;//displacement 4-vector
        TLorentzVector locDeltaMomentumXi = locBeamP4 - locXiMinusP4;//displaced momentum 4-vector
        Bool_t boolDeltaSpacetimeXi = TMath::Cos( locDeltaSpacetimeXi.Vect().Angle(locDeltaMomentumXi.Vect())) > 0;
        Double_t locPathLengthXi = boolDeltaSpacetimeXi ? locDeltaSpacetimeXi.Vect().Mag() : -1.0*locDeltaSpacetimeXi.Vect().Mag();//pathlength is just the magnitude
        Float_t locPathLengthSigmaXi = Get_Fundamental<Float_t>("DecayingXiMinus__PathLengthSigma", loc_i);
        Double_t locPathLengthSignificanceXi = locPathLengthXi/locPathLengthSigmaXi;
        Double_t locLifetimeXi = locDeltaSpacetimeXi.T();//lifetime of xi in lab
        Double_t locLifetimeRestFrameXi = locPathLengthXi*locXiMinusP4.M() / (29.9792458 * locXiMinusP4.P());//lifetime of xi in restframe t = (travel distance)*(xi mass)/(xi mom) // add 1/c[cm/ns] to correct dimensionality
        
		// Lambda Path Length
        TLorentzVector locVertex2X4 = dStep2Wrapper->Get_X4();
		TLorentzVector locDecayingLambdaX4 = dDecayingLambdaWrapper->Get_X4(); //Doesn't exist for M18
		TLorentzVector locDeltaSpacetimeLambda = locDecayingLambdaX4 - locDecayingXiX4;//vertex difference
        TLorentzVector locDeltaMomentumLambda = locXiMinusP4 - locDecayingLambdaP4;
        Bool_t boolDeltaSpacetimeLambda = TMath::Cos(locDeltaSpacetimeLambda.Angle(locDecayingLambdaP4.Vect())) > 0 ;
        Double_t locPathLengthLambda = boolDeltaSpacetimeLambda ? locDeltaSpacetimeLambda.Vect().Mag() : -1.0*locDeltaSpacetimeLambda.Vect().Mag();//pathlength is just the magnitude		
        Float_t locPathLengthSigmaLambda = dDecayingLambdaWrapper->Get_PathLengthSigma();
        Double_t locPathLengthSignificanceLambda = locPathLengthLambda/locPathLengthSigmaLambda;
        Double_t locLifetimeLambda = locDeltaSpacetimeLambda.T();//lifetime of lamb in lab
        Double_t locLifetimeRestFrameLambda = locPathLengthLambda*locDecayingLambdaP4_Measured.M() / (29.9792458 * locDecayingLambdaP4.P());//lifetime of lamb in restframe

		/******************************************** Get FOUR-POSITION **************************************************/

		TLorentzVector loc_beamX4 = dComboBeamWrapper->Get_X4();

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
        // if(locUsedSoFar_BeamEnergy.find(locBeamID) == locUsedSoFar_BeamEnergy.end())
        //   {
        //     cout << "BeamID in tracking: " << locBeamID << endl;
		// 	dHist_BeamEnergy->Fill(locBeamP4.E());
			
		// 	locUsedSoFar_BeamEnergy.insert(locBeamID);
        //   }

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

		//Missing Mass Squared
		Double_t locMissingMassSquared = locMissingP4_Measured.M2();
        
		//ChiSqNDF
		Double_t locChiSqNdf = dComboWrapper->Get_ChiSq_KinFit("") / dComboWrapper->Get_NDF_KinFit("");
        
        //Double_t locNDF = dComboWrapper->Get_NDF_KinFit("");
		//Double_t locChiSqNdf = locChiSq/locNDF;
        Double_t locConfidenceLvl = dComboWrapper->Get_ConfidenceLevel_KinFit("") ;

        //Shower Quality
        Float_t locPhoton1_Shower_Quality = dPhoton1Wrapper->Get_Shower_Quality();
        Float_t locPhoton2_Shower_Quality = dPhoton2Wrapper->Get_Shower_Quality();
        
        //Print PID
        cout << "BeamID " << " KPlus1ID " << " KPlus2ID " << " PiMinus1ID " << " PiMinus2ID " << " ProtonID " << " ChiSqNdf " << " InTime? " << endl;
        cout << locBeamID << setw(8) << locKPlus1TrackID << setw(10) << locKPlus2TrackID << setw(10) << locPiMinus1TrackID  << setw(12) << locPiMinus2TrackID << setw(12) << locProtonTrackID << setw(16) << locChiSqNdf << setw(4) << locInTimeBunch << endl;
        
        // Pre-cut isnan chisqndf cut
        if( std::isnan(locChiSqNdf) )
          {
            cout << "[WARNING]: Chisqndf is nan....." << endl; 
            dComboWrapper->Set_IsComboCut(true);
            continue;
          }
        //
        
        // skip all out of time beam bunches for best combo
        // if(abs(locRelBeamBucket)>0)
        //   { 
        //     dComboWrapper->Set_IsComboCut(true);
        //     continue;
        //   }

        if(locInTimeBunch==1)//only in-time events to catagorize
          {
            locNumComboSurvivedCut += 1;
            if(locNumComboSurvivedCut==1)//initiate first survived combo
              {
                locBestChiSqNdf = locChiSqNdf, locBestChiSqNdf_1 = locChiSqNdf; 
                locBestChiSqNdfComboNum = loc_i, locBestChiSqNdfComboNum_1 = loc_i; 
              }
            else //get best combo per event
              {

                
                if(locBestChiSqNdf > locChiSqNdf)
                  //if(locBestConfidenceLvl < locConfidenceLvl)//try with confidence level
                  locBestChiSqNdf = locChiSqNdf, locBestChiSqNdfComboNum = loc_i;
                
                if(abs(1.0-locBestChiSqNdf) > abs(1.0-locChiSqNdf))
                  locBestChiSqNdf_1 = locChiSqNdf, locBestChiSqNdfComboNum_1 = loc_i;
              }
          }
        
        locVectChiSq.push_back(locChiSqNdf);
        locVVComboBestCombo.push_back({(Int_t)loc_i, locBeamID, 1});// third argument is best_combo(1)
        //Get best combo for hydrid method
        if(locUsedSoFar_BeamEnergy.find(locBeamID) == locUsedSoFar_BeamEnergy.end())
          {
              cout << "Unique BeamID: " << endl;
              dHist_BeamEnergy->Fill(locBeamP4.E());
              locUsedSoFar_BeamEnergy.insert(locBeamID);
          }
        else
          {
            cout << "Repeat BeamID: " << endl;
            for(UInt_t prev_i=0; prev_i < locVectChiSq.size()-1; prev_i++)
              {
                // cout << "Indexing goes out of range?" << prev_i << " " << loc_i << endl;
                if(locVVComboBestCombo[prev_i][1] == locBeamID)//Find combo index with same photon
                  {
                    if(locVectChiSq[prev_i] < locChiSqNdf)//new element not best, nothing changes            
                      {
                        cout << "Combo Prev loc " << prev_i << ": " << locVectChiSq[prev_i] << " < " << locChiSqNdf << endl;
                        locVVComboBestCombo[prev_i][2] = 1;
                        locVVComboBestCombo.back()[2] = 0;            
                      }
                    else
                      {//new element is best
                        locVVComboBestCombo[prev_i][2] = 0;
                        locVVComboBestCombo.back()[2] = 1;            
                        cout << "New Combo Weight for " << prev_i << ": " << 0 << endl;
                      }
                  }
              }
          }
        cout << "Check Combo Weight " << loc_i << ": "<< locVVComboBestCombo.back()[2] << endl;
        //cout << "Check size of vectors " << locVectChiSq.size() << " " << locVVComboBestCombo.size() << endl;
        
		/*****************************************************
                             Perform Universal Cuts
        *****************************************************/
		//E.g. Cut
		if(locChiSqNdf > 30 )
		{
          dComboWrapper->Set_IsComboCut(true);
          //locUsedSoFar_MissingMass.insert(locUsedThisCombo_MissingMass);
          continue;
		}
		//E.g. Cut
		if(fabs(locMissingMassSquared) > 0.1 )
		{
          dComboWrapper->Set_IsComboCut(true);
          locUsedSoFar_MissingMass.insert(locUsedThisCombo_MissingMass);			
          continue;
		}
		// Beam Energy 
		if(locBeamP4.E() < 6.4 && locBeamP4.E() > 11.4)
		{
          dComboWrapper->Set_IsComboCut(true);
          locUsedSoFar_MissingMass.insert(locUsedThisCombo_MissingMass);			
          continue;
		}
        // Shower Quality
        // if(locPhoton1_Shower_Quality > 0.5 && locPhoto2_Shower_Quality > 0.5)
		// {
        //   dComboWrapper->Set_IsComboCut(true);
        //   locUsedSoFar_MissingMass.insert(locUsedThisCombo_MissingMass);			
        //   continue;
		// }
        
        // // vertex cuts
		// if(locProdSpacetimeVertex.Z() > 50.4 && locProdSpacetimeVertex.Z() < 79.1 && locDecayingXiX4.Z() > locDecayingLambdaX4.Z())
		// {
        //   dComboWrapper->Set_IsComboCut(true);
        //   locUsedSoFar_MissingMass.insert(locUsedThisCombo_MissingMass);			
        //   continue;
		// }

        // kaon momentum cut
		// if(locKPlusP4_highp.P() < 3.0 || locKPlusP4_lowp.P() >= 3.0)
		//   {
		//     dComboWrapper->Set_IsComboCut(true); 
        //     locUsedSoFar_MissingMass.insert(locUsedThisCombo_MissingMass);			
        //     continue;
		//   }
		
		// get the number of tracks used per survived combo
		Int_t locNumUnusedChargedTracks = dComboWrapper->Get_NumUnusedTracks();
		Int_t locNumUnusedNeutralTracks = dComboWrapper->Get_NumUnusedShowers();
		Int_t locNumReconChargedTracks = locNumChargedHypos + locNumUnusedChargedTracks; 
		Int_t locNumReconNeutralTracks = locNumNeutralHypos + locNumUnusedNeutralTracks; 

        /***************************************************************************************************************************************/
        
        //Fill User histograms
		
		/******************************************** EXECUTE ANALYSIS ACTIONS *******************************************/

		// Loop through the analysis actions, executing them in order for the active particle combo
		dAnalyzeCutActions->Perform_ActionWeight(locHistAccidWeightFactor);
		
		// Loop through the analysis actions, executing them in order for the active particle combo
		//dAnalyzeCutActions->Perform_Action(); // Must be executed before Execute_Actions()
		//if(!Execute_Actions()) //if the active combo fails a cut, IsComboCutFlag automatically set
        //continue;
		
		//if you manually execute any actions, and it fails a cut, be sure to call:
		//dComboWrapper->Set_IsComboCut(true);

		cout<< "*****************************************************************" << endl;

    } // end of combo loop for cuts

    //FILL HISTOGRAMS: Num combos / events surviving actions
	//dHist_NumCombosSurviveCut->Fill(locNumComboSurvivedCut);
	Fill_NumCombosSurvivedHists();
	
    /*********************************************************************************************************************/
   
    // begin combo loop to fill flat trees
    for(UInt_t loc_i = 0; loc_i < Get_NumCombos(); ++loc_i)
	{
		// Set branch array indices for combo and all combo particles
        UInt_t combo_index = dComboWrapper->Get_ComboIndex();	
        dComboWrapper->Set_ComboIndex(loc_i);
        
        //cout << "Combo_Index: " << combo_index << "\tBest Combo: " << locBestChiSqNdfComboNum << "\tBest Combo_1: " << locBestChiSqNdfComboNum_1 << endl;        
        // only keep best combo event for flat trees
        //if(locBestChiSqNdfComboNum != combo_index || locBestChiSqNdfComboNum_1 != combo_index) continue;
		
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

        		/********************************************** GET PARTICLE INDICES *********************************************/
		//Used for tracking uniqueness when filling histograms, and for determining unused particles

		//Step 0
		Int_t locBeamID = dComboBeamWrapper->Get_BeamID();
		Int_t locKPlus1TrackID = dKPlus1Wrapper->Get_TrackID();
		Int_t locKPlus2TrackID = dKPlus2Wrapper->Get_TrackID();

		//Step 1
		Int_t locPhoton1NeutralID = dPhoton1Wrapper->Get_NeutralID();
		Int_t locPhoton2NeutralID = dPhoton2Wrapper->Get_NeutralID();

		//Step 2
		Int_t locPiMinus1TrackID = dPiMinus1Wrapper->Get_TrackID();

		//Step 3
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
		TLorentzVector locDecayingPi0P4 = dDecayingPi0Wrapper->Get_P4();
		TLorentzVector locPhoton1P4 = dPhoton1Wrapper->Get_P4();
		TLorentzVector locPhoton2P4 = dPhoton2Wrapper->Get_P4();
		//Step 2
		TLorentzVector locPiMinus1P4 = dPiMinus1Wrapper->Get_P4();
		//Step 3
		TLorentzVector locDecayingLambdaP4 = dDecayingLambdaWrapper->Get_P4();
		TLorentzVector locPiMinus2P4 = dPiMinus2Wrapper->Get_P4();
		TLorentzVector locProtonP4 = dProtonWrapper->Get_P4();

		// Get Measured P4's:
		//Step 0
		TLorentzVector locBeamP4_Measured = dComboBeamWrapper->Get_P4_Measured();
		TLorentzVector locKPlus1P4_Measured = dKPlus1Wrapper->Get_P4_Measured();
		TLorentzVector locKPlus2P4_Measured = dKPlus2Wrapper->Get_P4_Measured();
		//Step 1
		TLorentzVector locPhoton1P4_Measured = dPhoton1Wrapper->Get_P4_Measured();
		TLorentzVector locPhoton2P4_Measured = dPhoton2Wrapper->Get_P4_Measured();
		//Step 2
		TLorentzVector locPiMinus1P4_Measured = dPiMinus1Wrapper->Get_P4_Measured();
		//Step 3
		TLorentzVector locPiMinus2P4_Measured = dPiMinus2Wrapper->Get_P4_Measured();
		TLorentzVector locProtonP4_Measured = dProtonWrapper->Get_P4_Measured();

		/********************************************* GET COMBO RF TIMING INFO *****************************************/

        Int_t locNumOutOfTimeBunchesInTree=1;
        TLorentzVector locBeamX4_Measured = dComboBeamWrapper->Get_X4_Measured();
        Double_t locBunchPeriod = dAnalysisUtilities.Get_BeamBunchPeriod(Get_RunNumber());
        Double_t locDeltaT_RF = dAnalysisUtilities.Get_DeltaT_RF(Get_RunNumber(), locBeamX4_Measured, dComboWrapper);
        Int_t locRelBeamBucket = dAnalysisUtilities.Get_RelativeBeamBucket(Get_RunNumber(), locBeamX4_Measured, dComboWrapper); // 0 for in-time events, non-zero integer for out-of-time photons
        if(locRunNumber>30000 && locRunNumber<40000)//2017 data 1 beam bunch on each side
          locNumOutOfTimeBunchesInTree = 1;
        else
          locNumOutOfTimeBunchesInTree = 4; //2018-01 & 2018-08 has 4 bunched on each side
        //Number of out-of-time beam bunches in tree (on a single side, so that total number out-of-time bunches accepted is 2 times this number for left + right bunches)
        
        Bool_t locSkipNearestOutOfTimeBunch = false; // True: skip events from nearest out-of-time bunch on either side (recommended).
        Int_t locNumOutOfTimeBunchesToUse = locNumOutOfTimeBunchesInTree>1 ? locNumOutOfTimeBunchesInTree-1:locNumOutOfTimeBunchesInTree;
        Double_t locAccidentalScalingFactor = dAnalysisUtilities.Get_AccidentalScalingFactor(Get_RunNumber(), locBeamP4.E()); // Ideal value would be 1, but deviations observed: need added factor.
        Double_t locAccidentalScalingFactorError = dAnalysisUtilities.Get_AccidentalScalingFactorError(Get_RunNumber(), locBeamP4.E()); // Ideal value would be 1, but deviations observed, need added factor.
        Double_t locHistAccidWeightFactor = locRelBeamBucket==0 ? 1 : -locAccidentalScalingFactor/(2*locNumOutOfTimeBunchesToUse) ; // Weight by 1 for in-time events, ScalingFactor*(1/NBunches) for out-of-time
        if(locSkipNearestOutOfTimeBunch && abs(locRelBeamBucket)==1 && locNumOutOfTimeBunchesInTree>1)
          continue; // Skip nearest out-of-time bunch: tails of in-time distribution also leak in
        Int_t locInTimeBunch = locRelBeamBucket==0 ? 1:0;

        // if(abs(locRelBeamBucket)>1)
        //   continue; Skip nearest out-of-time bunch: tails of in-time distribution also leak in
        
        /********************************************* COMBINE FOUR-MOMENTUM ********************************************/

		// DO YOUR STUFF HERE

		// Accidental Scale Factor
		//double scaling_factor = dAnalysisUtilities.Get_AccidentalScalingFactor(locRunNumber, locBeamP4.E());
		//double scaling_factor_err = dAnalysisUtilities.Get_AccidentalScalingFactorError(locRunNumber, locBeamP4.E());
		
		// Combine 4-vectors
		// Kaon Determincation
		TLorentzVector locKPlusLowP4;
        TLorentzVector locKPlusP4_lowtheta;
        TLorentzVector locKPlusLowEllipseP4;
		TLorentzVector locKPlusHighP4;
        TLorentzVector locKPlusP4_hightheta;
        TLorentzVector locKPlusHighEllipseP4;
		TLorentzVector locKPlusLowP4_Measured;
		TLorentzVector locKPlusHighP4_Measured;
		Int_t locKhighTrackID;
		Int_t locKlowTrackID;

        // Kaon determined from fcal bcal detectors
        TLorentzVector locKPlusLowP4_BCal;
        TLorentzVector locKPlusHighP4_FCal;
        Double_t kp1_efcal = dKPlus1Wrapper->Get_Energy_FCAL();
        Double_t kp1_ebcal = dKPlus1Wrapper->Get_Energy_BCAL();
        Double_t kp2_efcal = dKPlus2Wrapper->Get_Energy_FCAL();
        Double_t kp2_ebcal = dKPlus2Wrapper->Get_Energy_BCAL();
        
        // pick only events that are bcal(K2) and fcal(K1)
        if(kp1_efcal > 1e-1 && kp2_ebcal > 1e-1) {locKPlusHighP4_FCal = locKPlus1P4, locKPlusLowP4_BCal = locKPlus2P4;}
        else if(kp1_ebcal > 1e-1 && kp2_efcal > 1e-1) {locKPlusLowP4_BCal = locKPlus1P4, locKPlusHighP4_FCal = locKPlus2P4;}
        
		//Kaon Seperation with only momentum
		if(locKPlus1P4.P() > locKPlus2P4.P() )
		  {
		    locKPlusHighP4 = locKPlus1P4; 
		    locKPlusLowP4 = locKPlus2P4;
		    locKhighTrackID = locKPlus1TrackID;
		    locKlowTrackID = locKPlus2TrackID;
		    locKPlusHighP4_Measured = locKPlus1P4_Measured; 
		    locKPlusLowP4_Measured = locKPlus2P4_Measured;
		  }
		else
		  {
		    locKPlusHighP4 = locKPlus2P4; 
		    locKPlusLowP4 = locKPlus1P4;
		    locKhighTrackID = locKPlus2TrackID;
		    locKlowTrackID = locKPlus1TrackID;
		    locKPlusHighP4_Measured = locKPlus2P4_Measured; 
		    locKPlusLowP4_Measured = locKPlus1P4_Measured;
		  }
        //2D Kaon Seperation and cut
        if((pow((locKPlus2P4.P()-0.12)/2, 2) + pow((locKPlus1P4.P()-5.5)/3 , 2) <= 1) && locKPlus2P4.P() > 0.12 )
          {//kp1 high momentum region
            locKPlusHighEllipseP4 = locKPlus1P4, locKPlusLowEllipseP4 = locKPlus2P4;
          }
        else if(pow((locKPlus1P4.P()-0.12)/2, 2) + pow((locKPlus2P4.P()-5.5)/3 , 2) <= 1 && locKPlus1P4.P() > 0.12)
          {//kp2 low momentum region
            locKPlusHighEllipseP4 = locKPlus2P4, locKPlusLowEllipseP4 = locKPlus1P4;
          }
        
        //Kaon Seperation with only theta
        if(locKPlus1P4.Theta() < locKPlus2P4.Theta() )
          {
		    locKPlusP4_hightheta = locKPlus1P4; 
		    locKPlusP4_lowtheta = locKPlus2P4;
          }
        else
          {
		    locKPlusP4_hightheta = locKPlus2P4; 
		    locKPlusP4_lowtheta = locKPlus1P4;
          }

		Double_t locT = (locBeamP4-locKPlusHighP4).M2();
        Double_t locT_FCal = (locBeamP4-locKPlusHighP4_FCal).M2();
		Double_t locT_Ellipse = (locBeamP4-locKPlusHighEllipseP4).M2();
        Double_t locT_K1 = (locBeamP4-locKPlus1P4).M2();

		TLorentzVector locMissingP4_Measured = locBeamP4_Measured + dTargetP4;
        TLorentzVector locKPlus1KPlus2P4_Measured = locKPlus1P4_Measured + locKPlus2P4_Measured;
        TLorentzVector locDecayingLambdaP4_Measured = locProtonP4_Measured + locPiMinus2P4_Measured;
	  		
		TLorentzVector locXiMinusP4 = locDecayingLambdaP4 + locPiMinus1P4;
		TLorentzVector locXiMinusP4_Measured = locDecayingLambdaP4_Measured + locPiMinus1P4_Measured;
        TLorentzVector locDecayingXiX4 = dTreeInterface->Get_TObject<TLorentzVector>("DecayingXiMinus__X4",loc_i);
                
		TLorentzVector locYstar_Kp2Xim_P4 = locKPlus2P4 + locXiMinusP4;
		TLorentzVector locYstarAngleP4 = locKPlusP4_lowtheta + locXiMinusP4;
        TLorentzVector locYstarEllipseP4 = locKPlusLowEllipseP4 + locXiMinusP4;
        TLorentzVector locYstarP4 = locKPlusLowP4 + locXiMinusP4;
        TLorentzVector locYstarP4_Measured = locKPlusLowP4_Measured + locXiMinusP4_Measured;

		TLorentzVector locKPlusHigh_PiMinus1_P4 = locKPlusHighP4 + locPiMinus1P4;
		TLorentzVector locKPlusHigh_PiMinus2_P4 = locKPlusHighP4 + locPiMinus2P4;

		TLorentzVector locKPlusLow_PiMinus1_P4 = locKPlusLowP4 + locPiMinus1P4;
		TLorentzVector locKPlusLow_PiMinus2_P4 = locKPlusLowP4 + locPiMinus2P4;        
                
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
        TLorentzVector locBeamX4_CM = locBeamX4_Measured;
		//step 2
		TLorentzVector locKPlus1P4_CM = locKPlus1P4;
		TLorentzVector locKPlusHighP4_CM = locKPlusHighP4;
        TLorentzVector locKPlusP4_hightheta_CM = locKPlusP4_hightheta;
        TLorentzVector locYstarP4_CM = locYstarP4;
		TLorentzVector locYstarAngleP4_CM = locYstarAngleP4;
        //step 3
		TLorentzVector locXiMinusP4_CM = locXiMinusP4;
        TLorentzVector locXiMinusX4_CM = locDecayingXiX4;
		TLorentzVector locKPlus2P4_CM = locKPlus2P4;
		TLorentzVector locKPlusLowP4_CM = locKPlusLowP4;
		TLorentzVector locKPlusP4_lowtheta_CM = locKPlusP4_lowtheta;
        TLorentzVector locPiMinus1P4_CM = locPiMinus1P4;
		//step 4
		TLorentzVector locDecayingLambdaP4_CM = locDecayingLambdaP4;
		//Boost 4vectors in Com frame
		locBeamP4_CM.Boost(-boostCoM);
        locBeamX4_CM.Boost(-boostCoM);
		locYstarP4_CM.Boost(-boostCoM);
		locYstarAngleP4_CM.Boost(-boostCoM);
		locKPlus1P4_CM.Boost(-boostCoM);
		locKPlusHighP4_CM.Boost(-boostCoM);
        locKPlusP4_hightheta_CM.Boost(-boostCoM);
        locKPlus2P4_CM.Boost(-boostCoM);
		locKPlusLowP4_CM.Boost(-boostCoM);
        locKPlusP4_lowtheta_CM.Boost(-boostCoM);
        locXiMinusP4_CM.Boost(-boostCoM);
        locXiMinusX4_CM.Boost(-boostCoM);
		locPiMinus1P4_CM.Boost(-boostCoM);
		locDecayingLambdaP4_CM.Boost(-boostCoM);
		
		//Boost 4vector into rest frame of ystar from CoM
		TVector3 boostYstar_Rest = locYstarP4_CM.BoostVector();
		//Initialize 4vectors in ystar rest frame
		//step 1
		TLorentzVector locBeamP4_YstarRest = locBeamP4_CM;
		//step 2
		TLorentzVector locKPlus1P4_YstarRest = locKPlus1P4_CM;
		TLorentzVector locKPlusHighP4_YstarRest = locKPlusHighP4_CM;
        TLorentzVector locYstarP4_YstarRest = locYstarP4_CM;
		//step 3
		TLorentzVector locXiMinusP4_YstarRest = locXiMinusP4_CM;
		TLorentzVector locKPlus2P4_YstarRest = locKPlus2P4_CM;
		TLorentzVector locKPlusLowP4_YstarRest = locKPlusLowP4_CM;
        TLorentzVector locPiMinus1P4_YstarRest = locPiMinus1P4_CM;
		//step 4
		TLorentzVector locDecayingLambdaP4_YstarRest = locDecayingLambdaP4_CM;
		//Boost 4vectos to rest frame of ystar
		locBeamP4_YstarRest.Boost(-boostYstar_Rest);
		locYstarP4_YstarRest.Boost(-boostYstar_Rest);
		locKPlus1P4_YstarRest.Boost(-boostYstar_Rest);
		locKPlusHighP4_YstarRest.Boost(-boostYstar_Rest);
        locKPlus2P4_YstarRest.Boost(-boostYstar_Rest);
		locKPlusLowP4_YstarRest.Boost(-boostYstar_Rest);
        locXiMinusP4_YstarRest.Boost(-boostYstar_Rest);
		locPiMinus1P4_YstarRest.Boost(-boostYstar_Rest);
		locDecayingLambdaP4_YstarRest.Boost(-boostYstar_Rest);

        // Set helicity frame for ystar
        TVector3 z_hat_Ystar_HF = locYstarP4_CM.Vect().Unit();
        TVector3 y_hat_Ystar_HF = locBeamP4_CM.Vect().Cross(locYstarP4_CM.Vect()).Unit();//y direction normal to production plane gammap
        TVector3 x_hat_Ystar_HF = y_hat_Ystar_HF.Cross(z_hat_Ystar_HF);//maintain right handed coordinate system		
        TLorentzVector locKPlusP4_slow_HF(locKPlusLowP4_YstarRest.Vect().Dot(x_hat_Ystar_HF), locKPlusLowP4_YstarRest.Vect().Dot(y_hat_Ystar_HF), locKPlusLowP4_YstarRest.Vect().Dot(z_hat_Ystar_HF),locKPlusLowP4_YstarRest.E()); 
        Double_t KPlusLow_CosTheta_HF = locKPlusLowP4_YstarRest.Pz() / locKPlusLowP4_YstarRest.Vect().Mag();
        Double_t KPlusLow_Phi_HF = TMath::ATan(locKPlusLowP4_YstarRest.Py() / locKPlusLowP4_YstarRest.Px());
        TLorentzVector locXiMinusP4__YstarHF(locXiMinusP4_YstarRest.E(),locXiMinusP4_YstarRest.Vect().Dot(x_hat_Ystar_HF), locXiMinusP4_YstarRest.Vect().Dot(y_hat_Ystar_HF), locXiMinusP4_YstarRest.Vect().Dot(z_hat_Ystar_HF)); 
        Double_t XiMinus_CosTheta_HF = locXiMinusP4_YstarRest.Pz() / locXiMinusP4_YstarRest.Vect().Mag();
        Double_t XiMinus_Phi_HF = TMath::ATan(locXiMinusP4_YstarRest.Py() / locXiMinusP4_YstarRest.Px());

		//BoostVector in Xim rest frame from ystar rest frame
		TVector3 boostXim_HF = locXiMinusP4_YstarRest.BoostVector();
		//Initalize 4Vectors in Xim Rest frame
		//step 1
		TLorentzVector locBeamP4_HF = locBeamP4_YstarRest;
		//step 2
		TLorentzVector locKPlus1P4_HF = locKPlus1P4_YstarRest;
		TLorentzVector locKPlusHighP4_HF = locKPlusHighP4_YstarRest;
		TLorentzVector locYstarP4_HF = locYstarP4_YstarRest;
		//step 3
		TLorentzVector locXiMinusP4_HF = locXiMinusP4_YstarRest;
		TLorentzVector locKPlus2P4_HF = locKPlus2P4_YstarRest;
		TLorentzVector locKPlusLowP4_HF = locKPlusLowP4_YstarRest;
		TLorentzVector locPiMinus1P4_HF = locPiMinus1P4_YstarRest;
		//step 4
		TLorentzVector locDecayingLambdaP4_HF = locDecayingLambdaP4_YstarRest;
		//Boost 4vectors in Xim Rest frame
		locBeamP4_HF.Boost(-boostXim_HF);
		locKPlus1P4_HF.Boost(-boostXim_HF);
		locKPlusHighP4_HF.Boost(-boostXim_HF);
		locKPlus2P4_HF.Boost(-boostXim_HF);
		locKPlusLowP4_HF.Boost(-boostXim_HF);
		locXiMinusP4_HF.Boost(-boostXim_HF);
		locPiMinus1P4_HF.Boost(-boostXim_HF);
		locDecayingLambdaP4_HF.Boost(-boostXim_HF);
		//Create the GF reference frame
		TVector3 z_hat_HF = locXiMinusP4_YstarRest.Vect().Unit();//z direction is opposite direction of the boost or Ystar in rest frame
        TVector3 y_hat_HF = locXiMinusP4_YstarRest.Vect().Cross(locPiMinus1P4_YstarRest.Vect()).Unit();//y direction normal to production plane
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
		// TLorentzVector locDecayingLambdaP4 = locPiMinus2P4 +	locProtonP4; //Decaying Lambda for M18 is manually calculated
		TLorentzVector locProdSpacetimeVertex = dComboBeamWrapper->Get_X4();//Get production vertex
        // Xi Path Length
        TLorentzVector locDeltaSpacetimeXi = locDecayingXiX4 - locProdSpacetimeVertex;//displacement 4-vector
        TLorentzVector locDeltaMomentumXi = locBeamP4 - locXiMinusP4;//displaced momentum 4-vector
        Int_t locDeltaSpacetimeSign = std::round(TMath::Cos( locDeltaSpacetimeXi.Vect().Angle(locXiMinusP4.Vect())));
        // cout << "CASCADE****************************************" << endl;
        // cout << "Pathlen sign: " << locDeltaSpacetimeSign << endl;
        // cout << "Vertex Difference: " << locDecayingXiX4.Z() - locProdSpacetimeVertex.Z() << endl;
        //
        Double_t locPathLengthXi = locDeltaSpacetimeSign*locDeltaSpacetimeXi.Vect().Mag();//pathlength is just the magnitude
        Float_t locPathLengthSigmaXi = Get_Fundamental<Float_t>("DecayingXiMinus__PathLengthSigma", loc_i);
        Double_t locPathLengthSignificanceXi = locPathLengthXi/locPathLengthSigmaXi;
        Double_t locLifetimeXi = locDeltaSpacetimeXi.T();//lifetime of xi in lab
        Double_t locLifetimeRestFrameXi = locPathLengthXi*locXiMinusP4.M() / (29.9792458 * locXiMinusP4.P());//lifetime of xi in restframe t = (travel distance)*(xi mass)/(xi mom) // add 1/c[cm/ns] to correct dimensionality
        
		// Lambda Path Length
        TLorentzVector locDecayingLambdaX4 = dDecayingLambdaWrapper->Get_X4(); //Doesn't exist for M18
        TLorentzVector locDeltaSpacetimeLambda = locDecayingLambdaX4 - locDecayingXiX4;//vertex difference
        TLorentzVector locDeltaMomentumLambda = locXiMinusP4 - locDecayingLambdaP4;
        Int_t locLambdaPathlenSign = std::round(TMath::Cos(locDeltaSpacetimeLambda.Vect().Angle(locDecayingLambdaP4.Vect())));
        // cout << "LAMBDA***************************************" << endl;
        // cout << "Pathlen Sign: " << locLambdaPathlenSign << endl;
        // cout << "Vertex Difference: " << locDecayingLambdaX4.Z() - locDecayingXiX4.Z() << endl;
        //
        Double_t locPathLengthLambda = locLambdaPathlenSign*locDeltaSpacetimeLambda.Vect().Mag();//pathlength is just the magnitude
        Float_t locPathLengthSigmaLambda = dDecayingLambdaWrapper->Get_PathLengthSigma();
        Double_t locPathLengthSignificanceLambda = locPathLengthLambda/locPathLengthSigmaLambda;
        Double_t locLifetimeLambda = locDeltaSpacetimeLambda.T();//lifetime of lamb in lab
        Double_t locLifetimeRestFrameLambda = locPathLengthLambda*locDecayingLambdaP4_Measured.M() / (29.9792458 * locDecayingLambdaP4.P());//lifetime of lamb in restframe

        //Make wrappers to test vertex
        Get_ComboWrappers();
        TLorentzVector locVertex0X4 = dStep0Wrapper->Get_X4();
        TLorentzVector locComboBeamX4 = dTreeInterface->Get_TObject<TLorentzVector>("ComboBeam__X4_KinFit",loc_i);
        TLorentzVector locVertex1X4 = dStep1Wrapper->Get_X4();
        TLorentzVector locVertex2X4 = dStep2Wrapper->Get_X4();
        //TLorentzVector locLambdaX4_Measured = dDecayingLambdaWrapper->Get_X4_Measured();

        // Print vertices to check
        // if(boolDeltaSpacetimeXi && boolDeltaSpacetimeLambda)
        //   {
        //     cout << "Paticle Vertex Beam: " << locProdSpacetimeVertex.Z() << "\t Vertex: " << locVertex0X4.Z() << "\t" << locComboBeamX4.Z()  << endl;
        //     cout << "Paticle Vertex XiMinus: " << locDecayingXiX4.Z() << "\t Vertex: " << locVertex1X4.Z() << "\t Decay Particle: " << locPiMinus1X4.Z() <<  endl;
        //     cout << "Paticle Vertex Lambda: " << locDecayingLambdaX4.Z() << "\t Vertex: " << locVertex2X4.Z() << "\t Decay Particle: " << locPiMinus2X4.Z() << "\n" << endl;
        //   }
        
		//Missing Mass Squared
		Double_t locMissingMassSquared = locMissingP4_Measured.M2();

		//ChiSqNDF
		Double_t locChiSqNdf = dComboWrapper->Get_ChiSq_KinFit("") / dComboWrapper->Get_NDF_KinFit("");
        Double_t locConfidenceLvl = dComboWrapper->Get_ConfidenceLevel_KinFit("");

        Double_t locL1BCALEnergy = Get_L1BCALEnergy();
        Double_t locL1FCALEnergy = Get_L1FCALEnergy();

        // get the number of tracks used per survived combo
		Int_t locNumUnusedChargedTracks = dComboWrapper->Get_NumUnusedTracks();
		Int_t locNumUnusedNeutralTracks = dComboWrapper->Get_NumUnusedShowers();
		Int_t locNumReconChargedTracks = locNumChargedHypos + locNumUnusedChargedTracks; 
		Int_t locNumReconNeutralTracks = locNumNeutralHypos + locNumUnusedNeutralTracks; 

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
        dFlatTreeInterface->Fill_TObject<TLorentzVector>("beam_p4_truth", locTruthBeamP4);
        dFlatTreeInterface->Fill_TObject<TLorentzVector>("kp1_p4_truth", locKPlus1P4_Truth);
        dFlatTreeInterface->Fill_TObject<TLorentzVector>("kp1_p4_com", locKPlus1P4_CM);
        dFlatTreeInterface->Fill_TObject<TLorentzVector>("kp2_p4_com", locKPlus2P4_CM);
        dFlatTreeInterface->Fill_TObject<TLorentzVector>("kp1kp2_p4_meas", locKPlus1KPlus2P4_Measured);
        
        dFlatTreeInterface->Fill_TObject<TLorentzVector>("ystar_p4", locYstarP4);
        dFlatTreeInterface->Fill_TObject<TLorentzVector>("ystar_p4_com", locYstarP4_CM);
        dFlatTreeInterface->Fill_TObject<TLorentzVector>("ystar_angle_p4", locYstarAngleP4);
        dFlatTreeInterface->Fill_TObject<TLorentzVector>("ystar_angle_p4_com", locYstarAngleP4_CM);
        dFlatTreeInterface->Fill_TObject<TLorentzVector>("ystar_ellipse_p4", locYstarEllipseP4);

        dFlatTreeInterface->Fill_TObject<TLorentzVector>("kplow_ellipse_p4", locKPlusLowEllipseP4);
        dFlatTreeInterface->Fill_TObject<TLorentzVector>("kplow_p4", locKPlusLowP4);
        dFlatTreeInterface->Fill_TObject<TLorentzVector>("kplow_p4_com", locKPlusLowP4_CM);
        dFlatTreeInterface->Fill_TObject<TLorentzVector>("kplow_angle_p4", locKPlusP4_lowtheta);
        dFlatTreeInterface->Fill_TObject<TLorentzVector>("kplow_angle_p4_com", locKPlusP4_lowtheta_CM);
        dFlatTreeInterface->Fill_TObject<TLorentzVector>("kplow_bcal_p4", locKPlusLowP4_BCal);
        
        dFlatTreeInterface->Fill_TObject<TLorentzVector>("kphigh_ellipse_p4", locKPlusHighEllipseP4);
        dFlatTreeInterface->Fill_TObject<TLorentzVector>("kphigh_p4", locKPlusHighP4);
        dFlatTreeInterface->Fill_TObject<TLorentzVector>("kphigh_p4_com", locKPlusHighP4_CM);
        dFlatTreeInterface->Fill_TObject<TLorentzVector>("kphigh_angle_p4", locKPlusP4_hightheta);
        dFlatTreeInterface->Fill_TObject<TLorentzVector>("kphigh_angle_p4_com", locKPlusP4_hightheta_CM);
        dFlatTreeInterface->Fill_TObject<TLorentzVector>("kphigh_fcal_p4", locKPlusHighP4_FCal);

        dFlatTreeInterface->Fill_TObject<TLorentzVector>("kplow_p4_ystar_hf", locKPlusP4_slow_HF);
        
        dFlatTreeInterface->Fill_TObject<TLorentzVector>("decayxim_p4", locXiMinusP4);
        dFlatTreeInterface->Fill_TObject<TLorentzVector>("decayxim_p4_com", locXiMinusP4_CM);
        dFlatTreeInterface->Fill_TObject<TLorentzVector>("decayxim_p4_meas", locXiMinusP4_Measured);
        dFlatTreeInterface->Fill_TObject<TLorentzVector>("lambda_p4", locDecayingLambdaP4);
        dFlatTreeInterface->Fill_TObject<TLorentzVector>("lambda_p4_meas", locDecayingLambdaP4_Measured);
        //vertex x4
        dFlatTreeInterface->Fill_TObject<TLorentzVector>("vertex0_x4", locVertex0X4);
        dFlatTreeInterface->Fill_TObject<TLorentzVector>("combobeam_x4", locComboBeamX4);
        dFlatTreeInterface->Fill_TObject<TLorentzVector>("vertex1_x4", locVertex1X4);
        dFlatTreeInterface->Fill_TObject<TLorentzVector>("vertex2_x4", locVertex2X4);
        //
        //dFlatTreeInterface->Fill_TObject<TLorentzVector>("lambda_x4_meas", locLambdaX4_Measured);

        // beam stuff
        dFlatTreeInterface->Fill_Fundamental<Double_t>("beam_E", locBeamP4.E());
        dFlatTreeInterface->Fill_Fundamental<Double_t>("beam_E_Truth", locThrownBeamE);
        dFlatTreeInterface->Fill_Fundamental<Double_t>("beam_rfbunches", locDeltaT_RF);
        dFlatTreeInterface->Fill_Fundamental<Double_t>("beam_vertexX", locProdSpacetimeVertex.X());
        dFlatTreeInterface->Fill_Fundamental<Double_t>("beam_vertexY", locProdSpacetimeVertex.Y());
        dFlatTreeInterface->Fill_Fundamental<Double_t>("beam_vertexZ", locProdSpacetimeVertex.Z());
        // kp1
        dFlatTreeInterface->Fill_Fundamental<Double_t>("kp1_P3", locKPlus1P4.P() );
        dFlatTreeInterface->Fill_Fundamental<Double_t>("kp1_P3_Truth", locKPlus1P4_Truth.P() );
        dFlatTreeInterface->Fill_Fundamental<Double_t>("kp1_CosTheta", locKPlus1P4.CosTheta() );
        dFlatTreeInterface->Fill_Fundamental<Double_t>("kp1_Phi", locKPlus1P4.Phi() );
        dFlatTreeInterface->Fill_Fundamental<Double_t>("kp_highp_P3", locKPlusHighP4.P() );
        dFlatTreeInterface->Fill_Fundamental<Double_t>("kp_highp_Theta", locKPlusHighP4.Theta() );
        dFlatTreeInterface->Fill_Fundamental<Double_t>("kp_highp_CosTheta", locKPlusHighP4.CosTheta() );
        dFlatTreeInterface->Fill_Fundamental<Double_t>("kp_highp_Phi", locKPlusHighP4.Phi() );
        // kp2
        dFlatTreeInterface->Fill_Fundamental<Double_t>("kp2_P3", locKPlus2P4.P() );
        dFlatTreeInterface->Fill_Fundamental<Double_t>("kp2_P3_Truth", locKPlus2P4_Truth.P() );
        dFlatTreeInterface->Fill_Fundamental<Double_t>("kp_lowp_Theta", locKPlusLowP4.Theta() );
        dFlatTreeInterface->Fill_Fundamental<Double_t>("kp2_YstarRest_CosTheta", locKPlus2P4_YstarRest.CosTheta() );
        dFlatTreeInterface->Fill_Fundamental<Double_t>("kp2_YstarRest_Phi", locKPlus2P4_YstarRest.Phi() );
        dFlatTreeInterface->Fill_Fundamental<Double_t>("kp_lowp_P3", locKPlusLowP4.P() );
        dFlatTreeInterface->Fill_Fundamental<Double_t>("kplow_costheta_hf",  KPlusLow_CosTheta_HF);
        dFlatTreeInterface->Fill_Fundamental<Double_t>("kplow_phi_hf",  KPlusLow_Phi_HF);
        // ystar
        dFlatTreeInterface->Fill_Fundamental<Double_t>("ystar_M", locYstarP4.M());			  
        dFlatTreeInterface->Fill_Fundamental<Double_t>("ystar_P3", locYstarP4.P() );
        dFlatTreeInterface->Fill_Fundamental<Double_t>("ystar_Theta", locYstarP4.CosTheta() );
        // ximinus
        dFlatTreeInterface->Fill_Fundamental<Double_t>("decayxim_M", locXiMinusP4.M());
        dFlatTreeInterface->Fill_Fundamental<Double_t>("xim_P3", locXiMinusP4.P() );
        dFlatTreeInterface->Fill_Fundamental<Double_t>("xim_costheta_hf", XiMinus_CosTheta_HF );
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
        dFlatTreeInterface->Fill_Fundamental<Double_t>("t_ellipse_dist", -1.0*locT_Ellipse);
        dFlatTreeInterface->Fill_Fundamental<Double_t>("t_dist_fcal", -1.0*locT_FCal);
        dFlatTreeInterface->Fill_Fundamental<ULong64_t>("evnt_num", locEventNum);
        dFlatTreeInterface->Fill_Fundamental<UInt_t>("combo_num", Get_NumCombos());
        dFlatTreeInterface->Fill_Fundamental<Int_t>("combos_survived", locNumComboSurvivedCut);
        dFlatTreeInterface->Fill_Fundamental<Int_t>("rf_intime_weight", locInTimeBunch);
        
        if(locBestChiSqNdfComboNum == loc_i) dFlatTreeInterface->Fill_Fundamental<Int_t>("best_combo", 1);
        else dFlatTreeInterface->Fill_Fundamental<Int_t>("best_combo", 0);
        
        if(locBestChiSqNdfComboNum_1 == loc_i) dFlatTreeInterface->Fill_Fundamental<Int_t>("best_combo_1", 1);
        else dFlatTreeInterface->Fill_Fundamental<Int_t>("best_combo_1", 0);

        for(UInt_t loc_bc=0; loc_bc < locVVComboBestCombo.size(); ++loc_bc)
          {
            if( (UInt_t)locVVComboBestCombo[loc_bc][0]==loc_i)
              dFlatTreeInterface->Fill_Fundamental<Int_t>("best_combo_rf", locVVComboBestCombo[loc_bc][2]);
          }

        dFlatTreeInterface->Fill_Fundamental<Int_t>("num_unused_showers", locNumUnusedNeutralTracks);
        dFlatTreeInterface->Fill_Fundamental<UInt_t>("l1_trig_bit", locL1TriggerBits);
        dFlatTreeInterface->Fill_Fundamental<Double_t>("l1_fcal_en", locL1FCALEnergy);
        dFlatTreeInterface->Fill_Fundamental<Double_t>("l1_bcal_en", locL1BCALEnergy);
        
        //cout << "ChiSqNdf: " << locChiSqNdf << "\nXiPathLen: " << locPathLengthXi << "\nXiPathLenSigma: " << locPathLengthSigmaXi << "\nXiPathLenSignificance: " << locPathLengthSignificanceXi << "\n" << endl;
			  
        //FILL FLAT TREE
        Fill_FlatTree(); //for the active combo		   
    }// end combo loop for flat trees
	
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
*/
	//Loop over neutral particle hypotheses
	for(UInt_t loc_i = 0; loc_i < Get_NumNeutralHypos(); ++loc_i)
	{
		//Set branch array indices corresponding to this particle
		dNeutralHypoWrapper->Set_ArrayIndex(loc_i);
        
        
		//Do stuff with the wrapper here ...
	}


	/************************************ Example: FILL CLONE OF TTREE HERE WITH CUTS APPLIED ************************************/
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
