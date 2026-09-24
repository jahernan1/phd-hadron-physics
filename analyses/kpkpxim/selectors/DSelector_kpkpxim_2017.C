#include "DSelector_kpkpxim_2017.h"

void DSelector_kpkpxim_2017::Init(TTree *locTree)
{
	// USERS: IN THIS FUNCTION, ONLY MODIFY SECTIONS WITH A "USER" OR "EXAMPLE" LABEL. LEAVE THE REST ALONE.

	// The Init() function is called when the selector needs to initialize a new tree or chain.
	// Typically here the branch addresses and branch pointers of the tree will be set.
	// Init() will be called many times when running on PROOF (once per file to be processed).

	//USERS: SET OUTPUT FILE NAME //can be overriden by user in PROOF
	dOutputFileName = "kpkpxim2017.root"; //"" for none
	dOutputTreeFileName = ""; //"" for none
	dFlatTreeFileName = ""; //output flat tree (one combo per tree entry), "" for none
	dFlatTreeName = ""; //if blank, default name will be chosen

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
	// // For histogramming the phi mass in phi -> K+ K- //gamma p -> K-K+ Xi-
	// // Be sure to change this and dAnalyzeCutActions to match reaction
	std::deque<Particle_t> MyXim;
	MyXim.push_back(PiMinus); MyXim.push_back(Lambda);
	std::deque<Particle_t> MyLambda;
	MyLambda.push_back(PiMinus); MyLambda.push_back(Proton);

       	//ANALYSIS ACTIONS: //Executed in order if added to dAnalysisActions
	//false/true below: use measured/kinfit data
	
	//KinFit Results
	//dAnalysisActions.push_back(new DHistogramAction_KinFitResults(dComboWrapper, false));

	//KINEMATICS (NO CUT) 
	dAnalysisActions.push_back(new DHistogramAction_ParticleComboKinematics(dComboWrapper, false, " "));
	//dAnalysisActions.push_back(new DHistogramAction_ParticleComboKinematics(dComboWrapper, true, "Initial_KinFit"));
	
	//PID (NO CUT)
	dAnalysisActions.push_back(new DHistogramAction_ParticleID(dComboWrapper, false, " "));
	//dAnalysisActions.push_back(new DHistogramAction_ParticleID(dComboWrapper, true, "Initial_KinFit"));

	//MASSES (NO CUT)
	dAnalysisActions.push_back(new DHistogramAction_InvariantMass(dComboWrapper,false, 1, MyXim, 1000, 1.0, 1.5, "MyXim"));
	dAnalysisActions.push_back(new DHistogramAction_InvariantMass(dComboWrapper, true, 1, MyXim, 1000, 1.0, 1.5, "MyXim_Kinfit"));

	dAnalysisActions.push_back(new DHistogramAction_MissingMassSquared(dComboWrapper, false, 1000, -0.1, 0.1, "Measured"));
	dAnalysisActions.push_back(new DHistogramAction_MissingMassSquared(dComboWrapper, true, 1000, -0.1, 0.1, "KinFit"));
	
	//CUT CHISQ
	//dAnalysisActions.push_back(new DCutAction_KinFitChiSq(dComboWrapper, 35, "ChiSqCut"));
	
	//KINEMATICS
	dAnalysisActions.push_back(new DHistogramAction_ParticleComboKinematics(dComboWrapper, false, "Initial"));
	//dAnalysisActions.push_back(new DHistogramAction_ParticleComboKinematics(dComboWrapper, true, "Initial_KinFit"));
	
	//PID
	dAnalysisActions.push_back(new DHistogramAction_ParticleID(dComboWrapper, false, "Initial"));
	//dAnalysisActions.push_back(new DHistogramAction_ParticleID(dComboWrapper, true, "Initial_KinFit"));

	//MASSES
	dAnalysisActions.push_back(new DHistogramAction_InvariantMass(dComboWrapper,false, 1, MyXim, 1000, 1.0, 1.5, "Initial_MyXim"));
	dAnalysisActions.push_back(new DHistogramAction_InvariantMass(dComboWrapper, true, 1, MyXim, 1000, 1.0, 1.5, "Initial_MyXim_Kinfit"));

	dAnalysisActions.push_back(new DHistogramAction_MissingMassSquared(dComboWrapper, false, 1000, -0.1, 0.1, "Initial"));
	dAnalysisActions.push_back(new DHistogramAction_MissingMassSquared(dComboWrapper, true, 1000, -0.1, 0.1, "Initial_KinFit"));

	//KINFIT RESULTS
	dAnalysisActions.push_back(new DHistogramAction_KinFitResults(dComboWrapper));
	
	
	//CUT MISSING MASS
	dAnalysisActions.push_back(new DCutAction_MissingMassSquared(dComboWrapper, false, -0.04, 0.04));
	dAnalysisActions.push_back(new DCutAction_MissingMassSquared(dComboWrapper, true, -0.04, 0.04));
	
	//below: value: +/- N ns, Unknown: All PIDs, SYS_NULL: all timing systems
	//NOMINAL PID TIMING CUTS (https://halldweb.jlab.org/wiki/index.php/PID_study_proposal)
	dAnalysisActions.push_back(new DCutAction_PIDDeltaT(dComboWrapper, false, 0.75, KPlus, SYS_BCAL));
	dAnalysisActions.push_back(new DCutAction_PIDDeltaT(dComboWrapper, false, 0.3, KPlus, SYS_TOF));
	dAnalysisActions.push_back(new DCutAction_PIDDeltaT(dComboWrapper, false, 2.5, KPlus, SYS_FCAL));
	dAnalysisActions.push_back(new DCutAction_PIDDeltaT(dComboWrapper, false, 2.5, KPlus, SYS_START));
	dAnalysisActions.push_back(new DCutAction_PIDDeltaT(dComboWrapper, true, 0.75, KPlus, SYS_BCAL));
	dAnalysisActions.push_back(new DCutAction_PIDDeltaT(dComboWrapper, true, 0.3, KPlus, SYS_TOF));
	dAnalysisActions.push_back(new DCutAction_PIDDeltaT(dComboWrapper, true, 2.5, KPlus, SYS_FCAL));
	dAnalysisActions.push_back(new DCutAction_PIDDeltaT(dComboWrapper, true, 2.5, KPlus, SYS_START));

	dAnalysisActions.push_back(new DCutAction_PIDDeltaT(dComboWrapper, false, 1.0, Proton, SYS_BCAL));
	dAnalysisActions.push_back(new DCutAction_PIDDeltaT(dComboWrapper, false, 0.6, Proton, SYS_TOF));
	dAnalysisActions.push_back(new DCutAction_PIDDeltaT(dComboWrapper, false, 2.0, Proton, SYS_FCAL));
	dAnalysisActions.push_back(new DCutAction_PIDDeltaT(dComboWrapper, false, 2.5, Proton, SYS_START));
	dAnalysisActions.push_back(new DCutAction_PIDDeltaT(dComboWrapper, true, 1.0, Proton, SYS_BCAL));
	dAnalysisActions.push_back(new DCutAction_PIDDeltaT(dComboWrapper, true, 0.6, Proton, SYS_TOF));
	dAnalysisActions.push_back(new DCutAction_PIDDeltaT(dComboWrapper, true, 2.0, Proton, SYS_FCAL));
	dAnalysisActions.push_back(new DCutAction_PIDDeltaT(dComboWrapper, true, 2.5, Proton, SYS_START));
	
	dAnalysisActions.push_back(new DCutAction_PIDDeltaT(dComboWrapper, false, 1.0, PiMinus, SYS_BCAL));
	dAnalysisActions.push_back(new DCutAction_PIDDeltaT(dComboWrapper, false, 0.5, PiMinus, SYS_TOF));
	dAnalysisActions.push_back(new DCutAction_PIDDeltaT(dComboWrapper, false, 2.0, PiMinus, SYS_FCAL));
	dAnalysisActions.push_back(new DCutAction_PIDDeltaT(dComboWrapper, false, 2.5, PiMinus, SYS_START));
	dAnalysisActions.push_back(new DCutAction_PIDDeltaT(dComboWrapper, true, 1.0, PiMinus, SYS_BCAL));
	dAnalysisActions.push_back(new DCutAction_PIDDeltaT(dComboWrapper, true, 0.5, PiMinus, SYS_TOF));
	dAnalysisActions.push_back(new DCutAction_PIDDeltaT(dComboWrapper, true, 2.0, PiMinus, SYS_FCAL));
	dAnalysisActions.push_back(new DCutAction_PIDDeltaT(dComboWrapper, true, 2.5, PiMinus, SYS_START));

       	//KINEMATICS(FINAL)
	dAnalysisActions.push_back(new DHistogramAction_ParticleComboKinematics(dComboWrapper, false, "Final"));
	dAnalysisActions.push_back(new DHistogramAction_ParticleComboKinematics(dComboWrapper, true, "Final_KinFit"));
	
	//PID (FINAL)
	dAnalysisActions.push_back(new DHistogramAction_ParticleID(dComboWrapper, false, "Final")); 
	dAnalysisActions.push_back(new DHistogramAction_ParticleID(dComboWrapper, true, "Final_Kinfit"));

	//Masses(FINAL)
	dAnalysisActions.push_back(new DHistogramAction_InvariantMass(dComboWrapper, false, 1, MyXim, 1000, 1.0, 1.5, "Final_MyXim"));
	dAnalysisActions.push_back(new DHistogramAction_InvariantMass(dComboWrapper, true, 1, MyXim, 1000, 1.0, 1.5, "Final_MyXim_Kinfit"));
	
	//BEAM ENERGY
	//dAnalysisActions.push_back(new DHistogramAction_BeamEnergy(dComboWrapper, false));
	dAnalysisActions.push_back(new DCutAction_BeamEnergy(dComboWrapper, false, 8.0, 12.0));

	
	// ANALYZE CUT ACTIONS
	// // Change MyXi to match reaction
	dAnalyzeCutActions = new DHistogramAction_AnalyzeCutActions( dAnalysisActions, dComboWrapper, false, 1, MyXim, 1000, 0.9, 2.4, "CutActionEffect");
	//dAnalyzeCutActions = new DHistogramAction_AnalyzeCutActions( dAnalysisActions, dComboWrapper, true, 1, MyXim, 1000, 0.9, 2.4, "CutActionEffect_KinFit");

	
	//INITIALIZE ACTIONS
	//If you create any actions that you want to run manually (i.e. don't add to dAnalysisActions), be sure to initialize them here as well
	Initialize_Actions();
	dAnalyzeCutActions->Initialize(); // manual action, must call Initialize()

	/******************************** EXAMPLE USER INITIALIZATION: STAND-ALONE HISTOGRAMS *******************************/

	//EXAMPLE MANUAL HISTOGRAMS:
	dHist_MissingMassSquared = new TH1I("MissingMassSquared", ";Missing Mass Squared (GeV/c^{2})^{2}", 600, -0.06, 0.06);
	dHist_BeamEnergy = new TH1I("BeamEnergy", ";Beam Energy (GeV)", 600, 0.0, 12.0);
	
	dHist_XimInvariantMassMeasured_acc = new TH1I("XimInvariantMassMeasured_acc", "Invariant Mass; #Lambda #pi^{-}", 300, 1.2, 1.5);
	dHist_XimInvariantMassKinFit_acc = new TH1I("XimInvariantMassKinFit_acc", "Invariant Mass; #Lambda #pi^{-}", 300, 1.2, 1.5);

	dHist_XimInvariantMassMeasured = new TH1I("XimInvariantMassMeasured", "Invariant Mass; #Lambda #pi^{-}", 300, 1.2, 1.5); 
	dHist_XimMassMeasured_Uniq = new TH1I("XimMassMeasured_Uniq", "Invariant Mass; #Lambda #pi^{-}", 300, 0.0, 3.0); 
	dHist_XimMassMeasured_UniqCut = new TH1I("XimMassMeasured_UniqCut", "Invariant Mass; #Lambda #pi^{-}", 300, 0.0, 3.0);
	dHist_XimInvariantMassKinFit = new TH1I("XimInvariantMassKinFit", "Invariant Mass; #Lambda #pi^{-}", 300, 1.2, 1.5);

	dHist_LambdaInvariantMassMeasured = new TH1I("LambdaInvariantMassMeasured", "Invariant Mass; p #pi^{-}", 300, 1.0, 1.3);
	dHist_LambdaInvariantMassKinFit = new TH1I("LambdaInvariantMassKinFit", "Invariant Mass; p #pi^{-}", 300, 1.0, 1.3);

	//proton kinematics
	dHist_ProtonP3VsThetaMeasured = new TH2I("ProtonP3VsThetaMeasured", "Proton Measured Kinematics; #Theta; 3-Momentum ", 500, 0, 180, 500, 0, 10);
	dHist_ProtonP3VsTheta = new TH2I("ProtonP3VsTheta", "Proton Kinematics; #Theta; 3-Momentum ", 500, 0, 180, 500, 0, 10);
	dHist_ProtonP3VsTheta_Uniq = new TH2I("ProtonP3VsThetaUniq", "Uniq Proton Kinematics; #Theta; 3-Momentum ", 500, 0, 180, 500, 0, 10);
	

	//
	dHist_XimP3VsThetaMeasured = new TH2I("XimP3VsThetaMeasured","Measured Xi-Kinematic ; #Theta; 3-Momentum", 500, 0, 180, 500, 0, 10);
	dHist_XimP3VsTheta = new TH2I("XimP3VsTheta","Xim Kinematic ; #Theta; 3-Momentum", 500, 0, 180, 500, 0, 10);
	dHist_KPlus1P3VsThetaMeasured = new TH2I("KPlus1P3VsThetaMeasured","KPlus1 Measured Kinematic ; #Theta; 3-Momentum", 500, -1., 1, 500, 0, 10);
	dHist_KPlus1P3VsTheta = new TH2I("KPlus1P3VsTheta","KPlus1 Kinematic ; #Theta; 3-Momentum", 500, -1., 1, 500, 0, 10);
	dHist_KPlus2P3VsThetaMeasured = new TH2I("KPlus2P3VsThetaMeasured","KPlus2 Measured Kinematic ; #Theta; 3-Momentum", 500, -1., 1, 500, 0, 10);
	dHist_KPlus2P3VsTheta = new TH2I("KPlus2P3VsTheta","KPlus2 Kinematic ; #Theta; 3-Momentum", 500, -1., 1, 500, 0, 10);

	//Path Length
	dHist_XiPath_preCL = new TH1I("XiPathLength_preCL", ";#Xi^{-} Path Length (cm)", 600, 0.0, 15.0);
	dHist_XiPath_postCL = new TH1I("XiPathLength_postCL", ";#Xi^{-} Path Length (cm)", 600, 0.0, 15.0);
	dHist_LambPath_preCL = new TH1I("LambPathLength_preCL", ";#Lambda Path Length (cm)", 150, 0.0, 150.0);
	dHist_LambPath_postCL = new TH1I("LambPathLength_postCL", ";#Lambda Path Length (cm)", 150, 0.0, 150.0);

	//Cross Section Plots
	dHist_XiIM_Egamma = new TH2I("XiIM_Egamma", " ; E_{#gamma#}; Xi^{-}_{IM}", 100, 6.4, 11.4, 250, 1.25, 1.5);
	dHist_XiIM_Egamma_wacc = new TH2I("XiIM_Egamma_wacc", " ; E_{#gamma#}; Xi^{-}_{IM}", 100, 6.4, 11.4, 250, 1.25, 1.5);

	//chisq
	dHist_testChiSq = new TH1I("textChiSq", " ; ChiSq; ", 150, 0, 150);

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
	/*
	dFlatTreeInterface->Create_Branch_Fundamental<Int_t>("flat_my_int"); //fundamental = char, int, float, double, etc.
	dFlatTreeInterface->Create_Branch_FundamentalArray<Int_t>("flat_my_int_array", "flat_my_int");
	dFlatTreeInterface->Create_Branch_NoSplitTObject<TLorentzVector>("flat_my_p4");
	dFlatTreeInterface->Create_Branch_ClonesArray<TLorentzVector>("flat_my_p4_array");
	*/

	/************************************* ADVANCED EXAMPLE: CHOOSE BRANCHES TO READ ************************************/

	//TO SAVE PROCESSING TIME
		//If you know you don't need all of the branches/data, but just a subset of it, you can speed things up
		//By default, for each event, the data is retrieved for all branches
		//If you know you only need data for some branches, you can skip grabbing data from the branches you don't need
		//Do this by doing something similar to the commented code below

	//dTreeInterface->Clear_GetEntryBranches(); //now get none
	//dTreeInterface->Register_GetEntryBranch("Proton__P4"); //manually set the branches you want
}

Bool_t DSelector_kpkpxim_2017::Process(Long64_t locEntry)
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

	set<map<Particle_t, set<Int_t> > > locUsedSoFar_gXi;
	set<map<Particle_t, set<Int_t> > > locUsedSoFar_LambdaInvariantMass;
	set<map<Particle_t, set<Int_t> > > locUsedSoFar_ProtonKinematics;

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

	//Loop over combos
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

                // TLorentzVector locBeamX4_Measured = dComboBeamWrapper->Get_X4_Measured();
                // Double_t locBunchPeriod = dAnalysisUtilities.Get_BeamBunchPeriod(Get_RunNumber());
                // Double_t locDeltaT_RF = dAnalysisUtilities.Get_DeltaT_RF(Get_RunNumber(), locBeamX4_Measured, dComboWrapper);
                // Int_t locRelBeamBucket = dAnalysisUtilities.Get_RelativeBeamBucket(Get_RunNumber(), locBeamX4_Measured, dComboWrapper); // 0 for in-time events, non-zero integer for out-of-time photons
                // Int_t locNumOutOfTimeBunchesInTree = 2; //YOU need to specify this number
                //         //Number of out-of-time beam bunches in tree (on a single side, so that total number out-of-time bunches accepted is 2 times this number for left + right bunches) 

                // Bool_t locSkipNearestOutOfTimeBunch = true; // True: skip events from nearest out-of-time bunch on either side (recommended).
                // Int_t locNumOutOfTimeBunchesToUse = locSkipNearestOutOfTimeBunch ? locNumOutOfTimeBunchesInTree-1:locNumOutOfTimeBunchesInTree;
                // Double_t locAccidentalScalingFactor = dAnalysisUtilities.Get_AccidentalScalingFactor(Get_RunNumber(), locBeamP4.E()); // Ideal value would be 1, but deviations observed: need added factor.
                // Double_t locAccidentalScalingFactorError = dAnalysisUtilities.Get_AccidentalScalingFactorError(Get_RunNumber(), locBeamP4.E()); // Ideal value would be 1, but deviations observed, need added factor.
                // Double_t locHistAccidWeightFactor = locRelBeamBucket==0 ? 1 : -locAccidentalScalingFactor/(2*locNumOutOfTimeBunchesToUse) ; // Weight by 1 for in-time events, ScalingFactor*(1/NBunches) for out-of-time
                // if(locSkipNearestOutOfTimeBunch && abs(locRelBeamBucket)==1) continue; // Skip nearest out-of-time bunch: tails of in-time distribution also leak in
		
		/********************************************* COMBINE FOUR-MOMENTUM ********************************************/

		// DO YOUR STUFF HERE

		// Accidental Scale Factor

		//double scaling_factor = dAnalysisUtilities.Get_AccidentalScalingFactor(locRunNumber, locBeamP4.E());
		//double scaling_factor_err = dAnalysisUtilities.Get_AccidentalScalingFactorError(locRunNumber, locBeamP4.E());
		
		// Combine 4-vectors
		TLorentzVector locMissingP4_Measured = locBeamP4_Measured + dTargetP4;
		TLorentzVector locDecayingLambdaP4_Measured = locProtonP4_Measured + locPiMinus2P4_Measured;
	  		
		TLorentzVector locXiMinusP4 = locDecayingLambdaP4 + locPiMinus1P4;
		TLorentzVector locXiMinusP4_Measured = locDecayingLambdaP4_Measured + locPiMinus1P4_Measured;  
		  
		locMissingP4_Measured -= locKPlus1P4_Measured + locKPlus2P4_Measured + locPiMinus1P4_Measured + locPiMinus2P4_Measured + locProtonP4_Measured;

		//Path Length
		//TLorentzVector locDecayingLambdaP4 = locPiMinus2P4 +	locProtonP4; //Decaying Lambda for M18 is manually calculated
		TLorentzVector locDecayingLambdaX4 = dDecayingLambdaWrapper->Get_X4(); //Doesn't exist for M18
		TLorentzVector locDecayingXiX4 = dTreeInterface->Get_TObject<TLorentzVector>("DecayingXiMinus__X4",loc_i);
		TLorentzVector locDecayingLambX4 = dTreeInterface->Get_TObject<TLorentzVector>("DecayingLambda__X4",loc_i);
		TLorentzVector locProdSpacetimeVertex = dComboBeamWrapper->Get_X4();//Get production vertex
		TLorentzVector locDeltaSpacetimeXi = locProdSpacetimeVertex - locDecayingXiX4;//vertex difference
		TLorentzVector locDeltaSpacetimeLamb = locDecayingXiX4 - locDecayingLambX4;//vertex difference
		double locPathLengthXi = locDeltaSpacetimeXi.Vect().Mag();//pathlength is just the magnitude
		double locPathLengthLamb = locDeltaSpacetimeLamb.Vect().Mag();//pathlength is just the magnitude
		float locPathLengthSigmaXi = Get_Fundamental<Float_t>("DecayingXiMinus__PathLengthSigma", loc_i);
		float locPathLengthSigmaLamb = Get_Fundamental<Float_t>("DecayingLambda__PathLengthSigma", loc_i);
		double locPathLengthSignificanceXi = locPathLengthXi/locPathLengthSigmaXi;
		double locPathLengthSignificanceLamb = locPathLengthLamb/locPathLengthSigmaLamb;
		

		/******************************************** GET FOUR-POSITION **************************************************/

		//TLorentzVector loc_beamX4 = dComboBeamWrapper-> GetX4();


		/**************************************** EXAMPLE: FILL CUSTOM OUTPUT BRANCHES **************************************/

		/*
		TLorentzVector locMyComboP4(8.0, 7.0, 6.0, 5.0);
		//for arrays below: 2nd argument is value, 3rd is array index
		//NOTE: By filling here, AFTER the cuts above, some indices won't be updated (and will be whatever they were from the last event)
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
			locUsedSoFar_BeamEnergy.insert(locBeamID);
		}

		/************************************ EXAMPLE: HISTOGRAM MISSING MASS SQUARED ************************************/

		// uniqueness tracking 
		map<Particle_t, set<Int_t> > locUsedThisCombo_MissingMass;
		locUsedThisCombo_MissingMass[Unknown].insert(locBeamID); //beam
		locUsedThisCombo_MissingMass[KPlus].insert(locKPlus1TrackID);
		locUsedThisCombo_MissingMass[KPlus].insert(locKPlus2TrackID);
		locUsedThisCombo_MissingMass[PiMinus].insert(locPiMinus1TrackID);
		locUsedThisCombo_MissingMass[PiMinus].insert(locPiMinus2TrackID);
		locUsedThisCombo_MissingMass[Proton].insert(locProtonTrackID);

		map<Particle_t, set<Int_t> > locUsedThisCombo_gXi;
		locUsedThisCombo_gXi[Unknown].insert(locBeamID); //beam
		locUsedThisCombo_gXi[PiMinus].insert(locPiMinus1TrackID);
		locUsedThisCombo_gXi[PiMinus].insert(locPiMinus2TrackID);
		locUsedThisCombo_gXi[Proton].insert(locProtonTrackID);

		//Pre-cut histograms
		if(locUsedSoFar_gXi.find(locUsedThisCombo_gXi) == locUsedSoFar_gXi.end())
		{
			  //unique missing mass combo: histogram it, and register this combo of particles		  
		          dHist_LambPath_preCL->Fill(locPathLengthLamb);
			  dHist_XiPath_preCL->Fill(locPathLengthXi);
			  dHist_XimMassMeasured_Uniq->Fill(locXiMinusP4_Measured.M());
		          //locUsedSoFar_gXi.insert(locUsedThisCombo_gXi);
		}
		
		//Missing Mass Squared
		double locMissingMassSquared = locMissingP4_Measured.M2();
		Float_t locChiSq = dComboWrapper->Get_ChiSq_KinFit();
		dHist_testChiSq->Fill(locChiSq);

		//Uniqueness tracking: Build the map of particles used for the missing mass
		//For beam: Don't want to group with final-state photons. Instead use "Unknown" PID (not ideal, but it's easy).
		

		//E.g. Cut
		if(locChiSq > 35)
		{
		        dComboWrapper->Set_IsComboCut(true);
			//locUsedSoFar_gXi.insert(locUsedThisCombo_gXi);
			continue;
		}
	     
		//E.g. Cut
		if((locMissingMassSquared < -0.04) || (locMissingMassSquared > 0.04))
		{
			dComboWrapper->Set_IsComboCut(true);
			//locUsedSoFar_gXi.insert(locUsedThisCombo_gXi);
			continue;
		}
		
		if(locBeamP4.E() < 7.9)
		  {
		    dComboWrapper->Set_IsComboCut(true);
		    continue;
		  }

		//accidental subtracted w/o uniqueness tracking
		//dHist_XimInvariantMassMeasured_acc->Fill(locXiMinusP4_Measured.M(), locHistAccidWeightFactor);
		//dHist_XimInvariantMassKinFit_acc->Fill(locXiMinusP4.M(), locHistAccidWeightFactor);
		

	

		//compare to what's been used so far
		if(locUsedSoFar_MissingMass.find(locUsedThisCombo_MissingMass) == locUsedSoFar_MissingMass.end())
		{
			//unique missing mass combo: histogram it, and register this combo of particles
			dHist_MissingMassSquared->Fill(locMissingMassSquared);
			
			locUsedSoFar_MissingMass.insert(locUsedThisCombo_MissingMass);
		}

		//My Stuff

		//compare to what's been used so far
		if(locUsedSoFar_gXi.find(locUsedThisCombo_gXi) == locUsedSoFar_gXi.end())
		{
			  //unique missing mass combo: histogram it, and register this combo of particles
		          dHist_XimInvariantMassKinFit->Fill(locXiMinusP4.M());
		          dHist_XimInvariantMassMeasured->Fill(locXiMinusP4_Measured.M());
			  dHist_XimMassMeasured_UniqCut->Fill(locXiMinusP4_Measured.M());
			  //dHist_XimInvariantMassKinFit_acc->Fill(locXiMinusP4.M(), locHistAccidWeightFactor);
			  //dHist_XimInvariantMassMeasured_acc->Fill(locXiMinusP4_Measured.M(), locHistAccidWeightFactor);

			  dHist_LambdaInvariantMassKinFit->Fill(locDecayingLambdaP4.M());		           
		          dHist_LambdaInvariantMassMeasured->Fill(locDecayingLambdaP4_Measured.M());
			  //dHist_XimP3VsThetaMeasured->Fill(TMath::Cos(locXiMinusP4_Measured.Theta()),locXiMinusP4_Measured.P());

			  dHist_LambPath_postCL->Fill(locPathLengthLamb);
			  dHist_XiPath_postCL->Fill(locPathLengthXi);

			  dHist_XiIM_Egamma->Fill( locBeamP4.E(), locXiMinusP4.M() );
			  
		          locUsedSoFar_gXi.insert(locUsedThisCombo_gXi);
		}

		
		dHist_XimP3VsThetaMeasured->Fill((180/TMath::Pi())*(locXiMinusP4_Measured.Theta()),locXiMinusP4_Measured.P());
		dHist_XimP3VsTheta->Fill((180/TMath::Pi())*(locXiMinusP4.Theta()),locXiMinusP4.P());

		dHist_ProtonP3VsThetaMeasured->Fill((180/TMath::Pi())*(locProtonP4_Measured.Theta()),locProtonP4_Measured.P());
		dHist_ProtonP3VsTheta->Fill((180/TMath::Pi())*(locProtonP4.Theta()),locProtonP4.P());	        
		
		dHist_KPlus1P3VsThetaMeasured->Fill(TMath::Cos(locKPlus1P4_Measured.Theta()),locKPlus1P4_Measured.P());
		dHist_KPlus1P3VsTheta->Fill(TMath::Cos(locKPlus1P4.Theta()),locKPlus1P4.P());

		dHist_KPlus2P3VsThetaMeasured->Fill(TMath::Cos(locKPlus2P4_Measured.Theta()),locKPlus2P4_Measured.P());
		dHist_KPlus2P3VsTheta->Fill(TMath::Cos(locKPlus2P4.Theta()),locKPlus2P4.P());

		map<Particle_t, set<Int_t> > locUsedThisCombo_ProtonKinematics;
		locUsedThisCombo_ProtonKinematics[Proton].insert(locProtonTrackID);

		if(locUsedSoFar_ProtonKinematics.find(locUsedThisCombo_ProtonKinematics) == locUsedSoFar_ProtonKinematics.end())
		{
		  dHist_ProtonP3VsTheta_Uniq->Fill((180/TMath::Pi())*locProtonP4.Theta(),locProtonP4.P());
		  
		  locUsedSoFar_ProtonKinematics.insert(locUsedThisCombo_ProtonKinematics);
		}

		/******************************************** EXECUTE ANALYSIS ACTIONS *******************************************/

		// Loop through the analysis actions, executing them in order for the active particle combo
		//dAnalyzeCutActions->Perform_ActionWeight(locHistAccidWeightFactor);
		
		// Loop through the analysis actions, executing them in order for the active particle combo
		
		//dAnalyzeCutActions->Perform_Action(); // Must be executed before Execute_Actions()
		if(!Execute_Actions()) //if the active combo fails a cut, IsComboCutFlag automatically set
		  continue;
		
		//if you manually execute any actions, and it fails a cut, be sure to call:
			//dComboWrapper->Set_IsComboCut(true);

		/*****************************************************************************************************************/

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

		//FILL FLAT TREE
		//Fill_FlatTree(); //for the active combo
	} // end of combo loop

	//FILL HISTOGRAMS: Num combos / events surviving actions
	Fill_NumCombosSurvivedHists();

	/******************************************* LOOP OVER THROWN DATA (OPTIONAL) ***************************************/
/*
	//Thrown beam: just use directly
	if(dThrownBeam != NULL)
		double locEnergy = dThrownBeam->Get_P4().E();

	//Loop over throwns
	for(UInt_t loc_i = 0; loc_i < Get_NumThrown(); ++loc_i)
	{
		//Set branch array indices corresponding to this particle
		dThrownWrapper->Set_ArrayIndex(loc_i);

		//Do stuff with the wrapper here ...
	}
*/
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

void DSelector_kpkpxim_2017::Finalize(void)
{
	//Save anything to output here that you do not want to be in the default DSelector output ROOT file.

	//Otherwise, don't do anything else (especially if you are using PROOF).
		//If you are using PROOF, this function is called on each thread,
		//so anything you do will not have the combined information from the various threads.
		//Besides, it is best-practice to do post-processing (e.g. fitting) separately, in case there is a problem.

	//DO YOUR STUFF HERE

	//CALL THIS LAST
	DSelector::Finalize(); //Saves results to the output file
}
