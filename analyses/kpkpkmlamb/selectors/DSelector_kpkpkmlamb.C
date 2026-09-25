#include "DSelector_kpkpkmlamb.h"

void DSelector_kpkpkmlamb::Init(TTree *locTree)
{
	// USERS: IN THIS FUNCTION, ONLY MODIFY SECTIONS WITH A "USER" OR "EXAMPLE" LABEL. LEAVE THE REST ALONE.

	// The Init() function is called when the selector needs to initialize a new tree or chain.
	// Typically here the branch addresses and branch pointers of the tree will be set.
	// Init() will be called many times when running on PROOF (once per file to be processed).

	//USERS: SET OUTPUT FILE NAME //can be overriden by user in PROOF
	dOutputFileName = "kpkpkmlamb.root"; //"" for none
	dOutputTreeFileName = ""; //"" for none
	dFlatTreeFileName = "flatTree_kpkpkmlamb.root"; //output flat tree (one combo per tree entry), "" for none
	dFlatTreeName = "flatTree_kpkpkmlamb"; //if blank, default name will be chosen
	//dSaveDefaultFlatBranches = true; // False: don't save default branches, reduce disk footprint.
	//dSaveTLorentzVectorsAsFundamentaFlatTree = false; // Default (or false): save particles as TLorentzVector objects. True: save as four doubles instead.

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
	// // For histogramming the phi mass in phi -> K+ K-
	// // Be sure to change this and dAnalyzeCutActions to match reaction
	std::deque<Particle_t> MyXiStar;
	MyXiStar.push_back(Lambda); MyXiStar.push_back(KMinus);

	//ANALYSIS ACTIONS: //Executed in order if added to dAnalysisActions
	//false/true below: use measured/kinfit data

	//PID
	dAnalysisActions.push_back(new DHistogramAction_ParticleID(dComboWrapper, false));
	//below: value: +/- N ns, Unknown: All PIDs, SYS_NULL: all timing systems
	//dAnalysisActions.push_back(new DCutAction_PIDDeltaT(dComboWrapper, false, 0.5, KPlus, SYS_BCAL));

	//PIDFOM (for charged tracks)
	dAnalysisActions.push_back(new DHistogramAction_PIDFOM(dComboWrapper));
	//dAnalysisActions.push_back(new DCutAction_PIDFOM(dComboWrapper, KPlus, 0.1));
	//dAnalysisActions.push_back(new DCutAction_EachPIDFOM(dComboWrapper, 0.1));

	//MASSES
	dAnalysisActions.push_back(new DHistogramAction_InvariantMass(dComboWrapper, false, Lambda, 1000, 1.0, 1.2, "Lambda"));
	//dAnalysisActions.push_back(new DHistogramAction_MissingMassSquared(dComboWrapper, false, 1000, -0.1, 0.1));

	//KINFIT RESULTS
	dAnalysisActions.push_back(new DHistogramAction_KinFitResults(dComboWrapper));

	//CUT MISSING MASS
	dAnalysisActions.push_back(new DCutAction_MissingMassSquared(dComboWrapper, false, -0.03, 0.02));
    dAnalysisActions.push_back(new DCutAction_KinFitChiSq(dComboWrapper, 6, "ChiSqNdfCut"));
	//CUT ON SHOWER QUALITY
	//dAnalysisActions.push_back(new DCutAction_ShowerQuality(dComboWrapper, SYS_FCAL, 0.5));

	//BEAM ENERGY
	dAnalysisActions.push_back(new DHistogramAction_BeamEnergy(dComboWrapper, false));
	//dAnalysisActions.push_back(new DCutAction_BeamEnergy(dComboWrapper, false, 8.2, 8.8));  // Coherent peak for runs in the range 30000-59999

	//KINEMATICS
	dAnalysisActions.push_back(new DHistogramAction_ParticleComboKinematics(dComboWrapper, false));

	// ANALYZE CUT ACTIONS
	// // Change MyXiStar to match reaction
	dAnalyzeCutActions = new DHistogramAction_AnalyzeCutActions( dAnalysisActions, dComboWrapper, false, 0, MyXiStar, 1000, 0.9, 2.4, "CutActionEffect" );

	//INITIALIZE ACTIONS
	//If you create any actions that you want to run manually (i.e. don't add to dAnalysisActions), be sure to initialize them here as well
	Initialize_Actions();
	dAnalyzeCutActions->Initialize(); // manual action, must call Initialize()

	/******************************** EXAMPLE USER INITIALIZATION: STAND-ALONE HISTOGRAMS *******************************/

	//EXAMPLE MANUAL HISTOGRAMS:
	dHist_MissingMassSquared = new TH1I("MissingMassSquared", ";Missing Mass Squared (GeV/c^{2})^{2}", 600, -0.06, 0.06);
	dHist_BeamEnergy = new TH1I("BeamEnergy", ";Beam Energy (GeV)", 300, 6.0, 12.0);

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

    // All measurements are kinfit unless specified by _meas
    // 4-vectors
    dFlatTreeInterface->Create_Branch_NoSplitTObject<TLorentzVector>("beam_p4");
    dFlatTreeInterface->Create_Branch_NoSplitTObject<TLorentzVector>("ystar_p4");
    dFlatTreeInterface->Create_Branch_NoSplitTObject<TLorentzVector>("kplow_p4");
    dFlatTreeInterface->Create_Branch_NoSplitTObject<TLorentzVector>("kplow2d_p4");
    dFlatTreeInterface->Create_Branch_NoSplitTObject<TLorentzVector>("kplow_p4_ystar_hf");
    dFlatTreeInterface->Create_Branch_NoSplitTObject<TLorentzVector>("kphigh_p4");
    dFlatTreeInterface->Create_Branch_NoSplitTObject<TLorentzVector>("kphigh2d_p4");
    dFlatTreeInterface->Create_Branch_NoSplitTObject<TLorentzVector>("ximstar_p4");    
    dFlatTreeInterface->Create_Branch_NoSplitTObject<TLorentzVector>("lambda_p4");
    dFlatTreeInterface->Create_Branch_NoSplitTObject<TLorentzVector>("kp1_km_p4");
    dFlatTreeInterface->Create_Branch_NoSplitTObject<TLorentzVector>("kp1_pim_p4");
    dFlatTreeInterface->Create_Branch_NoSplitTObject<TLorentzVector>("kp1_lamb_p4");
    dFlatTreeInterface->Create_Branch_NoSplitTObject<TLorentzVector>("kp2_km_p4");
    dFlatTreeInterface->Create_Branch_NoSplitTObject<TLorentzVector>("kp2_pim_p4");
    dFlatTreeInterface->Create_Branch_NoSplitTObject<TLorentzVector>("kp2_lamb_p4");
    dFlatTreeInterface->Create_Branch_NoSplitTObject<TLorentzVector>("kphigh_km_p4");
    dFlatTreeInterface->Create_Branch_NoSplitTObject<TLorentzVector>("kphigh_pim_p4");
    dFlatTreeInterface->Create_Branch_NoSplitTObject<TLorentzVector>("kphigh_lamb_p4");
    dFlatTreeInterface->Create_Branch_NoSplitTObject<TLorentzVector>("kplow_km_p4");
    dFlatTreeInterface->Create_Branch_NoSplitTObject<TLorentzVector>("kplow_pim_p4");
    dFlatTreeInterface->Create_Branch_NoSplitTObject<TLorentzVector>("kplow_lamb_p4");
    dFlatTreeInterface->Create_Branch_NoSplitTObject<TLorentzVector>("km_p_p4");    
    //get vertex position
    dFlatTreeInterface->Create_Branch_NoSplitTObject<TLorentzVector>("vertex0_x4");
    dFlatTreeInterface->Create_Branch_NoSplitTObject<TLorentzVector>("vertex1_x4");
    
    // beam stuff
 	dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("beam_E");
    dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("beam_rfbunches");
	dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("beam_vertexX");
	dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("beam_vertexY");
	dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("beam_vertexZ");
	// kp1
    dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("kp1_P3");
    dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("kp1_P3_Truth");
    dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("kphigh_P3");
    dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("kphigh_Theta");
    dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("kphigh_CosTheta");
	dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("kphigh_Phi");
	// kp2
    dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("kp2_P3");
	dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("kp2_P3_Truth");
	dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("kplow_P3");
    dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("kplow_Theta");
    dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("kp2_YstarRest_CosTheta");
	dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("kp2_YstarRest_Phi");
    dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("kplow_costheta_hf");
	dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("kplow_phi_hf");
    // ystar
	dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("ystar_M");
	dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("ystar_P3");
	dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("ystar_Theta");
    // ximinus
	dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("ximstar_M");
    dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("xim_P3");
    dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("xim_costheta_hf");
    // lambda
    dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("decaylamb_M");
    dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("lambda_P3");
	dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("lambda_vertexX");
	dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("lambda_vertexY");
	dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("lambda_vertexZ");
	dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("lambda_pathlen");
	dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("lambda_pathlensig");
	dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("lambda_lifetime");
	dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("lambda_lifetime_restframe");
    // pions
	dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("km_P3");
    dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("km_costheta_hf");
	dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("pim_P3");
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
    dFlatTreeInterface->Create_Branch_Fundamental<Int_t>("rf_intime_weight");
    
	/************************** EXAMPLE USER INITIALIZATION: CUSTOM OUTPUT BRANCHES - FLAT TREE *************************/

	// RECOMMENDED: CREATE ACCIDENTAL WEIGHT BRANCH
	// dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("accidweight");

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

	/************************************** DETERMINE IF ANALYZING SIMULATED DATA *************************************/

	dIsMC = (dTreeInterface->Get_Branch("MCWeight") != NULL);

}

Bool_t DSelector_kpkpkmlamb::Process(Long64_t locEntry)
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
	if(dThrownBeam != NULL)
		Double_t locEnergy = dThrownBeam->Get_P4().E();

	TLorentzVector locKPlus1P4_Truth;
	TLorentzVector locKPlus2P4_Truth;
    TLorentzVector locKMinusP4_Truth;
    TLorentzVector locPiMinusP4_Truth;
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
		cout << "\t Parent Particle Index: " << locParentIndex << endl;

		if(locPID == 11) {
		  if(loc_j == 0) {locKPlus1P4_Truth = locThrownP4; }
		  if(loc_j == 1) {locKPlus2P4_Truth = locThrownP4; }
		}
        if(locPID == 12) locKMinusP4_Truth = locThrownP4;
		if(locPID == 9) locPiMinusP4_Truth = locThrownP4;
        //if(locParentIndex == 5)
        if(locPID == 14) locProtonP4_Truth = locThrownP4;		  
        if(locPID == 18 ) locLambdaP4_Truth = locThrownP4;
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
    Int_t locNumCombos = Get_NumCombos();
	Int_t locNumComboSurvivedCut = 0;
    Double_t locBestChiSqNdf = 0;
    Double_t locBestConfidenceLvl = 0;
    UInt_t locBestChiSqNdfComboNum = 999;
    Double_t locBestChiSqNdf_1 = 0;
    UInt_t locBestChiSqNdfComboNum_1 = 999;
    ULong64_t locEventNum = Get_EventNumber();

    cout << "EVENT NUMBER: " << locEventNum <<"****************************************************" << endl;
	//Loop over combos for best combo
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
		Int_t locKMinusTrackID = dKMinusWrapper->Get_TrackID();

		//Step 1
		Int_t locPiMinusTrackID = dPiMinusWrapper->Get_TrackID();
		Int_t locProtonTrackID = dProtonWrapper->Get_TrackID();

		/*********************************************** GET FOUR-MOMENTUM **********************************************/

		// Get P4's: //is kinfit if kinfit performed, else is measured
		//dTargetP4 is target p4
		//Step 0
		TLorentzVector locBeamP4 = dComboBeamWrapper->Get_P4();
		TLorentzVector locKPlus1P4 = dKPlus1Wrapper->Get_P4();
		TLorentzVector locKPlus2P4 = dKPlus2Wrapper->Get_P4();
		TLorentzVector locKMinusP4 = dKMinusWrapper->Get_P4();
		//Step 1
		TLorentzVector locPiMinusP4 = dPiMinusWrapper->Get_P4();
		TLorentzVector locProtonP4 = dProtonWrapper->Get_P4();

		// Get Measured P4's:
		//Step 0
		TLorentzVector locBeamP4_Measured = dComboBeamWrapper->Get_P4_Measured();
		TLorentzVector locKPlus1P4_Measured = dKPlus1Wrapper->Get_P4_Measured();
		TLorentzVector locKPlus2P4_Measured = dKPlus2Wrapper->Get_P4_Measured();
		TLorentzVector locKMinusP4_Measured = dKMinusWrapper->Get_P4_Measured();
		//Step 1
		TLorentzVector locPiMinusP4_Measured = dPiMinusWrapper->Get_P4_Measured();
		TLorentzVector locProtonP4_Measured = dProtonWrapper->Get_P4_Measured();

		/********************************************* GET COMBO RF TIMING INFO *****************************************/

		TLorentzVector locBeamX4_Measured = dComboBeamWrapper->Get_X4_Measured();
		Double_t locBunchPeriod = dAnalysisUtilities.Get_BeamBunchPeriod(Get_RunNumber());
		Double_t locDeltaT_RF = dAnalysisUtilities.Get_DeltaT_RF(Get_RunNumber(), locBeamX4_Measured, dComboWrapper);
		Int_t locRelBeamBucket = dAnalysisUtilities.Get_RelativeBeamBucket(Get_RunNumber(), locBeamX4_Measured, dComboWrapper); // 0 for in-time events, non-zero integer for out-of-time photons
		Int_t locNumOutOfTimeBunchesInTree = 4; //YOU need to specify this number
        //Number of out-of-time beam bunches in tree (on a single side, so that total number out-of-time bunches accepted is 2 times this number for left + right bunches) 

		Bool_t locSkipNearestOutOfTimeBunch = true; // True: skip events from nearest out-of-time bunch on either side (recommended).
		Int_t locNumOutOfTimeBunchesToUse = locSkipNearestOutOfTimeBunch ? locNumOutOfTimeBunchesInTree-1:locNumOutOfTimeBunchesInTree; 
		Double_t locAccidentalScalingFactor = dAnalysisUtilities.Get_AccidentalScalingFactor(Get_RunNumber(), locBeamP4.E(), dIsMC); // Ideal value would be 1, but deviations require added factor, which is different for data and MC.
		Double_t locAccidentalScalingFactorError = dAnalysisUtilities.Get_AccidentalScalingFactorError(Get_RunNumber(), locBeamP4.E()); // Ideal value would be 1, but deviations observed, need added factor.
		Double_t locHistAccidWeightFactor = locRelBeamBucket==0 ? 1 : -locAccidentalScalingFactor/(2*locNumOutOfTimeBunchesToUse) ; // Weight by 1 for in-time events, ScalingFactor*(1/NBunches) for out-of-time
		if(locSkipNearestOutOfTimeBunch && abs(locRelBeamBucket)==1) { // Skip nearest out-of-time bunch: tails of in-time distribution also leak in
			dComboWrapper->Set_IsComboCut(true); 
			continue; 
		} 

		/********************************************* COMBINE FOUR-MOMENTUM ********************************************/

		// DO YOUR STUFF HERE

		// Combine 4-vectors
		TLorentzVector locMissingP4_Measured = locBeamP4_Measured + dTargetP4;
		locMissingP4_Measured -= locKPlus1P4_Measured + locKPlus2P4_Measured + locKMinusP4_Measured + locPiMinusP4_Measured + locProtonP4_Measured;

        TLorentzVector locLambdaP4 = locPiMinusP4 + locProtonP4;
        TLorentzVector locXiStarP4 = locLambdaP4 + locKMinusP4;
        //TLorentzVector locKPlus1KMinusP4 = locKMinusP4 + locKPlus1P4;
        //TLorentzVector locKPlusFastKMinusP4 = locKPlusFastP4 + locKPlus1P4;

        /******************************************** EXECUTE ANALYSIS ACTIONS *******************************************/

		// Loop through the analysis actions, executing them in order for the active particle combo
		dAnalyzeCutActions->Perform_Action(); // Must be executed before Execute_Actions()
		if(!Execute_Actions()) //if the active combo fails a cut, IsComboCutFlag automatically set
			continue;

		//if you manually execute any actions, and it fails a cut, be sure to call:
			//dComboWrapper->Set_IsComboCut(true);

		/**************************************** EXAMPLE: FILL CUSTOM OUTPUT BRANCHES **************************************/

		/*
		TLorentzVector locMyComboP4(8.0, 7.0, 6.0, 5.0);
		//for arrays below: 2nd argument is value, 3rd is array index
		//NOTE: By filling here, AFTER the cuts above, some indices won't be updated (and will be whatever they were from the last event)
			//So, when you draw the branch, be sure to cut on "IsComboCut" to avoid these.
		dTreeInterface->Fill_Fundamental<Float_t>("my_combo_array", -2*loc_i, loc_i);
		dTreeInterface->Fill_TObject<TLorentzVector>("my_p4_array", locMyComboP4, loc_i);
		*/

		/**************************************** EXAMPLE: HISTOGRAM BEAM ENERGY *****************************************/

		//Histogram beam energy (if haven't already)
		if(locUsedSoFar_BeamEnergy.find(locBeamID) == locUsedSoFar_BeamEnergy.end())
		{
			dHist_BeamEnergy->Fill(locBeamP4.E()); // Fills in-time and out-of-time beam photon combos
			//dHist_BeamEnergy->Fill(locBeamP4.E(),locHistAccidWeightFactor); // Alternate version with accidental subtraction

			locUsedSoFar_BeamEnergy.insert(locBeamID);
		}

		/************************************ EXAMPLE: HISTOGRAM MISSING MASS SQUARED ************************************/

		//Missing Mass Squared
		double locMissingMassSquared = locMissingP4_Measured.M2();

		//Uniqueness tracking: Build the map of particles used for the missing mass
			//For beam: Don't want to group with final-state photons. Instead use "Unknown" PID (not ideal, but it's easy).
		map<Particle_t, set<Int_t> > locUsedThisCombo_MissingMass;
		locUsedThisCombo_MissingMass[Unknown].insert(locBeamID); //beam
		locUsedThisCombo_MissingMass[KPlus].insert(locKPlus1TrackID);
		locUsedThisCombo_MissingMass[KPlus].insert(locKPlus2TrackID);
		locUsedThisCombo_MissingMass[KMinus].insert(locKMinusTrackID);
		locUsedThisCombo_MissingMass[PiMinus].insert(locPiMinusTrackID);
		locUsedThisCombo_MissingMass[Proton].insert(locProtonTrackID);

		//compare to what's been used so far
		if(locUsedSoFar_MissingMass.find(locUsedThisCombo_MissingMass) == locUsedSoFar_MissingMass.end())
		{
			//unique missing mass combo: histogram it, and register this combo of particles
			dHist_MissingMassSquared->Fill(locMissingMassSquared); // Fills in-time and out-of-time beam photon combos
			//dHist_MissingMassSquared->Fill(locMissingMassSquared,locHistAccidWeightFactor); // Alternate version with accidental subtraction

			locUsedSoFar_MissingMass.insert(locUsedThisCombo_MissingMass);
		}

		//ChiSqNDF
		Double_t locChiSqNdf = dComboWrapper->Get_ChiSq_KinFit("") / dComboWrapper->Get_NDF_KinFit("");
        Double_t locConfidenceLvl = dComboWrapper->Get_ConfidenceLevel_KinFit("") ;
        
        // Pre-cut isnan chisqndf cut
        if( std::isnan(locChiSqNdf) )
          {
            dComboWrapper->Set_IsComboCut(true);
            continue;
          }

        // skip all out of time beam bunches for best combo
        if(abs(locRelBeamBucket)>0)
          { 
            dComboWrapper->Set_IsComboCut(true);
            continue;
          }

        //Print PID
        cout << left << "BeamID " << " KPlus1ID " << " KPlus2ID " << " KMinusID " << " PiMinusID " << " ProtonID" << endl;
        cout << left << locBeamID << "\t" << locKPlus1TrackID << setw(9) << locKPlus2TrackID << setw(10) << locKMinusTrackID  << setw(12) << locPiMinusTrackID << setw(12) << locProtonTrackID << endl;

        locNumComboSurvivedCut += 1;
        if(locNumComboSurvivedCut==1)//initiate first survived combo
          {
            locBestChiSqNdf = locChiSqNdf, locBestChiSqNdf_1 = locChiSqNdf; 
            locBestChiSqNdfComboNum = loc_i, locBestChiSqNdfComboNum_1 = loc_i; 
          }
        else //get best combo per combo
          {
            if(locBestChiSqNdf > locChiSqNdf)
            //if(locBestConfidenceLvl < locConfidenceLvl)//try with confidence level
              locBestChiSqNdf = locChiSqNdf, locBestChiSqNdfComboNum = loc_i;
                
            if(abs(1.0-locBestChiSqNdf) > abs(1.0-locChiSqNdf))
              locBestChiSqNdf_1 = locChiSqNdf, locBestChiSqNdfComboNum_1 = loc_i;
          }
        
		/****************************************** FILL FLAT TREE (IF DESIRED) ******************************************/

		// RECOMMENDED: FILL ACCIDENTAL WEIGHT
		// dFlatTreeInterface->Fill_Fundamental<Double_t>("accidweight",locHistAccidWeightFactor);

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
    
    //Loop over combos to fill flat trees
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
		Int_t locKMinusTrackID = dKMinusWrapper->Get_TrackID();

		//Step 1
		Int_t locPiMinusTrackID = dPiMinusWrapper->Get_TrackID();
		Int_t locProtonTrackID = dProtonWrapper->Get_TrackID();
        
		/*********************************************** GET FOUR-MOMENTUM **********************************************/

		// Get P4's: //is kinfit if kinfit performed, else is measured
		//dTargetP4 is target p4
		//Step 0
		TLorentzVector locBeamP4 = dComboBeamWrapper->Get_P4();
		TLorentzVector locKPlus1P4 = dKPlus1Wrapper->Get_P4();
		TLorentzVector locKPlus2P4 = dKPlus2Wrapper->Get_P4();
		TLorentzVector locKMinusP4 = dKMinusWrapper->Get_P4();
		//Step 1
		TLorentzVector locPiMinusP4 = dPiMinusWrapper->Get_P4();
		TLorentzVector locProtonP4 = dProtonWrapper->Get_P4();

		// Get Measured P4's:
		//Step 0
		TLorentzVector locBeamP4_Measured = dComboBeamWrapper->Get_P4_Measured();
		TLorentzVector locKPlus1P4_Measured = dKPlus1Wrapper->Get_P4_Measured();
		TLorentzVector locKPlus2P4_Measured = dKPlus2Wrapper->Get_P4_Measured();
		TLorentzVector locKMinusP4_Measured = dKMinusWrapper->Get_P4_Measured();
		//Step 1
		TLorentzVector locPiMinusP4_Measured = dPiMinusWrapper->Get_P4_Measured();
		TLorentzVector locProtonP4_Measured = dProtonWrapper->Get_P4_Measured();

		/********************************************* GET COMBO RF TIMING INFO *****************************************/

		TLorentzVector locBeamX4_Measured = dComboBeamWrapper->Get_X4_Measured();
		Double_t locBunchPeriod = dAnalysisUtilities.Get_BeamBunchPeriod(Get_RunNumber());
		Double_t locDeltaT_RF = dAnalysisUtilities.Get_DeltaT_RF(Get_RunNumber(), locBeamX4_Measured, dComboWrapper);
		Int_t locRelBeamBucket = dAnalysisUtilities.Get_RelativeBeamBucket(Get_RunNumber(), locBeamX4_Measured, dComboWrapper); // 0 for in-time events, non-zero integer for out-of-time photons
		Int_t locNumOutOfTimeBunchesInTree = 4; //YOU need to specify this number
        //Number of out-of-time beam bunches in tree (on a single side, so that total number out-of-time bunches accepted is 2 times this number for left + right bunches) 

		Bool_t locSkipNearestOutOfTimeBunch = true; // True: skip events from nearest out-of-time bunch on either side (recommended).
		Int_t locNumOutOfTimeBunchesToUse = locSkipNearestOutOfTimeBunch ? locNumOutOfTimeBunchesInTree-1:locNumOutOfTimeBunchesInTree; 
		Double_t locAccidentalScalingFactor = dAnalysisUtilities.Get_AccidentalScalingFactor(Get_RunNumber(), locBeamP4.E(), dIsMC); // Ideal value would be 1, but deviations require added factor, which is different for data and MC.
		Double_t locAccidentalScalingFactorError = dAnalysisUtilities.Get_AccidentalScalingFactorError(Get_RunNumber(), locBeamP4.E()); // Ideal value would be 1, but deviations observed, need added factor.
		Double_t locHistAccidWeightFactor = locRelBeamBucket==0 ? 1 : -locAccidentalScalingFactor/(2*locNumOutOfTimeBunchesToUse) ; // Weight by 1 for in-time events, ScalingFactor*(1/NBunches) for out-of-time
		if(locSkipNearestOutOfTimeBunch && abs(locRelBeamBucket)==1) { // Skip nearest out-of-time bunch: tails of in-time distribution also leak in
			dComboWrapper->Set_IsComboCut(true); 
			continue; 
		} 
        Int_t locInTimeBunch = locRelBeamBucket==0 ? 1:0;
		/********************************************* COMBINE FOUR-MOMENTUM ********************************************/

		// DO YOUR STUFF HERE

		// Combine 4-vectors
		TLorentzVector locMissingP4_Measured = locBeamP4_Measured + dTargetP4;
		locMissingP4_Measured -= locKPlus1P4_Measured + locKPlus2P4_Measured + locKMinusP4_Measured + locPiMinusP4_Measured + locProtonP4_Measured;

        TLorentzVector locDecayingLambdaP4 = locPiMinusP4 + locProtonP4;
        TLorentzVector locDecayingLambdaP4_Measured = locPiMinusP4_Measured + locProtonP4_Measured;
        TLorentzVector locXimStarP4 = locDecayingLambdaP4 + locKMinusP4;
        TLorentzVector locXimStarP4_Measured = locDecayingLambdaP4_Measured + locKMinusP4_Measured;

        // Kaon Determincation
		TLorentzVector locKPlusSlowP4;
        TLorentzVector locKPlusFastP4;
		TLorentzVector locKPlusSlow2DCutP4;
        TLorentzVector locKPlusFast2DCutP4;
        TLorentzVector locKPlusSlowP4_Measured;
		TLorentzVector locKPlusFastP4_Measured;
		Int_t locKhighTrackID;
		Int_t locKlowTrackID;

        // Kaon determined from fcal bcal detectors
        TLorentzVector locKPlusSlowP4_BCal;
        TLorentzVector locKPlusHighP4_FCal;
        Double_t kp1_efcal = dKPlus1Wrapper->Get_Energy_FCAL();
        Double_t kp1_ebcal = dKPlus1Wrapper->Get_Energy_BCAL();
        Double_t kp2_efcal = dKPlus2Wrapper->Get_Energy_FCAL();
        Double_t kp2_ebcal = dKPlus2Wrapper->Get_Energy_BCAL();
        
		//Kaon Seperation with only momentum
		if(locKPlus1P4.P() > locKPlus2P4.P() )
		  {
		    locKPlusFastP4 = locKPlus1P4; 
		    locKPlusSlowP4 = locKPlus2P4;
		    locKhighTrackID = locKPlus1TrackID;
		    locKlowTrackID = locKPlus2TrackID;
		    locKPlusFastP4_Measured = locKPlus1P4_Measured; 
		    locKPlusSlowP4_Measured = locKPlus2P4_Measured;
		  }
		else
		  {
		    locKPlusFastP4 = locKPlus2P4; 
		    locKPlusSlowP4 = locKPlus1P4;
		    locKhighTrackID = locKPlus2TrackID;
		    locKlowTrackID = locKPlus1TrackID;
		    locKPlusFastP4_Measured = locKPlus2P4_Measured; 
		    locKPlusSlowP4_Measured = locKPlus1P4_Measured;
		  }

        //Kaon Seperation with 2d momentum and angle
        Double_t kaonDist = locKPlus1P4.P()*75 - locKPlus1P4.Theta()*180/TMath::Pi()*9;
        if(kaonDist > 0 )
          {
            locKPlusFast2DCutP4 = locKPlus1P4;
            locKPlusSlow2DCutP4 = locKPlus2P4;
          }
        else
          {
            locKPlusFast2DCutP4 = locKPlus2P4;
            locKPlusSlow2DCutP4 = locKPlus1P4;
          }
        cout << "Kaon selection critria (Kfast, KSlow): " << kaonDist << " (" << locKPlusFast2DCutP4.P() << ", " << locKPlusSlow2DCutP4.P() << ")" << endl;
        
		Double_t locT = (locBeamP4-locKPlusFastP4).M2();
        Double_t locT_K1 = (locBeamP4-locKPlus1P4).M2();

        TLorentzVector locKPlus1KMinusP4 = locKPlus1P4 + locKMinusP4;
        TLorentzVector locKPlus1PiMinusP4 = locKPlus1P4 + locPiMinusP4;
        TLorentzVector locKPlus1LambdaP4 = locKPlus1P4 + locDecayingLambdaP4;
        TLorentzVector locKPlus2KMinusP4 = locKPlus2P4 + locKMinusP4;
        TLorentzVector locKPlus2PiMinusP4 = locKPlus2P4 + locPiMinusP4;
        TLorentzVector locKPlus2LambdaP4 = locKPlus2P4 + locDecayingLambdaP4;
        TLorentzVector locKPlusFastKMinusP4 = locKPlusFastP4 + locKMinusP4;
        TLorentzVector locKPlusFastPiMinusP4 = locKPlusFastP4 + locPiMinusP4;
        TLorentzVector locKPlusFastLambdaP4 = locKPlusFastP4 + locDecayingLambdaP4;
        TLorentzVector locKPlusSlowKMinusP4 = locKPlusSlowP4 + locKMinusP4;
        TLorentzVector locKPlusSlowPiMinusP4 = locKPlusSlowP4 + locPiMinusP4;
        TLorentzVector locKPlusSlowLambdaP4 = locKPlusSlowP4 + locDecayingLambdaP4;
        TLorentzVector locKMinusProtonP4 = locKMinusP4 + locProtonP4;
        TLorentzVector locYstar_Kp2Xim_P4 = locKPlus2P4 + locXimStarP4;
        TLorentzVector locYstarP4 = locKPlusSlowP4 + locXimStarP4;
        TLorentzVector locYstarP4_Measured = locKPlusSlowP4_Measured + locXimStarP4_Measured;

        TLorentzVector locKPlusSlow_PiMinus_P4 = locKPlusSlowP4 + locPiMinusP4;
		
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
		TLorentzVector locKPlusFastP4_CM = locKPlusFastP4;
        TLorentzVector locYstarP4_CM = locYstarP4;
        //step 3
		TLorentzVector locXimStarP4_CM = locXimStarP4;
        TLorentzVector locKPlus2P4_CM = locKPlus2P4;
		TLorentzVector locKPlusSlowP4_CM = locKPlusSlowP4;
        TLorentzVector locKMinusP4_CM = locKMinusP4;
		//step 4
		TLorentzVector locDecayingLambdaP4_CM = locDecayingLambdaP4;
		//Boost 4vectors in Com frame
		locBeamP4_CM.Boost(-boostCoM);
        locYstarP4_CM.Boost(-boostCoM);
        locKPlus1P4_CM.Boost(-boostCoM);
		locKPlusFastP4_CM.Boost(-boostCoM);
        locKPlus2P4_CM.Boost(-boostCoM);
		locKPlusSlowP4_CM.Boost(-boostCoM);
        locXimStarP4_CM.Boost(-boostCoM);
        locKMinusP4_CM.Boost(-boostCoM);
		locDecayingLambdaP4_CM.Boost(-boostCoM);
		
		//Boost 4vector into rest frame of ystar from CoM
		TVector3 boostYstar_Rest = locYstarP4_CM.BoostVector();
		//Initialize 4vectors in ystar rest frame
		//step 1
		TLorentzVector locBeamP4_YstarRest = locBeamP4_CM;
		//step 2
		TLorentzVector locKPlus1P4_YstarRest = locKPlus1P4_CM;
		TLorentzVector locKPlusFastP4_YstarRest = locKPlusFastP4_CM;
        TLorentzVector locYstarP4_YstarRest = locYstarP4_CM;
		//step 3
		TLorentzVector locXimStarP4_YstarRest = locXimStarP4_CM;
		TLorentzVector locKPlus2P4_YstarRest = locKPlus2P4_CM;
		TLorentzVector locKPlusSlowP4_YstarRest = locKPlusSlowP4_CM;
        TLorentzVector locKMinusP4_YstarRest = locKMinusP4_CM;
		//step 4
		TLorentzVector locDecayingLambdaP4_YstarRest = locDecayingLambdaP4_CM;
		//Boost 4vectos to rest frame of ystar
		locBeamP4_YstarRest.Boost(-boostYstar_Rest);
		locYstarP4_YstarRest.Boost(-boostYstar_Rest);
		locKPlus1P4_YstarRest.Boost(-boostYstar_Rest);
		locKPlusFastP4_YstarRest.Boost(-boostYstar_Rest);
        locKPlus2P4_YstarRest.Boost(-boostYstar_Rest);
		locKPlusSlowP4_YstarRest.Boost(-boostYstar_Rest);
        locXimStarP4_YstarRest.Boost(-boostYstar_Rest);
		locKMinusP4_YstarRest.Boost(-boostYstar_Rest);
		locDecayingLambdaP4_YstarRest.Boost(-boostYstar_Rest);

        // Set helicity frame for ystar
        TVector3 z_hat_Ystar_HF = locYstarP4_CM.Vect().Unit();
        TVector3 y_hat_Ystar_HF = locBeamP4_CM.Vect().Cross(locYstarP4_CM.Vect()).Unit();//y direction normal to production plane gammap
        TVector3 x_hat_Ystar_HF = y_hat_Ystar_HF.Cross(z_hat_Ystar_HF);//maintain right handed coordinate system		
        //TLorentzVector locKPlusSlowP4_HF(locKPlusSlowP4_YstarRest.Vect().Dot(x_hat_Ystar_HF), locKPlusSlowP4_YstarRest.Vect().Dot(y_hat_Ystar_HF), locKPlusSlowP4_YstarRest.Vect().Dot(z_hat_Ystar_HF),locKPlusSlowP4_YstarRest.E()); 
        Double_t KPlusSlow_CosTheta_HF = locKPlusSlowP4_YstarRest.Pz() / locKPlusSlowP4_YstarRest.Vect().Mag();
        Double_t KPlusSlow_Phi_HF = TMath::ATan(locKPlusSlowP4_YstarRest.Py() / locKPlusSlowP4_YstarRest.Px());
        TLorentzVector locXimStarP4__YstarHF(locXimStarP4_YstarRest.E(),locXimStarP4_YstarRest.Vect().Dot(x_hat_Ystar_HF), locXimStarP4_YstarRest.Vect().Dot(y_hat_Ystar_HF), locXimStarP4_YstarRest.Vect().Dot(z_hat_Ystar_HF)); 
        Double_t XimStar_CosTheta_HF = locXimStarP4_YstarRest.Pz() / locXimStarP4_YstarRest.Vect().Mag();
        Double_t XimStar_Phi_HF = TMath::ATan(locXimStarP4_YstarRest.Py() / locXimStarP4_YstarRest.Px());

		//BoostVector in Xim rest frame from ystar rest frame
		TVector3 boostXim_HF = locXimStarP4_YstarRest.BoostVector();
		//Initalize 4Vectors in Xim Rest frame
		//step 1
		TLorentzVector locBeamP4_HF = locBeamP4_YstarRest;
		//step 2
		TLorentzVector locKPlus1P4_HF = locKPlus1P4_YstarRest;
		TLorentzVector locKPlusFastP4_HF = locKPlusFastP4_YstarRest;
		TLorentzVector locYstarP4_HF = locYstarP4_YstarRest;
		//step 3
		TLorentzVector locXimStarP4_HF = locXimStarP4_YstarRest;
		TLorentzVector locKPlus2P4_HF = locKPlus2P4_YstarRest;
		TLorentzVector locKPlusSlowP4_HF = locKPlusSlowP4_YstarRest;
		TLorentzVector locKMinusP4_HF = locKMinusP4_YstarRest;
		//step 4
		TLorentzVector locDecayingLambdaP4_HF = locDecayingLambdaP4_YstarRest;
		//Boost 4vectors in Xim Rest frame
		locBeamP4_HF.Boost(-boostXim_HF);
		locKPlus1P4_HF.Boost(-boostXim_HF);
		locKPlusFastP4_HF.Boost(-boostXim_HF);
		locKPlus2P4_HF.Boost(-boostXim_HF);
		locKPlusSlowP4_HF.Boost(-boostXim_HF);
		locXimStarP4_HF.Boost(-boostXim_HF);
		locKMinusP4_HF.Boost(-boostXim_HF);
		locDecayingLambdaP4_HF.Boost(-boostXim_HF);
		//Create the GF reference frame
		TVector3 z_hat_HF = locXimStarP4_YstarRest.Vect().Unit();//z direction is opposite direction of the boost or Ystar in rest frame
        TVector3 y_hat_HF = locXimStarP4_YstarRest.Vect().Cross(locKMinusP4_YstarRest.Vect()).Unit();//y direction normal to production plane
        TVector3 x_hat_HF = y_hat_HF.Cross(z_hat_HF);//maintain right handed coordinate system		
        //Define angle of Klus2 and XimStar in HF
        TVector3 locKMinusP3_HF(locKMinusP4_HF.Vect().Dot(x_hat_HF), locKMinusP4_HF.Vect().Dot(y_hat_HF), locKMinusP4_HF.Vect().Dot(z_hat_HF)); 
        double KMinus_Angle_HF = z_hat_HF.Angle(locKMinusP4_HF.Vect());
        double KMinus_CosTheta_HF = locKMinusP3_HF.Z() / locKMinusP3_HF.Mag();
		Double_t Lambda_Angle_HF = z_hat_HF.Angle(locDecayingLambdaP4_HF.Vect());
		Double_t Lambda_CosAngle_HF = TMath::Cos(z_hat_HF.Angle(locKMinusP4_HF.Vect()));

        /*******************************
            Path Length Calculations
		*******************************/
        TLorentzVector locProdSpacetimeVertex = dComboBeamWrapper->Get_X4();//Get production vertex
		// Lambda Path Length
        TLorentzVector locDecayingLambdaX4 = dTreeInterface->Get_TObject<TLorentzVector>("DecayingLambda__X4",loc_i);
        TLorentzVector locDeltaSpacetimeLambda = locDecayingLambdaX4 - locProdSpacetimeVertex;//vertex difference
        Int_t locDecayingLambdaPathlenSign = std::round(TMath::Cos(locDeltaSpacetimeLambda.Vect().Angle(locDecayingLambdaP4.Vect())));
        cout << "LAMBDA***************************************" << endl;
        cout << "Pathlen Sign: " << locDecayingLambdaPathlenSign << endl;
        cout << "Vertex Difference: " << locDecayingLambdaX4.Z() - locProdSpacetimeVertex.Z() << endl;
        //
        Double_t locPathLengthLambda = locDecayingLambdaPathlenSign*locDeltaSpacetimeLambda.Vect().Mag();//pathlength is just the magnitude
        Float_t locPathLengthSigmaLambda = Get_Fundamental<Float_t>("DecayingLambda__PathLengthSigma", loc_i);
        Double_t locPathLengthSignificanceLambda = locPathLengthLambda/locPathLengthSigmaLambda;
        Double_t locLifetimeLambda = locDeltaSpacetimeLambda.T();//lifetime of lamb in lab
        Double_t locLifetimeRestFrameLambda = locPathLengthLambda*locDecayingLambdaP4_Measured.M() / (29.9792458 * locDecayingLambdaP4.P());//lifetime of lamb in restframe

        //Make wrappers to test vertex
        Get_ComboWrappers();
        TLorentzVector locVertex0X4 = dStep0Wrapper->Get_X4();
        //TLorentzVector locComboBeamX4 = dTreeInterface->Get_TObject<TLorentzVector>("ComboBeam__X4_KinFit",loc_i);
        TLorentzVector locVertex1X4 = dStep1Wrapper->Get_X4();
        
        /******************************************** EXECUTE ANALYSIS ACTIONS *******************************************/

		// Loop through the analysis actions, executing them in order for the active particle combo
		dAnalyzeCutActions->Perform_Action(); // Must be executed before Execute_Actions()
		if(!Execute_Actions()) //if the active combo fails a cut, IsComboCutFlag automatically set
			continue;

		//if you manually execute any actions, and it fails a cut, be sure to call:
			//dComboWrapper->Set_IsComboCut(true);

		/**************************************** EXAMPLE: FILL CUSTOM OUTPUT BRANCHES **************************************/

		/*
		TLorentzVector locMyComboP4(8.0, 7.0, 6.0, 5.0);
		//for arrays below: 2nd argument is value, 3rd is array index
		//NOTE: By filling here, AFTER the cuts above, some indices won't be updated (and will be whatever they were from the last event)
			//So, when you draw the branch, be sure to cut on "IsComboCut" to avoid these.
		dTreeInterface->Fill_Fundamental<Float_t>("my_combo_array", -2*loc_i, loc_i);
		dTreeInterface->Fill_TObject<TLorentzVector>("my_p4_array", locMyComboP4, loc_i);
		*/

		/**************************************** EXAMPLE: HISTOGRAM BEAM ENERGY *****************************************/

		//Histogram beam energy (if haven't already)
		if(locUsedSoFar_BeamEnergy.find(locBeamID) == locUsedSoFar_BeamEnergy.end())
		{
			dHist_BeamEnergy->Fill(locBeamP4.E()); // Fills in-time and out-of-time beam photon combos
			//dHist_BeamEnergy->Fill(locBeamP4.E(),locHistAccidWeightFactor); // Alternate version with accidental subtraction

			locUsedSoFar_BeamEnergy.insert(locBeamID);
		}

		/************************************ EXAMPLE: HISTOGRAM MISSING MASS SQUARED ************************************/

		//Missing Mass Squared
		double locMissingMassSquared = locMissingP4_Measured.M2();

		//Uniqueness tracking: Build the map of particles used for the missing mass
			//For beam: Don't want to group with final-state photons. Instead use "Unknown" PID (not ideal, but it's easy).
		map<Particle_t, set<Int_t> > locUsedThisCombo_MissingMass;
		locUsedThisCombo_MissingMass[Unknown].insert(locBeamID); //beam
		locUsedThisCombo_MissingMass[KPlus].insert(locKPlus1TrackID);
		locUsedThisCombo_MissingMass[KPlus].insert(locKPlus2TrackID);
		locUsedThisCombo_MissingMass[KMinus].insert(locKMinusTrackID);
		locUsedThisCombo_MissingMass[PiMinus].insert(locPiMinusTrackID);
		locUsedThisCombo_MissingMass[Proton].insert(locProtonTrackID);

		//compare to what's been used so far
		if(locUsedSoFar_MissingMass.find(locUsedThisCombo_MissingMass) == locUsedSoFar_MissingMass.end())
		{
			//unique missing mass combo: histogram it, and register this combo of particles
			dHist_MissingMassSquared->Fill(locMissingMassSquared); // Fills in-time and out-of-time beam photon combos
			//dHist_MissingMassSquared->Fill(locMissingMassSquared,locHistAccidWeightFactor); // Alternate version with accidental subtraction

			locUsedSoFar_MissingMass.insert(locUsedThisCombo_MissingMass);
		}
        
		//ChiSqNDF
		Double_t locChiSqNdf = dComboWrapper->Get_ChiSq_KinFit("") / dComboWrapper->Get_NDF_KinFit("");
        Double_t locConfidenceLvl = dComboWrapper->Get_ConfidenceLevel_KinFit("") ;
        
		/****************************************** FILL FLAT TREE (IF DESIRED) ******************************************/

		// RECOMMENDED: FILL ACCIDENTAL WEIGHT
		// dFlatTreeInterface->Fill_Fundamental<Double_t>("accidweight",locHistAccidWeightFactor);
        
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
        dFlatTreeInterface->Fill_TObject<TLorentzVector>("kplow_p4", locKPlusSlowP4);
        dFlatTreeInterface->Fill_TObject<TLorentzVector>("kplow2d_p4", locKPlusSlow2DCutP4);
        dFlatTreeInterface->Fill_TObject<TLorentzVector>("kphigh_p4", locKPlusFastP4);
        dFlatTreeInterface->Fill_TObject<TLorentzVector>("kphigh2d_p4", locKPlusFast2DCutP4);
        dFlatTreeInterface->Fill_TObject<TLorentzVector>("kplow_p4_ystar_hf", locKPlusSlowP4_HF);
        dFlatTreeInterface->Fill_TObject<TLorentzVector>("ximstar_p4", locXimStarP4);
        dFlatTreeInterface->Fill_TObject<TLorentzVector>("lambda_p4", locDecayingLambdaP4);
        dFlatTreeInterface->Fill_TObject<TLorentzVector>("kp1_km_p4", locKPlus1KMinusP4);
        dFlatTreeInterface->Fill_TObject<TLorentzVector>("kp1_pim_p4", locKPlus1PiMinusP4);
        dFlatTreeInterface->Fill_TObject<TLorentzVector>("kp1_lamb_p4", locKPlus1LambdaP4);
        dFlatTreeInterface->Fill_TObject<TLorentzVector>("kp2_km_p4", locKPlus2KMinusP4);
        dFlatTreeInterface->Fill_TObject<TLorentzVector>("kp2_pim_p4", locKPlus2PiMinusP4);
        dFlatTreeInterface->Fill_TObject<TLorentzVector>("kp2_lamb_p4", locKPlus2LambdaP4);
        dFlatTreeInterface->Fill_TObject<TLorentzVector>("kphigh_km_p4", locKPlusFastKMinusP4);
        dFlatTreeInterface->Fill_TObject<TLorentzVector>("kphigh_pim_p4", locKPlusFastPiMinusP4);
        dFlatTreeInterface->Fill_TObject<TLorentzVector>("kphigh_lamb_p4", locKPlusFastLambdaP4);
        dFlatTreeInterface->Fill_TObject<TLorentzVector>("kplow_km_p4", locKPlusSlowKMinusP4);
        dFlatTreeInterface->Fill_TObject<TLorentzVector>("kplow_pim_p4", locKPlusSlowPiMinusP4);
        dFlatTreeInterface->Fill_TObject<TLorentzVector>("kplow_lamb_p4", locKPlusSlowLambdaP4);
        dFlatTreeInterface->Fill_TObject<TLorentzVector>("km_p_p4", locKMinusProtonP4);
        //dFlatTreeInterface->Fill_TObject<TLorentzVector>("lambda_p4_meas", locDecayingLambdaP4_Measured);
        //vertex x4
        dFlatTreeInterface->Fill_TObject<TLorentzVector>("vertex0_x4", locVertex0X4);
        dFlatTreeInterface->Fill_TObject<TLorentzVector>("vertex1_x4", locVertex1X4);
        // beam stuff
        dFlatTreeInterface->Fill_Fundamental<Double_t>("beam_E", locBeamP4.E());
        dFlatTreeInterface->Fill_Fundamental<Double_t>("beam_rfbunches", locDeltaT_RF);
        dFlatTreeInterface->Fill_Fundamental<Double_t>("beam_vertexX", locProdSpacetimeVertex.X());
        dFlatTreeInterface->Fill_Fundamental<Double_t>("beam_vertexY", locProdSpacetimeVertex.Y());
        dFlatTreeInterface->Fill_Fundamental<Double_t>("beam_vertexZ", locProdSpacetimeVertex.Z());
        // kp1
        dFlatTreeInterface->Fill_Fundamental<Double_t>("kp1_P3", locKPlus1P4.P() );
        dFlatTreeInterface->Fill_Fundamental<Double_t>("kp1_P3_Truth", locKPlus1P4_Truth.P() );
        //dFlatTreeInterface->Fill_Fundamental<Double_t>("kp1_CosTheta", locKPlus1P4.CosTheta() );
        //dFlatTreeInterface->Fill_Fundamental<Double_t>("kp1_Phi", locKPlus1P4.Phi() );
        dFlatTreeInterface->Fill_Fundamental<Double_t>("kphigh_P3", locKPlusFastP4.P() );
        dFlatTreeInterface->Fill_Fundamental<Double_t>("kphigh_Theta", locKPlusFastP4.Theta() );
        dFlatTreeInterface->Fill_Fundamental<Double_t>("kphigh_CosTheta", locKPlusFastP4.CosTheta() );
        dFlatTreeInterface->Fill_Fundamental<Double_t>("kphigh_Phi", locKPlusFastP4.Phi() );
        // kp2
        dFlatTreeInterface->Fill_Fundamental<Double_t>("kp2_P3", locKPlus2P4.P() );
        dFlatTreeInterface->Fill_Fundamental<Double_t>("kp2_P3_Truth", locKPlus2P4_Truth.P() );
        dFlatTreeInterface->Fill_Fundamental<Double_t>("kplow_Theta", locKPlusSlowP4.Theta() );
        dFlatTreeInterface->Fill_Fundamental<Double_t>("kp2_YstarRest_CosTheta", locKPlus2P4_YstarRest.CosTheta() );
        dFlatTreeInterface->Fill_Fundamental<Double_t>("kp2_YstarRest_Phi", locKPlus2P4_YstarRest.Phi() );
        dFlatTreeInterface->Fill_Fundamental<Double_t>("kplow_P3", locKPlusSlowP4.P() );
        dFlatTreeInterface->Fill_Fundamental<Double_t>("kplow_costheta_hf",  KPlusSlow_CosTheta_HF);
        dFlatTreeInterface->Fill_Fundamental<Double_t>("kplow_phi_hf",  KPlusSlow_Phi_HF);
        // ystar
        dFlatTreeInterface->Fill_Fundamental<Double_t>("ystar_M", locYstarP4.M());			  
        dFlatTreeInterface->Fill_Fundamental<Double_t>("ystar_P3", locYstarP4.P() );
        dFlatTreeInterface->Fill_Fundamental<Double_t>("ystar_Theta", locYstarP4.CosTheta() );
        // ximinus
        dFlatTreeInterface->Fill_Fundamental<Double_t>("ximstar_M", locXimStarP4.M());
        dFlatTreeInterface->Fill_Fundamental<Double_t>("xim_P3", locXimStarP4.P() );
        dFlatTreeInterface->Fill_Fundamental<Double_t>("xim_costheta_hf", XimStar_CosTheta_HF );
        // lambda
        dFlatTreeInterface->Fill_Fundamental<Double_t>("decaylamb_M", locDecayingLambdaP4.M());
        dFlatTreeInterface->Fill_Fundamental<Double_t>("lambda_P3", locDecayingLambdaP4.P() );
        dFlatTreeInterface->Fill_Fundamental<Double_t>("lambda_vertexX", locDecayingLambdaX4.X());
        dFlatTreeInterface->Fill_Fundamental<Double_t>("lambda_vertexY", locDecayingLambdaX4.Y());
        dFlatTreeInterface->Fill_Fundamental<Double_t>("lambda_vertexZ", locDecayingLambdaX4.Z());
        dFlatTreeInterface->Fill_Fundamental<Double_t>("lambda_pathlen", locPathLengthLambda);
        dFlatTreeInterface->Fill_Fundamental<Double_t>("lambda_pathlensig", locPathLengthSignificanceLambda);
        dFlatTreeInterface->Fill_Fundamental<Double_t>("lambda_lifetime", locLifetimeLambda);
        dFlatTreeInterface->Fill_Fundamental<Double_t>("lambda_lifetime_restframe", locLifetimeRestFrameLambda);
        // final state particles
        dFlatTreeInterface->Fill_Fundamental<Double_t>("km_P3", locKMinusP4.P() );
        dFlatTreeInterface->Fill_Fundamental<Double_t>("km_costheta_hf", KMinus_CosTheta_HF );     
        dFlatTreeInterface->Fill_Fundamental<Double_t>("pim_P3", locPiMinusP4.P() );
        dFlatTreeInterface->Fill_Fundamental<Double_t>("proton_P3", locProtonP4.P() );
        // other	    		    
        dFlatTreeInterface->Fill_Fundamental<Double_t>("acc_weight", locHistAccidWeightFactor);
        dFlatTreeInterface->Fill_Fundamental<Double_t>("chisqndf", locChiSqNdf);
        dFlatTreeInterface->Fill_Fundamental<Double_t>("confidencelvl", locConfidenceLvl);
        dFlatTreeInterface->Fill_Fundamental<Double_t>("total_mm2", locMissingMassSquared);
        dFlatTreeInterface->Fill_Fundamental<Double_t>("t_dist", -1.0*locT);
        dFlatTreeInterface->Fill_Fundamental<ULong64_t>("evnt_num", locEventNum);
        dFlatTreeInterface->Fill_Fundamental<UInt_t>("combo_num", Get_NumCombos());
        dFlatTreeInterface->Fill_Fundamental<Int_t>("combos_survived", locNumComboSurvivedCut);
        dFlatTreeInterface->Fill_Fundamental<Int_t>("rf_intime_weight", locInTimeBunch);
        
        if(locBestChiSqNdfComboNum == loc_i) dFlatTreeInterface->Fill_Fundamental<Int_t>("best_combo", 1);
        else dFlatTreeInterface->Fill_Fundamental<Int_t>("best_combo", 0);
        
        if(locBestChiSqNdfComboNum_1 == loc_i) dFlatTreeInterface->Fill_Fundamental<Int_t>("best_combo_1", 1);
        else dFlatTreeInterface->Fill_Fundamental<Int_t>("best_combo_1", 0);
        
        //cout << "ChiSqNdf: " << locChiSqNdf << "\nXiPathLen: " << locPathLengthXi << "\nXiPathLenSigma: " << locPathLengthSigmaXi << "\nXiPathLenSignificance: " << locPathLengthSignificanceXi << "\n" << endl;
			  
        //FILL FLAT TREE
        Fill_FlatTree(); //for the active combo		   
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

void DSelector_kpkpkmlamb::Finalize(void)
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
