#ifndef GETSBWEIGHT_H
#define GETSBWEIGHT_H

#include <iostream>
#include "./auxilliary/drawPlots/drawPlots.h"
#include "configSettings.h"
#include "TCanvas.h"
#include <RooAbsReal.h>
#include <RooRealVar.h>
#include <RooDataSet.h>
#include <RooDataHist.h>
#include <RooGenericPdf.h>
#include <RooGaussian.h>
#include <RooJohnson.h>
#include <RooBernstein.h>
#include <RooChebychev.h>
#include <RooAddPdf.h>
#include <RooProdPdf.h>
#include <RooConstVar.h>
#include <RooArgList.h>
#include <RooPlot.h>
// gxana: #include <RooMinuit.h> dropped (unused; header removed in ROOT 6.40)
#include <RooFitResult.h>
// gxana: #include <RooChi2Var.h> dropped (unused; header removed in ROOT 6.40)
#include <RooWorkspace.h>
#include <RooMinimizer.h>
#include <RooClassFactory.h>
#include "RooTrace.h"
#include "RooArgSet.h"
#include "RooArgList.h"
//#include "RooAddPdf.h"
//#include "RooFormulaVar.h"

using namespace std;

class fitManager
{
  //////////////////////////////////////////////////////////////////////
  // 1. Need to define the values to initialize the fits with and the RooFit variables/PDFs
  //          - You might need to #include some header files that contains the PDFs you want. i.e. if you want to use RooVoigian then add
  //            #include <RooVoigian.h> at the top.
  // 2. Need to define a way to reinitialize the PDFs
  // 3. Need to define a way to calculate the q-factor
  // 4. Need to define how you want to insert the data into the dataset
  // 5. (DEFAULTS PROBABLY FINE) Need to define how you want to fit the PDFs to the dataset
  // ----------------------     EXTRA     ----------------------------
  // HOW TO MAKE YOUR OWN CUSTOM PDFs : MUCH FASTER THAN ROOGENERICPDF
  //    THERE IS A README FOR AN EXAMPLE BIVARIATE GAUSSIAN CUSTOM PDF AT auxilliary/customPDFs/bivariateGaus/README 
  // 1. Use RooClassFactory to create a custom PDF class. Should create a .so file
  // 2. Include the header file here
  // 3. include something like -pathToSoFile when you compile the main program
  // -----------------------------------------------------------------
  //////////////////////////////////////////////////////////////////////
 public:
  // DEFINE VALUES TO INITIALIZE ROOFIT VARIABLES WITH
  float initMuX = 1.322;
  float initLambdaX = 0.006;
  float initGammaX = 0.0;
  float initDeltaX = 1.5;
  float initSigmaMean = 1.387;
  float initSigmaSig = 0.0394;
  float initWidthSig = 0.006;
  float initChebA = 0.08;
  float initChebB = -0.1;
  std::vector<float> fitRangeX={1.28,1.45};

  ///////////////////////////////
  // DEFINE VARIABLES FOR DATASET
  ///////////////////////////////
  float eff_nentries;
  float sigFrac;
  float NLL;
  int fitStatus;
  RooRealVar* x;
  RooRealVar* w;
  RooDataSet* rooData;
  ////////////////////////
  // DEFINE THE SIGNAL PDF
  ////////////////////////
  RooRealVar* mux;
  RooRealVar* lambdax;
  RooRealVar* gammax;
  RooRealVar* deltax;
  RooJohnson* rooSig;
  ////////////////////////
  // DEFINE THE BKG PDF
  ////////////////////////
  RooRealVar* sigma_mean;
  RooRealVar* sigma_sig;
  RooRealVar* sigma_width;
  RooGaussian* rooSigmaBkg;
  RooRealVar* cheb_parA;
  RooRealVar* cheb_parB;
  RooChebychev* rooBkg;
  ////////////////////////
  // DEFINE THE TOTAL PDF
  ////////////////////////
  RooRealVar* nsig;
  RooRealVar* nsigmabkg;
  RooRealVar* nbkg;

  RooAddPdf* rooSigBkg;
        
  fitManager(string iProcess){ 
    /////////////// VARIABLES
    x = new RooRealVar{("x"+iProcess).c_str(),"Mass GeV",(fitRangeX[1]-fitRangeX[0])/2+fitRangeX[0],fitRangeX[0],fitRangeX[1]};
    x->setRange(("roo_fitRangeXim_"+iProcess).c_str(),fitRangeX[0], fitRangeX[1]);
    x->setBins(50);
    w = new RooRealVar{("w"+iProcess).c_str(), "Weight", 0, -10, 10}; // Weights can take a wide range
    rooData = new RooDataSet{("rooData"+iProcess).c_str(),"rooData",RooArgSet(*x,*w),RooFit::WeightVar(*w)};
    /////////////// FOR SIGNAL PDF
    mux = new RooRealVar{("mux"+iProcess).c_str(),"mux",initMuX, initMuX-initLambdaX, initMuX+initLambdaX};
    lambdax = new RooRealVar{("lambdax"+iProcess).c_str(),"lambdax",initLambdaX,0.005, 0.007};
    gammax = new RooRealVar{("gammax"+iProcess).c_str(),"gammax",initGammaX};
    deltax = new RooRealVar{("deltax"+iProcess).c_str(),"deltax",initDeltaX,1,2};
    rooSig = new RooJohnson{("rooJohnsonXiMinus_"+iProcess).c_str(), "rooJohnsonXiMinus", *x, *mux, *lambdax, *gammax, *deltax};
    /////////////// FOR BKG PDF
    sigma_mean = new RooRealVar{("sigma_mean"+iProcess).c_str(),"sigma_mean",initSigmaMean, 1.386, 1.388};
    sigma_sig = new RooRealVar{("sigma_sig"+iProcess).c_str(),"sigma_sig",initSigmaSig, 0.01, 0.05};
    cheb_parA = new RooRealVar{("cheb_parA"+iProcess).c_str(),"cheb_parA",initChebA,0.1,2};
    //cheb_parB = new RooRealVar{("cheb_parB"+iProcess).c_str(),"cheb_parB",initChebB,-2,-1e-4};
    rooSigmaBkg = new RooGaussian{("rooSigmaBkg"+iProcess).c_str(), "rooSigmaBkg", *x, *sigma_mean, *sigma_sig};
    rooBkg = new RooChebychev{("rooBkg"+iProcess).c_str(), "rooBkg", *x, RooArgList(*cheb_parA)};//,*cheb_parB)};
    /////////////// FOR YIELDS
    nsig = new RooRealVar{("nsig"+iProcess).c_str(),"nsig",(float)kDim/10,0,(float)kDim};
    nbkg = new RooRealVar{("nbkg"+iProcess).c_str(),"nbkg",(float)kDim/2,1,(float)kDim};
    nsigmabkg = new RooRealVar{("nsigmabkg"+iProcess).c_str(),"nsigmabkg",(float)kDim/10,0,(float)kDim};
    /////////////// CREATING FINAL PDFS
    rooSigBkg = new RooAddPdf{("rooSumPdf"+iProcess).c_str(), "rooSumPdf", RooArgList(*rooSig,*rooBkg,*rooSigmaBkg),RooArgSet(*nsig,*nbkg,*rooSigmaBkg)};
  }

  void reinitialize(float initSigFrac){
    mux->setVal(initMuX);
    lambdax->setVal(initLambdaX);
    gammax->setVal(initGammaX);
    deltax->setVal(initDeltaX);
    sigma_mean->setVal(initSigmaMean);
    sigma_sig->setVal(initSigmaSig);
    cheb_parA->setVal(initChebA);
    //cheb_parB->setVal(initChebB);
    eff_nentries = rooData->sumEntries();
    nsig->setVal(initSigFrac*eff_nentries);
    nbkg->setVal((1-initSigFrac)*eff_nentries);
  }

  float calculate_q(float* vals){
    float valX = vals[0];
    float qvalue;
    x->setVal(valX);
    sigFrac = nsig->getVal()/(nsig->getVal()+nbkg->getVal()+nsigmabkg->getVal());
    RooArgSet normSet(*x); // gxana: named normalisation set; ROOT 6.40 rejects getVal(RooArgSet(...)) temporaries at run time
    float sigPdfVal = sigFrac*rooSig->getVal(normSet);
    float bkgPdfVal = (1-sigFrac)*(rooBkg->getVal(normSet)+rooSigmaBkg->getVal(normSet));
    float sigPlusBkgPdfVal = sigPdfVal+bkgPdfVal;
    float totPdfVal = rooSigBkg->getVal(normSet);

    if ((sigPdfVal==0)*(bkgPdfVal==0)){
      qvalue=0;    
      //PDFs have been seen to return 0s out here so qvalue is undefined
      cout << "PDF values are all 0, qvalue will be nan. SET TO ZERO" << endl;
    }
    else{
      qvalue = sigPdfVal/(sigPdfVal+bkgPdfVal);
                cout << "\tpostFit(Q=" << qvalue << ")(Status=" << fitStatus << ")(NLL=" << NLL << ") - sigFrac: " << sigFrac << " || sigPdfVal: " << sigPdfVal << " || bkgPdfVal: " << bkgPdfVal 
                     << " || sigPdfVal+bkgPdfVal: " << sigPlusBkgPdfVal << " || totPdfVal: " << totPdfVal 
                     << " || valY: " << valX << endl;
    }
            
    return qvalue;
  }
  float calculate_q(float valX, float valY){} // another signature for 2D fits

  bool insert(float* vals, float weight){
    // with 600 neighbors it seems like the fitTo command takes ~2x longer when using Range() argument which selects the fit range.
    // This is equivalent to shrinking the dataset range and fitting over the full range which will save time.
    // It might be useful to think of setting a fit range as to lower the effective number of nearest neighbors. Another thing that lowers
    // the effective number of neighbors is any weights we apply to the filling of the histograms
    float valX = vals[0];
    bool keptNeighbor = ( valX > fitRangeX[0] && 
                                  valX < fitRangeX[1]
			  );
    if (keptNeighbor){
      x->setVal(valX);
      w->setVal(weight);
      // w will get overwritten here when adding to RooDataSet but actually does not pick up the value. So we cannot use 
      // w.getVal() but the dataset will be weighted: https://root-forum.cern.ch/t/fit-to-a-weighted-unbinned-data-set/33495
      rooData->add(RooArgSet(*x,*w),weight);
    }
    return keptNeighbor;
  }

  RooArgSet* getParameters(){ 
    // Grabs the parameters of the fig function. Your discriminating variables are not parameters
    return rooSigBkg->getParameters(RooArgList(*x));
  }

  float fit(){
    // Looked in multiple places and Moneta always says you cant use Minos with weighted data but should just use SumW2Errors. If I dont
    //   use Minos then nsig+nbkg doesnt even sum to the original entries...
    //   lets not use minos for now since it significantly speeds things up. Bootstrapping might help alleviate this problem
    //   https://root-forum.cern.ch/t/parameter-uncertainties-by-rooabspdf-fitto-for-weighted-data-depend-on-minos-option-if-sumw2error-is-set-too/39599/4
    // SumW2Error - errors calculated from the inverse hessian does not give good coverage for weighted data. Setting to false is better for weighted I think? 
    // AsymptoticError - correct in the large N limit
    //   https://root.cern.ch/doc/master/rf611__weightedfits_8C.html
    RooFitResult* roo_result = rooSigBkg->fitTo(*rooData, 
						RooFit::Save(), 
						RooFit::Minimizer("Minuit","migrad"), // gxana: pin the ROOT 6.24 default minimizer (spec D25)
						RooFit::PrintLevel(-1), 
						//RooFit::BatchMode(true), 
						RooFit::SumW2Error(true),//false),
						//RooFit::AsymptoticError(true), 
						RooFit::Hesse(kFALSE)
						//RooFit::Minos(kTRUE)
						);
    NLL = roo_result->minNll();
    fitStatus = roo_result->status();
    delete roo_result;
    return NLL;
  }

  void drawFitPlots(float* vals, int ientry, TH1* dHist_qvaluesBS, double best_qvalue, float* chisqndf, int iBS, TCanvas* c, TFile* qHistsFile){ // gxana: chi2 argument required by the fork engine
    // vect contains the discriminating variables (i.e. Mpi0 and Meta)
    // ientry is the entry in the root tree we are trying to calculate the q-value for. Used just for naming
    // dHist_qvaluesBS is the histogram of the boostrapped qvalues
    // best_qvalue is taken from the best fitted qvalue if we do multiple fits per entry
    // iBS is the current bootstrap iteration. Useful if we wanted to see how each bootstrap iteration looks like
    // qHistsFile is the TFile we will save to
    float valX = vals[0];
    draw1DPlots(x, valX, ientry, eff_nentries,
		rooSigBkg, rooBkg, rooSig, rooData, sigFrac, NLL, chisqndf,
		dHist_qvaluesBS, best_qvalue, iBS, c, qHistsFile);
  }
};

#endif
