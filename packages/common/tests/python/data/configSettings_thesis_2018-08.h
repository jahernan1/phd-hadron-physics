#ifndef CONFIGSETTINGS_H
#define CONFIGSETTINGS_H
#include <iostream>
#include <TMath.h> // needed for Long64_t type
using namespace std;

// ------------------------------------------------------
// DO NOT MODIFY THIS SECTION. GETS OVERWRITTEN BY RUN.PY
// DO NOT MODIFY THIS SECTION. GETS OVERWRITTEN BY RUN.PY
string cwd="@CWD@";
string rootFileLoc="@INPUT@";
string rootTreeName="flatTree_kpkpxim";
string fileTag="kpkpxim__B4_M23_2018-08_ana02_nominal_kphighrap_1111111";
string runTag="";
string s_fitWeight="hybrid_combo";
string s_sigWeight="hybrid_combo";
string s_altWeight="hybrid_combo";
string s_discrimVar="decayxim_M";
string s_extraVar="decayxim_M";
string s_neighborReqs="";
string s_phaseVar="beam_E;kp_highp_CosTheta;kp_highp_Phi;kplow_costheta_hf;kplow_phi_hf;pim1_costheta_hf;decaylamb_M_meas";
string standardizationType="range";
string alwaysSaveTheseEvents="";
bool saveMemUsuage=1;
int nProcess=16;
int kDim=200;
const int ckDim=200; // same as kDim but just of const int type
const int phaseSpaceDim=7;
const int discrimVarDim=1;
const int extraVarDim=1;
const int fitWeightsDim=1;
const int sigWeightsDim=1;
const int altWeightsDim=1;
bool redistributeBkgSigFits=0;
bool doKRandomNeighbors=0;
int numberEventsToSavePerProcess=-1;
int seedShift=1341;
Long64_t nentries=-1;
int nRndRepSubset=0;
int nBS=0;
bool saveBShistsAlso=0;
bool override_nentries=0;
bool saveEventLevelProcessSpeed=1;
bool saveBranchOfNeighbors=1;
bool saveMemUsage=1;
bool verbose_outputDistCalc=false;
// DO NOT MODIFY THIS SECTION. GETS OVERWRITTEN BY RUN.PY
// DO NOT MODIFY THIS SECTION. GETS OVERWRITTEN BY RUN.PY
// ------------------------------------------------------
//
#endif
