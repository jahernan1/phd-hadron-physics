#include "DSelector_thrown_kpkpxim.h"

void DSelector_thrown_kpkpxim::Init(TTree *locTree)
{
	// USERS: IN THIS FUNCTION, ONLY MODIFY SECTIONS WITH A "USER" OR "EXAMPLE" LABEL. LEAVE THE REST ALONE.

	// The Init() function is called when the selector needs to initialize a new tree or chain.
	// Typically here the branch addresses and branch pointers of the tree will be set.
	// Init() will be called many times when running on PROOF (once per file to be processed).

	//USERS: SET OUTPUT FILE NAME //can be overriden by user in PROOF
	dOutputFileName = "thrown_kpkpxim.root"; //"" for none
	dFlatTreeFileName = "flatTree_thrown_kpkpxim.root"; //output flat tree (one combo per tree entry), "" for none
	dFlatTreeName = "flatTree_thrown_kpkpxim"; //if blank, default name will be chosen
    //USERS: SET OUTPUT TREE FILES/NAMES //e.g. binning into separate files for AmpTools
	//dOutputTreeFileNameMap["Bin1"] = "mcgen_bin1.root"; //key is user-defined, value is output file name
	//dOutputTreeFileNameMap["Bin2"] = "mcgen_bin2.root"; //key is user-defined, value is output file name
	//dOutputTreeFileNameMap["Bin3"] = "mcgen_bin3.root"; //key is user-defined, value is output file name

	//Because this function gets called for each TTree in the TChain, we must be careful:
		//We need to re-initialize the tree interface & branch wrappers, but don't want to recreate histograms
	bool locInitializedPriorFlag = dInitializedFlag; //save whether have been initialized previously
	DSelector::Init(locTree); //This must be called to initialize wrappers for each new TTree
	//gDirectory now points to the output file with name dOutputFileName (if any)
	if(locInitializedPriorFlag)
		return; //have already created histograms, etc. below: exit

	dPreviousRunNumber = 0;

    //Set trigger condition to avoid crashes
    //dSkipNoTriggerEvents = false;
	/******************************** EXAMPLE USER INITIALIZATION: STAND-ALONE HISTOGRAMS *******************************/

	dHist_thrown_YstarInvariantMass = new TH1I("thrown_YstarInvariantMass",";M(K_{2}#Xi^{-}) (GeV/c^{2}); Events", 280, 1.7, 4.5);
	dHist_thrown_YstarInvariantMass_wrong = new TH1I("thrown_YstarInvariantMass_wrong",";M(K_{2}#Xi^{-}) (GeV/c^{2}); Events", 280, 1.7, 4.5);
	dHist_thrown_XiInvariantMass = new TH1I("thrown_XiInvariantMass",";M(#Lambda#pi^{-}) (GeV/c^{2}); Events", 300, 0, 1.5);
    dHist_thrown_XiLifetimeRestFrame_postCL = new TH1I("thrown_XiLifetimeRestFrame_postCL",";#Xi Lifetime RestFrame (ns); Events", 150, 0, 3);
	dHist_thrown_KPlus1P3VsTheta = new TH2I("thrown_KPlus1P3VsTheta","KPlus1 Thrown Kinematic ; #Theta; 3-Momentum", 180, 0, 180, 500, 0, 10);
	dHist_thrown_KPlus1P3VsTheta_CM = new TH2I("thrown_KPlus1P3VsTheta_CoM","KPlus1 COM Thrown Kinematic ; #Theta; 3-Momentum", 180, 0, 180, 500, 0, 10);
	dHist_thrown_KPlus2P3VsTheta = new TH2I("thrown_KPlus2P3VsTheta","KPlus2 Thrown Kinematic ; #Theta; 3-Momentum", 180, 0, 180, 500, 0, 10);
	dHist_thrown_KPlus2P3VsTheta_CM = new TH2I("thrown_KPlus2P3VsTheta_CoM","KPlus2 COM Thrown Kinematic ; #Theta; 3-Momentum", 180, 0, 180, 500, 0, 10);
	dHist_thrown_YstarP3VsTheta = new TH2I("thrown_YstarP3VsTheta","Ystar Thrown Kinematic ; #Theta; 3-Momentum", 180, 0, 180, 500, 0, 10);
	dHist_thrown_YstarP3VsTheta_CM = new TH2I("thrown_YstarP3VsTheta_CoM","Ystar COM Thrown Kinematic ; #Theta; 3-Momentum", 180, 0, 180, 500, 0, 10);

	dHist_thrown_LambdaP3VsTheta = new TH2I("thrown_LambdaP3VsTheta","#Lambda Thrown Kinematic ; #Theta; 3-Momentum", 180, 0, 180, 500, 0, 10);
	dHist_thrown_LambdaP3VsTheta_CM = new TH2I("thrown_LambdaP3VsTheta_CM","#Lambda CM Thrown Kinematic ; #Theta_{CM}; 3-Momentum", 180, 0, 180, 500, 0, 10);
	dHist_thrown_LambdaP3VsTheta_HF = new TH2I("thrown_LambdaP3VsTheta_HF","#Lambda, #Xi->#Lambda#pi ; #Theta_{HF}; 3-Momentum", 180, 0, 180, 500, 0, 10);
	dHist_thrown_LambdaP3VsCosTheta_HF = new TH2I("thrown_LambdaP3VsCosTheta_HF","#Lambda, #Xi->#Lambda#pi ; cos(#Theta_{HF}; 3-Momentum", 200, -1, 1, 500, 0, 10);
	
	dHist_thrown_PiMinus1P3VsTheta = new TH2I("thrown_PiMinus1P3VsTheta","#pi^{-}_{1} Thrown Kinematic ; #Theta; 3-Momentum", 180, 0, 180, 500, 0, 10);
	dHist_thrown_PiMinus1P3VsTheta_HF = new TH2I("thrown_PiMinus1P3VsTheta_HF","#pi^{-}_{1} HF Thrown Kinematic ; #Theta_{HF}; 3-Momentum", 180, 0, 180, 500, 0, 10);
	dHist_thrown_PiMinus1P3VsCosTheta_HF = new TH2I("thrown_PiMinus1P3VsCosTheta_HF","#pi^{-}_{1} HF Thrown Kinematic ; #cos(#Theta_{HF}); 3-Momentum", 200, -1, 1, 500, 0, 10);

	dHist_thrown_PiMinus2P3VsTheta = new TH2I("thrown_PiMinus2P3VsTheta","#pi^{-}_{2} Thrown Kinematic ; #Theta; 3-Momentum", 180, 0, 180, 500, 0, 10);
	
	dHist_thrown_ProtonP3VsTheta = new TH2I("thrown_ProtonP3VsTheta","p Thrown Kinematic ; #Theta; 3-Momentum", 180, 0, 180, 500, 0, 10);

	dHist_thrown_Egamma = new TH1I("thrown_Egamma", ";E_{#gamma} (GeV)", 100, 6.4, 11.4);
	//dHist_thrown_Egamma = new TH1I("thrown_Egamma", ";E_{#gamma} (GeV)", 30, 3.0, 6.0);
	dHist_thrown_Egamma_t = new TH2I("thrown_t"," ; E_{#gamma} (GeV) ; -t", 100, 6.4 ,11.4, 200, 0.0, 10.0);

	dHist_thrown_ReactionTypes = new TH1I("thrown_ReactionTypes", " ; ; Counts", 6, 0, 6); 
	dHist_thrown_ReactionNumber = new TH1I("thrown_ReactionNumber", " ; Thrown Number; Counts", 12, 0, 12); 

	/************************** EXAMPLE USER INITIALIZATION: CUSTOM OUTPUT BRANCHES - FLAT TREE *************************/
    dFlatTreeInterface->Create_Branch_NoSplitTObject<TLorentzVector>("beam_p4");
    dFlatTreeInterface->Create_Branch_NoSplitTObject<TLorentzVector>("kp1_p4");
    dFlatTreeInterface->Create_Branch_NoSplitTObject<TLorentzVector>("kpfast_p4");
    dFlatTreeInterface->Create_Branch_NoSplitTObject<TLorentzVector>("ystar_p4");
    dFlatTreeInterface->Create_Branch_NoSplitTObject<TLorentzVector>("kp2_p4");
    dFlatTreeInterface->Create_Branch_NoSplitTObject<TLorentzVector>("kpslow_p4");
    dFlatTreeInterface->Create_Branch_NoSplitTObject<TLorentzVector>("kp2_p4_ystar_hf");
    dFlatTreeInterface->Create_Branch_NoSplitTObject<TLorentzVector>("decayxim_p4");
    dFlatTreeInterface->Create_Branch_NoSplitTObject<TLorentzVector>("pim1_p4");    
    dFlatTreeInterface->Create_Branch_NoSplitTObject<TLorentzVector>("lambda_p4");
    dFlatTreeInterface->Create_Branch_NoSplitTObject<TLorentzVector>("pim2_p4");
    dFlatTreeInterface->Create_Branch_NoSplitTObject<TLorentzVector>("proton_p4");

    dFlatTreeInterface->Create_Branch_NoSplitTObject<TLorentzVector>("kp1_cm_p4");

    dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("kp2_costheta_hf");
    dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("kp2_phi_hf");
    dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("xim_costheta");
    dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("xim_costheta_hf");
    dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("xim_costheta_gen_amp");
    dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("xim_phi_hf");
    dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("pim1_costheta_hf");
    dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("xim_pathlen");
    dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("xim_lifetime");
    dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("xim_lifetime_restframe");
    dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("lambda_pathlen");
    dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("lambda_lifetime");
    dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("lambda_lifetime_restframe");
    dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("beam_E");
    dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("t_dist");
    dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("t_dist_fast");
    dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("beam_vertexZ");
    dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("ystar_vertexZ");
    dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("xim_vertexZ");
	dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("lambda_vertexZ");
    dFlatTreeInterface->Create_Branch_Fundamental<Int_t>("main_pid");//weight for only main event for comparison plots
    
    // dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("kp2_YstarRest_CosTheta");
	// dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("kp2_YstarRest_Phi");
    
	// dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("chisqndf");
	// dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("confidencelvl");
    // dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("total_mm2");    

    // dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("beam_vertexX");
	// dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("beam_vertexY");
	
	// dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("xim_vertexX");
	// dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("xim_vertexY");
	
	
	// dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("xim_pathlensig");
	// dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("xim_lifetime");
	// dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("xim_lifetime_restframe");
	// dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("lambda_vertexX");
	// dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("lambda_vertexY");

	
	// dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("lambda_pathlensig");
	// dFlatTreeInterface->Create_Branch_Fundamental<Double_t>("lambda_lifetime");
	
    /************************************* ADVANCED EXAMPLE: CHOOSE BRANCHES TO READ ************************************/
	//TO SAVE PROCESSING TIME
		//If you know you don't need all of the branches/data, but just a subset of it, you can speed things up
		//By default, for each event, the data is retrieved for all branches
		//If you know you only need data for some branches, you can skip grabbing data from the branches you don't need
		//Do this by doing something similar to the commented code below

	//dTreeInterface->Clear_GetEntryBranches(); //now get none
	//dTreeInterface->Register_GetEntryBranch("Proton__P4"); //manually set the branches you want
}

Bool_t DSelector_thrown_kpkpxim::Process(Long64_t locEntry)
{
	// The Process() function is called for each entry in the tree. The entry argument
	// specifies which entry in the currently loaded tree is to be processed.
	//
	// This function should contain the "body" of the analysis. It can contain
	// simple or elaborate selection criteria, run algorithms on the data
	// of the event and typically fill histograms
	//
	// The processing can be stopped by calling Abort().
	// Use fStatus to set the return value of TTree::Process().
	// The return value is currently not used.

	//CALL THIS FIRST
	DSelector::Process(locEntry); //Gets the data from the tree for the entry
	//cout << "RUN " << Get_RunNumber() << ", EVENT " << Get_EventNumber() << endl;

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

	//INSERT USER ANALYSIS UNIQUENESS TRACKING HERE
	set<map<Particle_t,set<Int_t>>>locUsedSoFar_thrown;

	/******************************************* LOOP OVER THROWN DATA ***************************************/

	//Thrown beam: just use directly
	double locBeamEnergyUsedForBinning = 0.0;
    TLorentzVector locBeamP4;
	TLorentzVector locBeamX4;
	if(dThrownBeam != NULL)
	  {
		locBeamEnergyUsedForBinning = dThrownBeam->Get_P4().E();
		locBeamP4 = dThrownBeam->Get_P4();
		locBeamX4 = dThrownBeam->Get_X4();
	  }

	TLorentzVector locKPlus1P4, locKPlusSlowP4, locKPlus1X4;
	TLorentzVector locKPlus2P4, locKPlusFastP4, locKPlus2X4;
	TLorentzVector locXiMinusP4, locXiMinusX4;
	TLorentzVector locPiMinus1P4, locPiMinus1X4;
    TLorentzVector locPiMinus2P4, locPiMinus2X4;
	TLorentzVector locProtonP4, locProtonX4;
	TLorentzVector locLambdaP4, locLambdaX4;
	vector<Int_t> arr_Reaction;
	vector<vector<Int_t>> arr_ReactionPID =
	  {
	    {11,11,9,9,14,23,18},
	    {11,11,9,13,1,1,23,18,7},
        {11,11,9,13,2,3,1,23,18,7},
	    {11,11,9,23},
	    {11,11}
	  };

	cout << "NEW GENERATED EVENT>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>" << endl;
	//Loop over throwns
	for(UInt_t loc_i = 0; loc_i < Get_NumThrown(); ++loc_i)
	{
		//Set branch array indices corresponding to this particle
		dThrownWrapper->Set_ArrayIndex(loc_i);

		//Do stuff with the wrapper here ...
		Particle_t locPID = dThrownWrapper->Get_PID();
		Int_t locParentIndex = dThrownWrapper->Get_ParentIndex();
		TLorentzVector locThrownP4 = dThrownWrapper->Get_P4();
		TLorentzVector locThrownX4 = dThrownWrapper->Get_X4();
		
        arr_Reaction.push_back(locPID);

		cout << "Thrown " << loc_i << ": " << locPID << ", " << locThrownP4.Px() << ", " << locThrownP4.Py() << ", " << locThrownP4.Pz() << ", " << locThrownP4.E() << locThrownP4.P() << endl;
		cout << "\t Parent Particle Index: " << locParentIndex << endl;
        cout << "\t Paricle Vertex: " << locThrownX4.Z() << endl;

		if(locPID == 11) 
          {
            if(loc_i == 0) {locKPlus1P4 = locKPlusFastP4 = locThrownP4; locKPlus1X4 = locThrownX4;}
            if(loc_i == 1) {locKPlus2P4 = locKPlusSlowP4 = locThrownP4; locKPlus2X4 = locThrownX4;}
          }
		if(locPID == 9)
          {
            if(loc_i == 2) {locPiMinus1P4 = locThrownP4; locPiMinus1X4 = locThrownX4;}
            if(loc_i == 3) {locPiMinus2P4 = locThrownP4; locPiMinus2X4 = locThrownX4;}
          }
		if(locPID == 14 ) {locProtonP4 = locThrownP4; locProtonX4 = locThrownX4;}		  
		if(locPID == 23 ) {locXiMinusP4 = locThrownP4; locXiMinusX4 = locThrownX4;}
		if(locPID == 18 ) {locLambdaP4 = locThrownP4;  locLambdaX4 = locThrownX4;}
	}

    // if(locKPlus2P4.P()>locKPlus1P4.P())
    //   {
    //     locKPlusSlowP4 = locKPlus1P4;
    //     locKPlusFastP4 = locKPlus2P4;
    //   }
    
    TLorentzVector locYstarP4 = locKPlusSlowP4 + locXiMinusP4;
    TLorentzVector locYstarX4 = locKPlus2X4 + locXiMinusX4;
        
	TLorentzVector locYstarP4_wrong = locKPlusFastP4 + locXiMinusP4;
	double tval = (locBeamP4 - locKPlus1P4).M2();
    double tvalfast = (locBeamP4 - locKPlusFastP4).M2();
    
	//BoostVector in COM frame
  	TLorentzVector locCoMP4 = locBeamP4 + dTargetP4;
  	TVector3 boostCoM = locCoMP4.BoostVector();
    //Define com 4-vectors
  	TLorentzVector locBeamP4_CM = locBeamP4;
    TLorentzVector locBeamX4_CM = locBeamX4;
	TLorentzVector locKPlus1P4_CM = locKPlus1P4;
	TLorentzVector locKPlusFastP4_CM = locKPlusFastP4;
	TLorentzVector locYstarP4_CM = locYstarP4;
    TLorentzVector locYstarX4_CM = locYstarX4;
	TLorentzVector locKPlus2P4_CM = locKPlus2P4;
	TLorentzVector locKPlusSlowP4_CM = locKPlusSlowP4;
	TLorentzVector locXiMinusP4_CM = locXiMinusP4;
    TLorentzVector locXiMinusX4_CM = locXiMinusX4;
	TLorentzVector locPiMinus1P4_CM = locPiMinus1P4;
	TLorentzVector locDecayingLambdaP4_CM =  locLambdaP4;
    TLorentzVector locDecayingLambdaX4_CM =  locLambdaX4;
    locBeamP4_CM.Boost(-boostCoM);
    locBeamX4_CM.Boost(-boostCoM);
	locKPlus1P4_CM.Boost(-boostCoM);
	locKPlusFastP4_CM.Boost(-boostCoM);
	locYstarP4_CM.Boost(-boostCoM);
    locYstarX4_CM.Boost(-boostCoM);
	locKPlus2P4_CM.Boost(-boostCoM);
	locKPlusSlowP4_CM.Boost(-boostCoM);
	locXiMinusP4_CM.Boost(-boostCoM);
    locXiMinusX4_CM.Boost(-boostCoM);
	locPiMinus1P4_CM.Boost(-boostCoM);
	locDecayingLambdaP4_CM.Boost(-boostCoM);
    locDecayingLambdaX4_CM.Boost(-boostCoM);

	//Boost 4vector into rest frame of ystar from CoM 
	TVector3 boostYstar_Rest = locYstarP4_CM.BoostVector();
	//Initialize 4vectors in ystar rest frame
	//step 1
	TLorentzVector locBeamP4_YstarRest = locBeamP4_CM;
    TLorentzVector locBeamX4_YstarRest = locBeamX4_CM;
	//step 2
	TLorentzVector locKPlus1P4_YstarRest = locKPlus1P4_CM;
	TLorentzVector locKPlusFastP4_YstarRest = locKPlusFastP4_CM;
	TLorentzVector locYstarP4_YstarRest = locYstarP4_CM;
    TLorentzVector locYstarX4_YstarRest = locYstarX4_CM;
	//step 3
	TLorentzVector locXiMinusP4_YstarRest = locXiMinusP4_CM;
    TLorentzVector locXiMinusX4_YstarRest = locXiMinusX4_CM;
    TLorentzVector locKPlus2P4_YstarRest = locKPlus2P4_CM;
	TLorentzVector locKPlusSlowP4_YstarRest = locKPlusSlowP4_CM;
	TLorentzVector locPiMinus1P4_YstarRest = locPiMinus1P4_CM;
	//step 4
	TLorentzVector locDecayingLambdaP4_YstarRest = locDecayingLambdaP4_CM;
    TLorentzVector locDecayingLambdaX4_YstarRest = locDecayingLambdaX4_CM;
	//Boost 4vectos to rest frame of ystar
	locBeamP4_YstarRest.Boost(-boostYstar_Rest);
    locBeamX4_YstarRest.Boost(-boostYstar_Rest);
	locYstarP4_YstarRest.Boost(-boostYstar_Rest);
    locYstarX4_YstarRest.Boost(-boostYstar_Rest);
	locKPlus1P4_YstarRest.Boost(-boostYstar_Rest);
	locKPlusFastP4_YstarRest.Boost(-boostYstar_Rest);
	locKPlus2P4_YstarRest.Boost(-boostYstar_Rest);
	locKPlusSlowP4_YstarRest.Boost(-boostYstar_Rest);
	locXiMinusP4_YstarRest.Boost(-boostYstar_Rest);
	locPiMinus1P4_YstarRest.Boost(-boostYstar_Rest);
	locDecayingLambdaP4_YstarRest.Boost(-boostYstar_Rest);
		
    // Set helicity frame for ystar
    TVector3 z_hat_Ystar_HF = locYstarP4_CM.Vect().Unit();
    TVector3 y_hat_Ystar_HF = locBeamP4_CM.Vect().Cross(locYstarP4_CM.Vect()).Unit();//y direction normal to production plane gammap
    TVector3 x_hat_Ystar_HF = y_hat_Ystar_HF.Cross(z_hat_Ystar_HF);//maintain right handed coordinate system

    // get cascade hf variables
    TVector3 locXiMinusP3_HF(locXiMinusP4_YstarRest.Vect().Dot(x_hat_Ystar_HF), locXiMinusP4_YstarRest.Vect().Dot(y_hat_Ystar_HF), locXiMinusP4_YstarRest.Vect().Dot(z_hat_Ystar_HF));
    Double_t XiMinus_Phi = TMath::ATan(locXiMinusP4_YstarRest.Py() / locXiMinusP4_YstarRest.Px());
    Double_t XiMinus_CosTheta = locXiMinusP4_YstarRest.Pz() / locXiMinusP4_YstarRest.Vect().Mag();
    Double_t XiMinus_CosTheta_HF = locXiMinusP3_HF.Pz() / locXiMinusP3_HF.Mag();
    Double_t XiMinus_Phi_HF = TMath::ATan(locXiMinusP3_HF.Py() / locXiMinusP3_HF.Px());
    
    // get kplow hf variables
    TVector3 locKPlus2P3_HF(locKPlus2P4_YstarRest.Vect().Dot(x_hat_Ystar_HF), locKPlus2P4_YstarRest.Vect().Dot(y_hat_Ystar_HF), locKPlus2P4_YstarRest.Vect().Dot(z_hat_Ystar_HF)); 
    Double_t KPlus2_CosTheta_HF = locKPlus2P3_HF.Pz() / locKPlus2P3_HF.Mag();
    Double_t KPlus2_Phi_HF = TMath::ATan(locKPlus2P3_HF.Py() / locKPlus2P3_HF.Px());
        
    //BoostVector in Xim rest frame from ystar rest frame
	TVector3 boostXim_HF = locXiMinusP4_YstarRest.BoostVector();
    //Initalize 4Vectors in Xim Rest frame
	//step 1
	TLorentzVector locBeamP4_HF = locBeamP4_YstarRest;
    TLorentzVector locBeamX4_HF = locBeamX4_YstarRest;
	//step 2
	TLorentzVector locKPlus1P4_HF = locKPlus1P4_YstarRest;
	TLorentzVector locKPlusFastP4_HF = locKPlusFastP4_YstarRest;
	TLorentzVector locYstarP4_HF = locYstarP4_YstarRest;
    TLorentzVector locYstarX4_HF = locYstarX4_YstarRest;
	//step 3
	TLorentzVector locXiMinusP4_HF = locXiMinusP4_YstarRest;
    TLorentzVector locXiMinusX4_HF = locXiMinusX4_YstarRest;
	TLorentzVector locKPlus2P4_HF = locKPlus2P4_YstarRest;
	TLorentzVector locKPlusSlowP4_HF = locKPlusSlowP4_YstarRest;
	TLorentzVector locPiMinus1P4_HF = locPiMinus1P4_YstarRest;
	//step 4
	TLorentzVector locDecayingLambdaP4_HF = locDecayingLambdaP4_YstarRest;
    TLorentzVector locDecayingLambdaX4_HF = locDecayingLambdaX4_YstarRest;
	//Boost 4vectors in Xim Rest frame
	locBeamP4_HF.Boost(-boostXim_HF);
    locBeamX4_HF.Boost(-boostXim_HF);
	locKPlus1P4_HF.Boost(-boostXim_HF);
	locKPlusFastP4_HF.Boost(-boostXim_HF);
    locYstarP4_HF.Boost(-boostXim_HF);
    locYstarX4_HF.Boost(-boostXim_HF);
    locKPlus2P4_HF.Boost(-boostXim_HF);
	locKPlusSlowP4_HF.Boost(-boostXim_HF);
	locXiMinusP4_HF.Boost(-boostXim_HF);
    locXiMinusX4_HF.Boost(-boostXim_HF);
    locPiMinus1P4_HF.Boost(-boostXim_HF);
	locDecayingLambdaP4_HF.Boost(-boostXim_HF);
    locDecayingLambdaX4_HF.Boost(-boostXim_HF);

    //Create the GF reference frame
	TVector3 z_hat_HF = locXiMinusP4_YstarRest.Vect().Unit();//z direction is opposite direction of the boost or Ystar in rest frame
	TVector3 y_hat_HF = locXiMinusP4_YstarRest.Vect().Cross(locPiMinus1P4_YstarRest.Vect()).Unit();//y direction normal to production plane
	TVector3 x_hat_HF = y_hat_HF.Cross(z_hat_HF);//maintain right handed coordinate system		
	//Define angle of Klus2 and XiMinus in HF
	TVector3 locPiMinus1P3_HF(locPiMinus1P4_HF.Vect().Dot(x_hat_HF), locPiMinus1P4_HF.Vect().Dot(y_hat_HF), locPiMinus1P4_HF.Vect().Dot(z_hat_HF)); 
	double PiMinus1_Angle_HF = z_hat_HF.Angle(locPiMinus1P4_HF.Vect());
	double PiMinus1_CosTheta_HF = locPiMinus1P3_HF.Z() / locPiMinus1P3_HF.Mag();
	double Lambda_Angle_HF = z_hat_HF.Angle(locDecayingLambdaP4_HF.Vect());
	double Lambda_CosAngle_HF = TMath::Cos(z_hat_HF.Angle(locDecayingLambdaP4_HF.Vect()));

    //Lambda Restframe
    TVector3 boostLambdaRest = locDecayingLambdaP4_HF.BoostVector();
    TLorentzVector locXiMinusX4_LambRest = locXiMinusP4_HF;
    TLorentzVector locLambdaX4_LambRest = locDecayingLambdaP4_HF;
    locLambdaX4_LambRest.Boost(-boostLambdaRest);
    locXiMinusX4_LambRest.Boost(-boostLambdaRest);


    // try something for gen_amp_v2
    TVector3 boostYstar = locYstarP4.BoostVector();
    //Initialize 4vectors in ystar rest frame
    TLorentzVector locBeamP4_boostYstar = locBeamP4;
    locBeamP4_boostYstar.Boost(-boostYstar);
    TLorentzVector locKPlus1P4_boostYstar = locKPlus1P4;
    locKPlus1P4_boostYstar.Boost(-boostYstar);
    TLorentzVector locYstarP4_boostYstar = locYstarP4;
    locYstarP4_boostYstar.Boost(-boostYstar);
    TLorentzVector locXiMinusP4_boostYstar = locXiMinusP4;
    locXiMinusP4_boostYstar.Boost(-boostYstar);
    TLorentzVector locKPlus2P4_boostYstar = locKPlus2P4;
    locKPlus2P4_boostYstar.Boost(-boostYstar);

    TVector3 z_YstarRest = locYstarP4.Vect().Unit();
    TVector3 y_YstarRest = locBeamP4.Vect().Cross(locYstarP4.Vect()).Unit();//y direction normal to production plane gammap
    TVector3 x_YstarRest = y_YstarRest.Cross(z_YstarRest);//maintain right handed coordinate system

    // get cascade hf variables
    TVector3 locXiMinusP3_YstarRest(locXiMinusP4_boostYstar.Vect().Dot(x_YstarRest),
                                    locXiMinusP4_boostYstar.Vect().Dot(y_YstarRest),
                                    locXiMinusP4_boostYstar.Vect().Dot(z_YstarRest));
    Double_t XiMinus_CosTheta_YstarRest = locXiMinusP4_boostYstar.Pz() / locXiMinusP4_boostYstar.Vect().Mag();
        
    /***************************************************************************
     ********************** Xim lifetime
    ****************************************************************************/
    //TLorentzVector locDecayingLambdaP4 = locPiMinus2P4 +	locProtonP4; //Decaying Lambda for M18 is manually calculated
    TLorentzVector locProdSpacetimeVertex = locBeamX4;//Get production vertex
    // Xim Path Length
    TLorentzVector locXiMinusProd = locXiMinusX4;
    TLorentzVector locLambdaProd = locLambdaX4;
    TLorentzVector locDeltaSpacetimeXi = locXiMinusProd - locLambdaProd;
    //TLorentzVector locDeltaSpacetimeXi = locXiMinusX4_YstarRest - locBeamX4_YstarRest;//vertex difference
    Double_t locPathLengthXi = locDeltaSpacetimeXi.Vect().Mag();//pathlength is just the magnitude
    Double_t locLifetimeXi = locDeltaSpacetimeXi.T();//lifetime of xi in lab
    Double_t locLifetimeRestFrameXi = locPathLengthXi*locXiMinusP4.M() / (29.9792458 * locXiMinusP4.P());//lifetime of xi in restframe t = (travel distance)*(xi mass)/(xi mom) // add 1/c[cm/ns] to correct dimensionality
    // Double_t locLifetimeRestFrameXi = locPathLengthXi*locXiMinusP4.M() / (29.9792458 * locXiMinusP4_YstarRest.P());

    /***************************************************************************
     ********************** Lambda lifetime
    ****************************************************************************/
    //TLorentzVector locDeltaSpacetimeLambda = locLambdaX4 - locProdSpacetimeVertex;//vertex difference
    TLorentzVector locLambdaDecay = locPiMinus2X4;
    TLorentzVector locDeltaSpacetimeLambda = locLambdaProd - locLambdaDecay;//vertex difference
    Double_t locPathLengthLambda = locDeltaSpacetimeLambda.Vect().Mag();//pathlength is just the magnitude
    Double_t locLifetimeLambda = locDeltaSpacetimeLambda.T();//lifetime of lamb in lab
    Double_t locLifetimeRestFrameLambda = locPathLengthLambda*locLambdaP4.M() / (29.9792458 * locLambdaP4.P());//lifetime of lamb in restframe
    // Double_t locLifetimeRestFrameLambda = locPathLengthLambda*locLambdaP4.M() / (29.9792458 * locDecayingLambdaP4_HF.P());//lifetime of lamb in restframe

    /************************************************************************************************************************************************************************      ********** PRINT PARTICLE INFO *****************************************************************************************************************************************      *************************************************************************************************/
    cout << endl;
    cout << "Number of thrown particles: " << Get_NumThrown() << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>" << endl;
    cout << "GammaP4: (" << locBeamP4.E() << ", " << locBeamP4.Px() << ", " << locBeamP4.Py() << ", " << locBeamP4.Pz() << ")" << endl;
    cout << "\t Mass: " << locBeamP4.M() << "\t Momentum: " << locBeamP4.P() << endl; 
    cout << endl;

    cout << "KPlus1P4: (" << locKPlus1P4.E() << ", " << locKPlus1P4.Px() << ", " << locKPlus1P4.Py() << ", " << locKPlus1P4.Pz() << ")" << endl; 
    cout << "\t Mass: " << locKPlus1P4.M() << "\t Momentum: " << locKPlus1P4.P() << endl; 
    cout << endl;

    cout << "YstarP4: (" << locYstarP4.E() << ", " << locYstarP4.Px() << ", " << locYstarP4.Py() << ", " << locYstarP4.Pz() << ")" << endl;
    cout << "\t Mass: " << locYstarP4.M() << "\t Momentum: " << locYstarP4.P() << endl; 
    cout << endl;

    cout << "KPlus2P4: (" << locKPlus2P4.E() << ", " << locKPlus2P4.Px() << ", " << locKPlus2P4.Py() << ", " << locKPlus2P4.Pz() << ")" << endl;
    cout << "\t Mass: " << locKPlus2P4.M() << "\t Momentum: " << locKPlus2P4.P() << endl; 
    cout << endl;

    cout << "XiMinusP4: (" << locXiMinusP4.E() << ", " << locXiMinusP4.Px() << ", " << locXiMinusP4.Py() << ", " << locXiMinusP4.Pz() << ")" << endl; 
    cout << "\t Mass: " << locXiMinusP4.M() << "\t Momentum: " << locXiMinusP4.P() << endl; 
    cout << endl;

    cout << "PiMinus1P4: (" << locPiMinus1P4.E() << ", " << locPiMinus1P4.Px() << ", " << locPiMinus1P4.Py() << ", " << locPiMinus1P4.Pz() << ")" << endl; 
    cout << "\t Mass: " << locPiMinus1P4.M() << "\t Momentum: " << locPiMinus1P4.P() << endl; 
    cout << endl;

    cout << "LambdaP4P4: (" << locLambdaP4.E() << ", " << locLambdaP4.Px() << ", " << locLambdaP4.Py() << ", " << locLambdaP4.Pz() << ")" << endl;
    cout << "\t Mass: " << locLambdaP4.M() << "\t Momentum: " << locLambdaP4.P() << endl; 
    cout << endl;
	      
    cout << "PiMinus2P4: (" << locPiMinus2P4.E() << ", " << locPiMinus2P4.Px() << ", " << locPiMinus2P4.Py() << ", " << locPiMinus2P4.Pz() << ")" << endl;
    cout << "\t Mass: " << locPiMinus2P4.M() << "\t Momentum: " << locPiMinus2P4.P() << endl; 
    cout << endl;
	      
    cout << "ProtonP4: (" << locProtonP4.E() << ", " << locProtonP4.Px() << ", " << locProtonP4.Py() << ", " << locProtonP4.Pz() << ")" << endl; 
    cout << "\t Mass: " << locProtonP4.M() << "\t Momentum: " << locProtonP4.P() << endl; 
    cout << endl;
	    
    //************************************************************************************************************************************************************************************* FILL HISTOGRAMS**********************************************************************************************************************************************************************************************************************************************
    
    dHist_thrown_ReactionNumber->Fill(Get_NumThrown());
    
    //Get information of reaction types generated
    if (arr_Reaction == arr_ReactionPID[0] ){
      //
      dHist_thrown_ReactionTypes->Fill("K^{+} K^{+} #pi^{-} #pi^{-} p", 1);
      // Cross Section Hists
      dHist_thrown_Egamma->Fill(locBeamP4.E());
      dHist_thrown_Egamma_t->Fill(locBeamP4.E(), -1.0*tval );
    }
    else if (arr_Reaction == arr_ReactionPID[1] ){ 
      dHist_thrown_ReactionTypes->Fill("K^{+} K^{+} #pi^{-} n #gamma #gamma", 1);
      // Cross Section Hists
      dHist_thrown_Egamma->Fill(locBeamP4.E());
      dHist_thrown_Egamma_t->Fill(locBeamP4.E(), -1.0*tval );
    }
    else if (arr_Reaction == arr_ReactionPID[2] ){ 
      dHist_thrown_ReactionTypes->Fill("K^{+} K^{+} #pi^{-} n e^{+} e^{-} #gamma", 1);
      // Cross Section Hists
      dHist_thrown_Egamma->Fill(locBeamP4.E());
      dHist_thrown_Egamma_t->Fill(locBeamP4.E(), -1.0*tval );
    }
    else if (arr_Reaction == arr_ReactionPID[3])
      {dHist_thrown_ReactionTypes->Fill("K^{+} K^{+} #pi^{-}", 1); }
    else if (arr_Reaction == arr_ReactionPID[4])
      {dHist_thrown_ReactionTypes->Fill("K^{+} K^{+}", 1);}
    else
      {dHist_thrown_ReactionTypes->Fill("Other", 1);}

    //fill histograms for only reaction of interest
    if(arr_Reaction == arr_ReactionPID[0]){
      //
      dHist_thrown_YstarInvariantMass->Fill(locYstarP4.M());
      dHist_thrown_YstarInvariantMass_wrong->Fill(locYstarP4_wrong.M());
      dHist_thrown_XiInvariantMass->Fill(locXiMinusP4.M());
      dHist_thrown_XiLifetimeRestFrame_postCL->Fill(locLifetimeRestFrameXi);	
	      
      dHist_thrown_KPlus1P3VsTheta->Fill((180/TMath::Pi())*(locKPlus1P4.Theta()),locKPlus1P4.P());
      dHist_thrown_KPlus1P3VsTheta_CM->Fill((180/TMath::Pi())*(locKPlus1P4_CM.Theta()),locKPlus1P4_CM.P());
      dHist_thrown_KPlus2P3VsTheta->Fill((180/TMath::Pi())*(locKPlus2P4.Theta()),locKPlus2P4.P());
      dHist_thrown_KPlus2P3VsTheta_CM->Fill((180/TMath::Pi())*(locKPlus2P4_CM.Theta()),locKPlus2P4_CM.P());
      dHist_thrown_YstarP3VsTheta->Fill((180/TMath::Pi())*(locYstarP4.Theta()),locYstarP4.P());
      dHist_thrown_YstarP3VsTheta_CM->Fill((180/TMath::Pi())*(locYstarP4_CM.Theta()),locYstarP4_CM.P());

      dHist_thrown_LambdaP3VsTheta->Fill(180/TMath::Pi()*locLambdaP4.Theta(),locLambdaP4.P());
      dHist_thrown_LambdaP3VsTheta_CM->Fill(180/TMath::Pi()*locDecayingLambdaP4_CM.Theta(),locDecayingLambdaP4_CM.P());
      dHist_thrown_LambdaP3VsTheta_HF->Fill(180/TMath::Pi()*Lambda_Angle_HF,locDecayingLambdaP4_HF.P());
      dHist_thrown_LambdaP3VsCosTheta_HF->Fill(Lambda_CosAngle_HF,locDecayingLambdaP4_HF.P());
	    
      dHist_thrown_PiMinus1P3VsTheta->Fill(180/TMath::Pi()*locPiMinus1P4.Theta(),locPiMinus1P4.P());
      dHist_thrown_PiMinus1P3VsTheta_HF->Fill(180/TMath::Pi()*PiMinus1_Angle_HF,locPiMinus1P4_HF.P());
      dHist_thrown_PiMinus1P3VsCosTheta_HF->Fill(PiMinus1_CosTheta_HF,locPiMinus1P4_HF.P());

      dHist_thrown_PiMinus2P3VsTheta->Fill(180/TMath::Pi()*locPiMinus2P4.Theta(),locPiMinus2P4.P());

      dHist_thrown_ProtonP3VsTheta->Fill(180/TMath::Pi()*locProtonP4.Theta(),locProtonP4.P());
    }
    
    //Fill only the correct generated reactions
    if(true)//(arr_Reaction == arr_ReactionPID[0] || arr_Reaction == arr_ReactionPID[1] || arr_Reaction == arr_ReactionPID[2]) && (locBeamP4.E() > 6.4 && locBeamP4.E() < 11.4))
      {
        //Fill flat trees with everything else to omit branchin fractions in cross section and have more stats when weighting
        /****************************************** FILL FLAT TREE (IF DESIRED) ******************************************/
        dFlatTreeInterface->Fill_TObject<TLorentzVector>("beam_p4", locBeamP4);
        dFlatTreeInterface->Fill_TObject<TLorentzVector>("kp1_p4", locKPlus1P4);
        dFlatTreeInterface->Fill_TObject<TLorentzVector>("kpfast_p4", locKPlusFastP4);    
        dFlatTreeInterface->Fill_TObject<TLorentzVector>("ystar_p4", locYstarP4);
        dFlatTreeInterface->Fill_TObject<TLorentzVector>("kp2_p4", locKPlus2P4);
        dFlatTreeInterface->Fill_TObject<TLorentzVector>("kpslow_p4", locKPlusSlowP4);
        //dFlatTreeInterface->Fill_TObject<TLorentzVector>("kp2_p4_ystar_hf", locKPlus2P4_Ystar_HF);
        dFlatTreeInterface->Fill_TObject<TLorentzVector>("decayxim_p4", locXiMinusP4);
        dFlatTreeInterface->Fill_TObject<TLorentzVector>("pim1_p4", locPiMinus1P4);    
        dFlatTreeInterface->Fill_TObject<TLorentzVector>("lambda_p4", locLambdaP4);
        dFlatTreeInterface->Fill_TObject<TLorentzVector>("pim2_p4", locPiMinus2P4);
        dFlatTreeInterface->Fill_TObject<TLorentzVector>("proton_p4", locProtonP4);    

        dFlatTreeInterface->Fill_TObject<TLorentzVector>("kp1_cm_p4", locKPlus1P4_CM);
          
        dFlatTreeInterface->Fill_Fundamental<Double_t>("kp2_costheta_hf", KPlus2_CosTheta_HF);
        dFlatTreeInterface->Fill_Fundamental<Double_t>("kp2_phi_hf", KPlus2_Phi_HF);
        dFlatTreeInterface->Fill_Fundamental<Double_t>("xim_costheta", XiMinus_CosTheta);
        dFlatTreeInterface->Fill_Fundamental<Double_t>("xim_costheta_hf", XiMinus_CosTheta_HF);
        dFlatTreeInterface->Fill_Fundamental<Double_t>("xim_costheta_gen_amp", XiMinus_CosTheta_YstarRest);
        dFlatTreeInterface->Fill_Fundamental<Double_t>("xim_phi_hf", XiMinus_Phi_HF);
        dFlatTreeInterface->Fill_Fundamental<Double_t>("pim1_costheta_hf",PiMinus1_CosTheta_HF);
        dFlatTreeInterface->Fill_Fundamental<Double_t>("xim_pathlen", locPathLengthXi);
        dFlatTreeInterface->Fill_Fundamental<Double_t>("xim_lifetime", locLifetimeXi);
        dFlatTreeInterface->Fill_Fundamental<Double_t>("xim_lifetime_restframe", locLifetimeRestFrameXi);
        dFlatTreeInterface->Fill_Fundamental<Double_t>("lambda_pathlen", locPathLengthLambda);
        dFlatTreeInterface->Fill_Fundamental<Double_t>("lambda_lifetime",locLifetimeLambda);
        dFlatTreeInterface->Fill_Fundamental<Double_t>("lambda_lifetime_restframe", locLifetimeRestFrameLambda);
        dFlatTreeInterface->Fill_Fundamental<Double_t>("beam_E", locBeamP4.E());
        dFlatTreeInterface->Fill_Fundamental<Double_t>("t_dist", -1.0*tval);
        dFlatTreeInterface->Fill_Fundamental<Double_t>("t_dist_fast", -1.0*tvalfast);
        dFlatTreeInterface->Fill_Fundamental<Double_t>("beam_vertexZ", locKPlus1X4.Z());
        dFlatTreeInterface->Fill_Fundamental<Double_t>("ystar_vertexZ", locYstarX4.Z());
        dFlatTreeInterface->Fill_Fundamental<Double_t>("xim_vertexZ", locPiMinus1X4.Z());
        dFlatTreeInterface->Fill_Fundamental<Double_t>("lambda_vertexZ", locPiMinus2X4.Z());
    
        Int_t main_pid = (arr_Reaction == arr_ReactionPID[0]) ? 1:0;
        dFlatTreeInterface->Fill_Fundamental<Int_t>("main_pid", main_pid);
        
        // dFlatTreeInterface->Fill_Fundamental<Double_t>("chisqndf", );
        // dFlatTreeInterface->Fill_Fundamental<Double_t>("confidencelvl");
        // dFlatTreeInterface->Fill_Fundamental<Double_t>("total_mm2");
        // dFlatTreeInterface->Fill_Fundamental<Double_t>("t_dist");

        // dFlatTreeInterface->Fill_Fundamental<Double_t>("beam_vertexX");
        // dFlatTreeInterface->Fill_Fundamental<Double_t>("beam_vertexY");
        // dFlatTreeInterface->Fill_Fundamental<Double_t>("beam_vertexZ");
        // dFlatTreeInterface->Fill_Fundamental<Double_t>("xim_vertexX");
        // dFlatTreeInterface->Fill_Fundamental<Double_t>("xim_vertexY");
        // dFlatTreeInterface->Fill_Fundamental<Double_t>("xim_vertexZ");
        // dFlatTreeInterface->Fill_Fundamental<Double_t>("xim_pathlen");
        // dFlatTreeInterface->Fill_Fundamental<Double_t>("xim_pathlensig");
                  
        // dFlatTreeInterface->Fill_Fundamental<Double_t>("lambda_vertexX");
        // dFlatTreeInterface->Fill_Fundamental<Double_t>("lambda_vertexY");
        // dFlatTreeInterface->Fill_Fundamental<Double_t>("lambda_vertexZ");
        // dFlatTreeInterface->Fill_Fundamental<Double_t>("lambda_pathlen");
        // dFlatTreeInterface->Fill_Fundamental<Double_t>("lambda_pathlensig");
        // dFlatTreeInterface->Fill_Fundamental<Double_t>("lambda_lifetime_restframe");
        Fill_FlatTree();
      }   
	
    //OR Manually:
	//BEWARE: Do not expect the particles to be at the same array indices from one event to the next!!!!
	//Why? Because while your channel may be the same, the pions/kaons/etc. will decay differently each event.

	//BRANCHES: https://halldweb.jlab.org/wiki/index.php/Analysis_TTreeFormat#TTree_Format:_Simulated_Data
	TClonesArray** locP4Array = dTreeInterface->Get_PointerToPointerTo_TClonesArray("Thrown__P4");
	TBranch* locPIDBranch = dTreeInterface->Get_Branch("Thrown__PID");
/*
	Particle_t locThrown1PID = PDGtoPType(((Int_t*)locPIDBranch->GetAddress())[0]);
	TLorentzVector locThrown1P4 = *((TLorentzVector*)(*locP4Array)->At(0));
	cout << "Particle 1: " << locThrown1PID << ", " << locThrown1P4.Px() << ", " << locThrown1P4.Py() << ", " << locThrown1P4.Pz() << ", " << locThrown1P4.E() << endl;
	Particle_t locThrown2PID = PDGtoPType(((Int_t*)locPIDBranch->GetAddress())[1]);
	TLorentzVector locThrown2P4 = *((TLorentzVector*)(*locP4Array)->At(1));
	cout << "Particle 2: " << locThrown2PID << ", " << locThrown2P4.Px() << ", " << locThrown2P4.Py() << ", " << locThrown2P4.Pz() << ", " << locThrown2P4.E() << endl;
*/


	/******************************************* BIN THROWN DATA INTO SEPARATE TREES FOR AMPTOOLS ***************************************/

/*
	//THESE KEYS MUST BE DEFINED IN THE INIT SECTION (along with the output file names)
	if((locBeamEnergyUsedForBinning >= 8.0) && (locBeamEnergyUsedForBinning < 9.0))
		Fill_OutputTree("Bin1"); //your user-defined key
	else if((locBeamEnergyUsedForBinning >= 9.0) && (locBeamEnergyUsedForBinning < 10.0))
		Fill_OutputTree("Bin2"); //your user-defined key
	else if((locBeamEnergyUsedForBinning >= 10.0) && (locBeamEnergyUsedForBinning < 11.0))
		Fill_OutputTree("Bin3"); //your user-defined key
*/

	return kTRUE;
}

void DSelector_thrown_kpkpxim::Finalize(void)
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
