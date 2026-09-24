#include "gxana/common/Paths.h"
void RooFitHist(TH1* hist,  const char* histTitle);
using namespace RooFit;

void MakeXim1820_IM() {
    //Get histogram 
    TFile* tf = TFile::Open(gxana::EnvPath("GXANA_DATA", "KpKpKmL012017012018082018Real_31July.root").c_str());
    TH1F *hist = (TH1F*)tf->Get("March04Histograms/XiStarTBin0_Prompt")->Clone("");
    TH1F *hist_acc = (TH1F*)tf->Get("March04Histograms/XiStarTBin0_Accidental")->Clone("");
    //Perform accidental subtraction
    TH1F *hist_accsub = (TH1F*)hist->Clone("");
    hist_accsub->Add(hist_acc, -0.16666666666);
    hist_accsub->RebinX(3);
    double binWidth = hist_accsub->GetXaxis()->GetBinWidth(1);
    char histTitle[100];
    sprintf(histTitle, " ;M(#LambdaK^{-}) (GeV); Events / %.0f MeV", binWidth*1000);
    
    gStyle->SetOptFit(1);
    //hist_accsub->GetXaxis()->CenterTitle(true);
    //hist_accsub->GetYaxis()->CenterTitle(true);
    hist_accsub->GetXaxis()->SetTitleSize(0.065);
    hist_accsub->GetYaxis()->SetTitleSize(0.065);
    hist_accsub->GetXaxis()->SetLabelSize(0.05);
    hist_accsub->GetYaxis()->SetLabelSize(0.05);
    hist_accsub->GetXaxis()->SetTitleOffset(0.88);
    hist_accsub->GetYaxis()->SetTitleOffset(0.73);
    hist_accsub->GetYaxis()->SetNdivisions(510);
    hist_accsub->GetYaxis()->SetMaxDigits(2);
    hist_accsub->SetMarkerStyle(24);
    hist_accsub->SetMarkerColor(kBlue);
    hist_accsub->SetLineColor(kBlue);
    hist_accsub->SetMarkerSize(0.9);

    RooFitHist(hist_accsub,histTitle);
    // Draw options
    // TCanvas *c = new TCanvas("","",700,500);
    // c->SetFrameBorderMode(0);
    // c->SetFrameLineWidth(0);
    // c->SetBottomMargin(0.125);
    // gPad->SetRightMargin(0.025);
    // gPad->SetFrameLineWidth(0);
    // hist_accsub->Draw("e1");
        
    // TLatex latex;
    // latex.SetTextSize(0.04);
    // latex.SetTextAlign(13);  //align at top
    // latex.DrawLatex(1.34,4200,"Created by: Jesse A. Hernandez");
    
    //c->Print("Xi1820massFit.pdf");
}

void RooFitHist(TH1* hist,  const char* histTitle)
{
  RooMsgService::instance().setGlobalKillBelow(ERROR);
  RooWorkspace* w = new RooWorkspace(histTitle);
  RooRealVar mass("mass", "M(K^{-}#Lambda) (GeV/c^{2})", 1.61, 2.3);
  RooDataHist *data = new RooDataHist("data", "Dataset of mass", mass, hist);
  mass.setRange("signal", 1.62, 2.3);
  RooPlot* massframe = mass.frame(Title(histTitle));
  TCanvas *fitCan = new TCanvas("fitCan"," c", 700, 700);
  TPad* topPad = new TPad("topPad", "Top Pad", 0.0, 0.3, 1.0, 1.0);
  TPad* bottomPad = new TPad("bottomPad", "Bottom Pad", 0.0, 0.0, 1.0, 0.3);
  topPad->SetBottomMargin(0.02);
  topPad->SetGrid();
  bottomPad->SetTopMargin(0.015);
  bottomPad->SetBottomMargin(0.4);
  bottomPad->SetGrid(1,0);
  
  fitCan->cd();
  topPad->Draw();
  bottomPad->Draw();

  // Draw graphs in the top pad
  topPad->cd();
  

  // TCanvas *fitCan = new TCanvas("fitCan"," c", 700, 500);
  // //fitCan->SetFrameBorderMode(0);
  // //fitCan->SetFrameLineWidth(0);
  // fitCan->SetBottomMargin(0.13);
  // //gPad->SetFrameLineWidth(0);
  // gPad->SetTopMargin(0.055);
  // gPad->SetRightMargin(0.025);
  // gPad->SetLeftMargin(0.12);
  // gPad->SetBottomMargin(0.15);
  //massframe->GetXaxis()->CenterTitle(true);
  //massframe->GetYaxis()->CenterTitle(true);
  //massframe->GetYaxis()->SetTitle("Events");
  massframe->GetXaxis()->SetTitleSize(0.065);
  massframe->GetYaxis()->SetTitleSize(0.065);
  massframe->GetXaxis()->SetLabelSize(0.05);
  massframe->GetYaxis()->SetLabelSize(0.05);
  massframe->GetXaxis()->SetTitleOffset(0.96);
  massframe->GetYaxis()->SetTitleOffset(0.89);
  massframe->GetYaxis()->SetNdivisions(510);
  massframe->GetYaxis()->SetMaxDigits(2);
  massframe->SetMarkerStyle(24);
  massframe->SetMarkerColor(kBlue);
  massframe->SetLineColor(kBlue);
  massframe->SetMarkerSize(0.5);

  w->import(RooArgSet(mass));
  w->factory("Polynomial::bkgd(mass,{a0[-5.7e+03, -1.e4,1.e4],a1[8.68644e+03,-1.e4,1e4], a2[-4.1e+03,-1.e4,1e4], a3[650, 1e-2,1e6]})");//
  w->factory("BreitWigner::xim1820(mass,mean1820[1.823,1.818,1.828],sigma1820[0.02, 0.004, 0.08])");
  w->factory("BreitWigner::xim1690(mass,mean1690[1.690,1.68,1.71],sigma1690[0.02, 0.02, 0.06])");
  //w->factory("Gaussian::xim1690(mass,mean1690[1.690,1.68,1.71],sigma1690[0.02, 0.02, 0.025])");
  //w->factory("Gaussian::xim1820(mass,mean1820[1.823,1.818,1.828],sigma1820[0.01, 0.004, 0.05])");
//w->factory("Voigtian::xim1820(mass,mean1820[1.823,1.818,1.828],sigma1820[0.01, 0.004, 0.08], gamma1820[0.015,0.005,0.025])");
  //w->factory("Voigtian::xim1690(mass,mean1690[1.690,1.68,1.73],sigma1690[0.01, 0.004, 0.03], gamma1690[0.02, 0.005,0.035])");
  w->factory("BreitWigner::xim1620(mass,mean1620[1.630,1.61,1.645],sigma1620[0.005, 0.004, 0.04])");
     
  //Create model and fit to data
  w->factory("SUM::model( nxim1820[5.20482e+01,1,2000]*xim1820, nxim1690[2.64526e+01,1,1000]*xim1690, nxim1620[0]*xim1620, nbkgd[1000,1,1e8]*bkgd)");//nbkgd[200,1,1e6]*bkgd,
  w->pdf("model")->fitTo(*data,Extended(true),PrintLevel(-1),PrintEvalErrors(-1),Verbose(false),Warnings(false));
  //Plot model and data 
  data->plotOn(massframe, Name("data"), MarkerStyle(24), MarkerSize(0.9),MarkerColor(kBlue), LineColor(kBlue));
  w->pdf("model")->paramOn(massframe, Format("NE",AutoPrecision(1)), Layout(0.57, 0.97, 0.94), Parameters(RooArgSet(*w->var("nxim1820"),*w->var("mean1820"),*w->var("sigma1820"), *w->var("nxim1690"),*w->var("mean1690"),*w->var("sigma1690"))));//Parameters(RooArgSet(*w->var("nxim1820"),*w->var("mean1820"),*w->var("gamma1820"), *w->var("nxim1690"),*w->var("mean1690"),*w->var("gamma1690")))
  w->pdf("model")->plotOn(massframe, LineWidth(2), Name("model"), Range("signal"));//,Range(1.68,2.3)
  w->pdf("xim1820")->plotOn(massframe,DrawOption("F"), FillColor(kBlue-9),FillStyle(3001), VLines(),MoveToBack(), Normalization(w->var("nxim1820")->getVal(), RooAbsReal::NumEvent), Name("xim1820"));
  w->pdf("xim1690")->plotOn(massframe,DrawOption("F"), FillColor(kBlue-9),FillStyle(3001),VLines(),MoveToBack(), Normalization(w->var("nxim1690")->getVal(), RooAbsReal::NumEvent), Name("xim1690"));
  w->pdf("xim1620")->plotOn(massframe,DrawOption("F"), FillColor(kBlue-9),FillStyle(3001),MoveToBack(), Normalization(w->var("nxim1620")->getVal(), RooAbsReal::NumEvent), Name("xim1620"));
  w->pdf("bkgd")->plotOn(massframe, LineStyle(kDotted), Normalization(w->var("nbkgd")->getVal(), RooAbsReal::NumEvent), Name("bkgd"), Range("signal"));
  
  //Get approxmate chisq of model and data fit
  double chiSqNdf = massframe->chiSquare("model","data",8);
  char str_chi[50];
  std::sprintf(str_chi,"%.2f", chiSqNdf);
  
  double pxmax = fitCan->GetUxmax();
  double pxmin = fitCan->GetUxmax();
  double pymax = fitCan->GetUymax();
  double pymin = fitCan->GetUymax();
  
  double ypadmin = massframe->GetMaximum();
  double ypadmax = massframe->GetMinimum();
  cout << ypadmin << "\t" << ypadmax << endl;
  
  auto legend = new TLegend(pxmin*0.72,pymin*0.73,pxmax*0.88,pymax*0.94); //top right corner
  legend->SetTextSize(0.06);
  legend->SetBorderSize(0);
  legend->SetFillStyle(0);
    
  legend->AddEntry(massframe->findObject("data") ,"GlueX","lep");
  legend->AddEntry(massframe->findObject("model"),"Fit","l");
  // legend->AddEntry(massframe->findObject("xigaus"),"Johnson","f");
  // legend->AddEntry(massframe->findObject("sigma"),"Voigtian","l");
  legend->AddEntry(massframe->findObject("bkgd"), "Background", "l");
  
  TLine *l1620 = new TLine(1.62, ypadmin,1.62,ypadmax);
  l1620->SetLineWidth(2);
  l1620->SetLineStyle(2);
  //massframe->addObject(l1620, " ");

  TLine *l1950 = new TLine(1.95, ypadmin,1.95,ypadmax);
  l1950->SetLineWidth(2);
  l1950->SetLineStyle(2);
  massframe->addObject(l1950, " ");

  TLine *l2030 = new TLine(2.03, ypadmin,2.03,ypadmax);
  l2030->SetLineWidth(2);
  l2030->SetLineStyle(2);
  massframe->addObject(l2030, " ");

  //draw frame and legend
  massframe->Draw();  
  //legend->Draw("same");
  //draw words
  TLatex latex;
  latex.SetTextSize(0.055);
  latex.DrawLatex(1.84, 475, "#Xi(1820)^{-}");
  latex.DrawLatex(1.66, 300, "#Xi(1690)^{-}");
  
  //Show residual and pull
  // Construct a histogram with the residuals of the data w.r.t. the curve
  RooHist* hresid = massframe->residHist("data","model") ;
  
  // Construct a histogram with the pulls of the data w.r.t the curve
  RooHist* hpull = massframe->pullHist("data","model") ;
  
  // Create a new frame to draw the residual distribution and add the distribution to the frame
  RooPlot* frame2 = mass.frame(Title("Residual Distribution")) ;
  frame2->addPlotable(hresid,"P") ;
  // new TCanvas;
  // frame2->DrawClone();
  
  // Create a new frame to draw the pull distribution and add the distribution to the frame
  RooPlot* frame3 = mass.frame(Title("Pull Distribution")) ;
  frame3->addPlotable(hpull,"P") ;
  
  frame3->DrawClone();
  //fitCan->Print("Xi1820massFit_Params.pdf");
}

