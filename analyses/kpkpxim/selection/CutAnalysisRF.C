#include "gxana/common/Paths.h"
#include "gxana/common/Style.h"
#include "TF1.h"
#include "TH1.h"
#include "TFile.h"
#include "TCanvas.h"
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <math.h>
#include <string.h>
#include "TMath.h"
#include "RooPlot.h"
#include "RooMsgService.h"
#include "RooRealVar.h"
#include "RooDataHist.h"
using namespace RooFit;

void rooFitHist(TH1* hist, string histTitle, double* sigYield, double* sigYieldErr, double* bkgYield, double* bkgYieldErr);
TH1* AccSubTH1(string rootFilePath, string histName);
void plotRatio(string plotName, string dataSetName, string plotTitle, vector<double> arr_yield, double cutVal=8);
void setStyle();
void GetFilterHist(TH2 *hist, string root_file_path, vector<double> vec_cutVal, string cutName="chisqndf",int n_threads = 16);
void GetCutAnalysis(vector<double> vec_CutParamBin, string rootFileName, vector<double> vec_cutVal, string cutNameDelim="chisqndf", string cutPlotName="#chi^{2}_{#nu}");

int CutAnalysisRF()
{
    //
    vector<string> rootFile = {"flatTree_kpkpxim__M23_2017-01_ana56", "flatTree_kpkpxim__B4_M23_2018-01_ana03", "flatTree_kpkpxim__B4_M23_2018-08_ana02","kpkpxim_2018-08_ana-05_LE","kpkpxim_2019-11_ana-04_b05_b12"};
    vector<double> chiSqNdfBins{24, 0, 12, 4};//0=numbins, 1=minCutVal, 2=maxCutVal, 3=minBinNum  
    vector<double> mm2Bins{24, 0, 0.048, 1};
    vector<double> xifsBins{200, 0, 50, 4};
    vector<double> vec_cutVal{8,0.02,2};
    vector<double> vec_cutVal201801{8,0.02,2};
  
    //run cut analysis
    for(int loc_i=0; loc_i < 3; loc_i++)
        {
            setStyle();
            GetCutAnalysis(chiSqNdfBins, rootFile[loc_i], vec_cutVal);
            setStyle();
            GetCutAnalysis(mm2Bins, rootFile[loc_i], vec_cutVal , "total_mm2", "#left|p^{#mu}_{MM_{X}}#right|^{2} #lower[0.15]{(GeV^{2} )}" );
        }

    return 0;
}

void GetCutAnalysis(vector<double> vec_CutParamBin, string rootFileName, vector<double> vec_cutVal, string cutNameDelim="chisqndf", string cutPlotName="#chi^{2}_{#nu}")
{// vec_CutParamBins: 0=numbins, 1=minCutVal, 2=maxCutVal, 3=minBinNum  
    string thisDir = gxana::EnvPath("GXANA_OUTPUT", "kpkpxim/cut_analysis_plots/");  
    vector<TH1*> hist_XiMass;
    vector<double> ratioFOM;
    vector<double> ratioFOMErr;
    vector<double> ratioSB;
    vector<double> ratioSBErr;
    vector<double> arr_yield;
    FILE *fCutFOM = fopen((thisDir+"data/"+cutNameDelim+"Cut_FOM_"+rootFileName+".txt").c_str(), "w+");
    FILE *fCutSB = fopen((thisDir+"data/"+cutNameDelim+"Cut_SB_"+rootFileName+".txt").c_str(), "w+");
    FILE *fCutYield = fopen((thisDir+"data/"+cutNameDelim+"Cut_Yield_"+ rootFileName+".txt").c_str(), "w+");
    int cnt = 0;
    TH2* hist_XiMass_Cut = new TH2D("",(";M(#Lambda#pi^{-}) (GeV);"+cutPlotName).c_str(),100,1.25,1.45, vec_CutParamBin[0],vec_CutParamBin[1],vec_CutParamBin[2]);
    
    //Get the histogram with cuts
    GetFilterHist(hist_XiMass_Cut, rootFileName.c_str(), vec_cutVal, cutNameDelim);
    double numBins = hist_XiMass_Cut->GetNbinsY()-vec_CutParamBin[3];
    
    RooMsgService::instance().setGlobalKillBelow(RooFit::ERROR);
    TCanvas* cut_Can = new TCanvas( ("XiMinus_"+cutNameDelim+"Cut").c_str(), (cutNameDelim+"Cut Fits").c_str(), 800, 1100);// Plot Fits;
    //cut_Can->DivideSquare(numBins,1e-4,1e-4);
    cut_Can->Divide(4,std::ceil(numBins/4),1e-4,1e-4);
    //XiMinusInvariantMass_ChiSqNdf->Draw("e1");
  
    for(int bin_i = int(vec_CutParamBin[3]); bin_i < hist_XiMass_Cut->GetNbinsY()+1; ++bin_i )
        //for(int bin_i = int(vec_CutParamBin[3]); bin_i < 20; bin_i++ )
        {
            double sigYield, sigYieldErr, bkgYield, bkgYieldErr;
            double cut_val = hist_XiMass_Cut->GetYaxis()->GetBinCenter( bin_i ) + hist_XiMass_Cut->GetYaxis()->GetBinWidth(bin_i)/2 ;
      
            // Get mass projection for cut
            hist_XiMass.push_back(hist_XiMass_Cut->ProjectionX(("_px_"+rootFileName+"_"+to_string(cut_val)).c_str(),1,bin_i,"e"));
            //hist_XiMass.push_back(hist_XiMass_Cut->ProjectionX(("_px_"+rootFileName+"_"+to_string(cut_val)).c_str(),bin_i,numBins,"e"));
      
            cut_Can->cd(cnt+1);
            cut_Can->SetLeftMargin(0.07);
            cut_Can->SetRightMargin(0.07);
           
            // Fit mass distribution
            if(cutNameDelim=="chisqndf")
                rooFitHist(hist_XiMass[cnt], (cutPlotName +" < " + to_string(cut_val).substr(0, to_string(cut_val).find_last_not_of('0')+1)).c_str(), &sigYield, &sigYieldErr, &bkgYield, &bkgYieldErr);
            else
                rooFitHist(hist_XiMass[cnt], ("|MM_{#it{X}}|^{2} < " + to_string(cut_val).substr(0, to_string(cut_val).find_last_not_of('0')+1)).c_str(), &sigYield, &sigYieldErr, &bkgYield, &bkgYieldErr);
            
            cout << "Fit Results: " << cut_val << "\t" << sigYield << "\t" << bkgYield << "\n" << endl;

            ratioFOM.push_back( sigYield / sqrt (sigYield +  bkgYield));
            ratioSB.push_back(sigYield /  bkgYield);
            //ratioSBErr.push_back( ratioSB[bin_i] * sqrt( pow( sigYieldErr / sigYield, 2 ) + pow( bkgYieldErr / bkgYield, 2 ) ));
            arr_yield.push_back(sigYield);
      
            // Record significance values
            fprintf(fCutFOM, "%f \t %f \n", cut_val, ratioFOM[cnt]);//, ratioSBErr[i]);
            fprintf(fCutSB, "%f \t %f \n", cut_val, ratioSB[cnt]);//, ratioSBErr[i]);
            fprintf(fCutYield, "%f \t %f \n", cut_val, sigYield);//, sigYieldErr);//, ratioSBErr[i]);
            cnt=cnt+1;
        }
  
    // Close Files
    fclose(fCutFOM);
    fclose(fCutSB);
    fclose(fCutYield);

    // Save Fits
    cut_Can->SaveAs((thisDir+"results/"+"XiMinus_"+cutNameDelim+"Cut_Fits_"+ rootFileName +"_kphighrap.pdf").c_str());
    cut_Can->Close();
    // Plot FOM and SB
    gStyle->SetPadBottomMargin(0.20);
    gStyle->SetPadTopMargin   (0.03);
    gStyle->SetPadLeftMargin  (0.18);
    gStyle->SetPadRightMargin (0.16);
    gStyle->SetTitleOffset(0.98,"Y");
  
    if(cutNameDelim=="chisqndf")
        plotRatio(cutNameDelim+"Cut", rootFileName, " ;"+cutPlotName +"; N_{S}#scale[1.6]{/}#sqrt{N_{S}+N_{B}}", arr_yield, vec_cutVal[0]);
    else if(cutNameDelim=="total_mm2")
        plotRatio(cutNameDelim+"Cut", rootFileName, " ;"+cutPlotName +"; N_{S}#scale[1.6]{/}#sqrt{N_{S}+N_{B}}", arr_yield, vec_cutVal[1]);
}

void GetFilterHist(TH2 *hist, string root_file_name, vector<double> vec_cutVal, string cutName="chisqndf", int n_threads = 4)
{
    // initalize some cut strings
    string str_chiSqCut = "chisqndf < " + to_string( vec_cutVal[0] );
    string str_mm2Cut = "abs(total_mm2) < " + to_string( vec_cutVal[1] );
    string str_ximPathCut = "xim_pathlensig > " + to_string( vec_cutVal[2] ); 
 	// Parallelize with n threads
    if(n_threads > 0.0)	ROOT::EnableImplicitMT(n_threads);
	
	// Branches you want to use get
	std::vector<std::string> branches = {"decayxim_M", "kplow_p4", "kphigh_p4",
        "xim_pathlensig", "lambda_pathlensig",
        "chisqndf","total_mm2",
        "beam_E","beam_vertexZ",
        "best_combo_rf","acc_weight"
    };
    
	// make data frame
	// format : tree name, file name, branches to open
    auto df = ROOT::RDataFrame("flatTree_kpkpxim", (gxana::EnvPath("GXANA_DATA", "Trees/flatTree/rawTrees/")+root_file_name+".root").c_str())
        .Define("hybrid_combo","best_combo_rf*acc_weight")
        .Define("kphigh_prapidity","kphigh_p4.Rapidity()")
        .Filter("beam_E > 6.4 && beam_E < 11.4");
        //
    auto df1 = df.Filter(str_mm2Cut.c_str(), str_mm2Cut.c_str())
        .Filter("beam_vertexZ > 50.4 && beam_vertexZ < 79.1","targetZCut")
        .Filter(str_ximPathCut.c_str(), str_ximPathCut.c_str() )
        .Filter("lambda_pathlensig > 0", "LambPathLenSigCut>0")
        .Filter("kphigh_p4.Rapidity()>2","KpHighRapidityCut");
                
    auto df2 = df.Define("total_mm22", [](double mm2){return TMath::Abs(mm2);}, {"total_mm2"})
        .Filter("chisqndf < 15")
        .Filter("beam_vertexZ > 50.4 && beam_vertexZ < 79.1","targetZCut")
        .Filter(str_ximPathCut.c_str(), str_ximPathCut.c_str() )
        .Filter("lambda_pathlensig > 0", "LambPathLenSigCut>0")
        .Filter("kphigh_p4.Rapidity()>2","KpHighRapidityCut");
                 
    if(cutName=="chisqndf")
        df1.Foreach([&hist](double mass, double cut, double weight){hist->Fill(mass, cut, weight);},{"decayxim_M", cutName.c_str(), "hybrid_combo"});
    else if(cutName=="total_mm2")
        df2.Foreach([&hist](double mass, double cut, double weight){hist->Fill(mass, cut, weight);},{"decayxim_M", "total_mm22", "hybrid_combo"});
}

void plotRatio(string plotName, string dataSetName, string plotTitle, vector<double> arr_yield, double cutVal=8)
{
    string thisDir = gxana::EnvPath("GXANA_OUTPUT", "kpkpxim/cut_analysis_plots/");
    string analysisDir = gxana::EnvPath("GXANA_OUTPUT", "kpkpxim/analysis/event_selection/chisqndf_cut/");
    TCanvas* c = new TCanvas(plotName.c_str(), plotName.c_str() );
    TPad *pad1 = new TPad((plotName+"_Pad1").c_str(),"",0,0,1,1);
    TPad *pad2 = new TPad((plotName+"_Pad2").c_str(),"",0,0,1,1);
    pad2->SetFillStyle(4000); //will be transparent
    pad2->SetFrameFillStyle(0);
    //pad2->SetFrameLineColor(kRed+1);
    //pad2->GetFrame()->SetLineWidth(10);
    pad1->SetGrid();
    //set pad
  
    TGraphErrors *g1 = new TGraphErrors( (thisDir+"data/"+plotName+"_FOM_"+dataSetName+".txt").c_str(), "%lg %lg");//, option=" \t,;");
    g1->SetTitle(plotTitle.c_str());
    g1->SetMarkerStyle(20);
    g1->SetMarkerSize(1.4);
    g1->SetDrawOption("APL");
    g1->SetMarkerColor(kBlack);
    g1->SetMarkerStyle(24);
    g1->SetLineWidth(2);
    g1->SetFillStyle(0);
    //g1->GetXaxis()->SetNdivisions(510);
    //gStyle->SetPadTickX(1);
    pad1->Draw();
    pad1->cd();
    g1->Draw("APL");
  
    TGraphErrors *g2 = new TGraphErrors( (thisDir+"data/"+plotName+"_SB_"+dataSetName+".txt").c_str(), "%lg %lg");//, option=" \t,;");
    g2->SetTitle(" ; ; N_{S}/N_{B}");
    g2->SetMarkerStyle(20);
    g2->SetMarkerSize(1.4);
    g2->SetDrawOption("APLY+");
    g2->SetMarkerColor(kRed+1);
    g2->SetLineColor(kRed+1);
    g2->SetLineWidth(2);
    g2->SetFillStyle(0);  
    g2->GetYaxis()->SetAxisColor(kRed+1);
    //g2->GetYaxis()->SetMaxDigits(3);
    g2->GetYaxis()->SetTitleOffset(0.85);
    g2->GetYaxis()->SetTitleColor(kRed+1);
    g2->GetYaxis()->SetLabelColor(kRed+1);
    pad2->Draw();
    pad2->cd();
    g2->Draw("APLY+");

    c->Update();
    //double ypadmax = pad2->PixeltoY(UtoPixel(pad2->GetUymax()));
    double ypadmin,ypadmax,xpadmin,xpadmax;
    pad2->GetRangeAxis(xpadmin,ypadmin,xpadmax,ypadmax);
    cout << "Pad Values: " << xpadmin << "\t "<< xpadmax << "\t" << ypadmin << "\t" << ypadmax << endl;
    TLine* cutLine = new TLine(cutVal, ypadmin, cutVal, ypadmax);
    cutLine->SetLineWidth(4);
    cutLine->SetLineStyle(10);
    cutLine->SetLineColor(kBlue+1);
    cutLine->Draw();
  
    TArrow *arCut = new TArrow(cutVal,(ypadmax+ypadmin)/2,cutVal*0.5,(ypadmax+ypadmin)/2, 0.05,"|>");
    //ar4->SetAngle(60);
    arCut->SetLineWidth(4);
    arCut->SetFillColor(kBlue+1);
    arCut->SetLineColor(kBlue+1);
    arCut->Draw();
    // Save Plot
    c->Update();
    c->SaveAs((thisDir+"results/"+plotName+"_"+dataSetName+"_kphighrap.pdf").c_str());
    c->SaveAs((analysisDir+plotName+"_"+dataSetName+"_kphighrap.pdf").c_str());
}

void rooFitHist(TH1* hist, string histTitle, double* sigYield, double* sigYieldErr, double* bkgYield, double* bkgYieldErr)
{
    Double_t min_mass = hist->GetXaxis()->GetBinLowEdge(hist->FindFirstBinAbove(0,1,5, hist->FindBin(1.3)));
    if(min_mass < 1.28) min_mass = 1.28;
  
    RooWorkspace* w = new RooWorkspace(histTitle.c_str());
    RooRealVar mass("mass", "M(#Lambda#pi^{-}) (GeV/c^{2})", 1.28, 1.45);
    RooDataHist *data = new RooDataHist("data", "Dataset of mass", mass, hist);
    RooPlot* massframe = mass.frame(RooFit::Title(histTitle.c_str()));
    w->import(RooArgSet(mass));
    //Style the histogram
    massframe->GetYaxis()->SetMaxDigits(3);
    massframe->SetNdivisions(505);
    
    //Define pdfs
    w->factory("Chebychev::bkgd(mass,{a0[0.8,0.1,1.5],a1[-0.2,-1,-0.1]})");//,a1[-0.1,-2,-1e-2]
    //w->factory("Voigtian::sigma(mass,mean[1.385,1.383,1.388],sig[0.0055. 0.004, 0.006], width[0.015, 0.01, 0.042])");
    //w->factory("Gaussian::sigma(mass,mean[1.385,1.383,1.390],sig[0.019,0.018,0.022])");
    w->factory("Johnson::xigaus(mass,mu[1.3217,1.32,1.33],lambda[0.0055,0.004,0.006], gamma[0], delta[1.3,1.,2.])");
    //w->factory("Gaussian::xigaus(mass,mean_xi[1.322,1.31,1.33],sigma_xi[0.0055,0.004,0.008])");
    
    //Create model and fit to data
    w->factory("SUM::model(  nbkgd[2000,1,1e6]*bkgd, nxi[1000,1,1e6]*xigaus)");//nsigma[300,1,1e6]*sigma,
    w->pdf("model")->fitTo(*data,Extended(true),SumW2Error(true),PrintLevel(-1),PrintEvalErrors(-1),Verbose(false),Warnings(false));
    //Plot params and data and fit
    data->plotOn(massframe,MarkerStyle(24),MarkerSize(0.4));
    w->pdf("model")->paramOn(massframe, Format("N",AutoPrecision(0)), Layout(0.45, 0.9, 0.85) ,Parameters(RooArgSet(*w->var("nxi"), *w->var("mu"), *w->var("nbkgd"))));// *w->var("nsigma"), *w->var("mean"), *w->var("sig")
    massframe->getAttText()->SetTextSize(0.08) ;
    massframe->getAttFill()->SetFillStyle(0) ;
    massframe->getAttLine()->SetLineColor(0) ;
    w->pdf("model")->plotOn(massframe, LineWidth(1));
    w->pdf("xigaus")->plotOn(massframe,LineWidth(1), LineStyle(kDotted), Normalization(w->var("nxi")->getVal(), RooAbsReal::NumEvent));
    //w->pdf("sigma")->plotOn(massframe, LineWidth(1), LineStyle(kDotted), Normalization(w->var("nsigma")->getVal(), RooAbsReal::NumEvent));
    w->pdf("bkgd")->plotOn(massframe,LineWidth(1), LineStyle(kDotted), Normalization(w->var("nbkgd")->getVal(), RooAbsReal::NumEvent));
  
    //Get background under signal region
    double xiMu = w->var("mu")->getVal();
    double xiLambda = w->var("lambda")->getVal();
    double xiDelta = w->var("delta")->getVal();
    double xiGamma = w->var("gamma")->getVal();
    double xiMean = xiMu - xiLambda*exp(1 / (2*pow(xiDelta,2)) )*sinh(xiGamma/xiDelta);
    double xiSigma = sqrt( pow(xiLambda, 2)/2*(exp(pow(xiDelta, -2) ) - 1 )*(exp(pow(xiDelta, -2) )*cosh(2*xiGamma/xiDelta )+1));
    //For Gaussian signal
    // double xiMean = w->var("mean_xi")->getVal();
    // double xiSigma = w->var("sigma_xi")->getVal();
  
    printf("Mean and Sigma: %f, %f \n", xiMean, xiSigma);
    double xCutL = xiMean - 2*xiSigma;
    double xCutR = xiMean + 2*xiSigma;
    w->var("mass")->setRange("signal", xCutL, xCutR);
  
    RooAbsReal* sig_sig = w->pdf("xigaus")->createIntegral(mass, RooFit::NormSet(mass), RooFit::Range("signal"));
    printf("Signal Fraction in 2Sigma Window: %f \n", sig_sig->getVal());
    printf("Signal Yield in 2Sigma Window: %f \n", w->var("nxi")->getVal()*sig_sig->getVal());

    *sigYield = w->var("nxi")->getVal()*sig_sig->getVal();
    *sigYieldErr = w->var("nxi")->getError()*sig_sig->getVal();
  
    RooAbsReal* bkg_sig = w->pdf("bkgd")->createIntegral(mass, RooFit::NormSet(mass), RooFit::Range("signal"));
    //RooAbsReal* bkg_sigma = w->pdf("sigma")->createIntegral(mass, RooFit::NormSet(mass), RooFit::Range("signal"));
    printf("Background Fraction of 2Sigma Window: %f \n", bkg_sig->getVal());//,bkg_sigma->getVal());
    printf("Background Yield in 2Sigma Window: %f \n", w->var("nbkgd")->getVal()*bkg_sig->getVal());//+w->var("nsigma")->getVal()*bkg_sigma->getVal());
  
    *bkgYield = w->var("nbkgd")->getVal()*bkg_sig->getVal();//+w->var("nsigma")->getVal()*bkg_sigma->getVal();
    *bkgYieldErr = w->var("nbkgd")->getError()*bkg_sig->getVal();

    //Draw cut lines for signal region
    //massframe->SetMaximum(massframe->GetMaximum()+100);
    double ypadmax = massframe->GetMaximum();
    TLine* cutLineL = new TLine(xCutL, 0.0, xCutL, ypadmax);
    cutLineL->SetLineWidth(1);
    cutLineL->SetLineColor(kRed+1);
    massframe->addObject(cutLineL, " ");
    TLine* cutLineR = new TLine(xCutR, 0.0, xCutR, ypadmax);
    cutLineR->SetLineWidth(1);
    cutLineR->SetLineColor(kRed+1);
    massframe->addObject(cutLineR, " ");

    massframe->Draw();
    massframe->SetMarkerStyle(24);
    massframe->SetMarkerSize(0.2);
}

// input specific style formatting of user choice
void setStyle()
{
    gxana::StyleParams p = gxana::ComparisonStyle();
    p.canvasDefH = 700;
    p.canvasDefW = 800;
    p.padBottomMargin = 0.17;
    p.padTopMargin = 0.11;
    p.padLeftMargin = 0.2;
    p.markerSize = 1.0;
    p.markerStyle = 24;
    p.labelSizeX = 0.055;
    p.labelSizeY = 0.055;
    p.titleX = 0.95;
    p.titleSizeT = 0.07;
    p.titleOffsetX = 0.9;
    p.titleOffsetY = 1.2;
    p.textSize = 0.09;
    gxana::ApplyStyle(p);
}
