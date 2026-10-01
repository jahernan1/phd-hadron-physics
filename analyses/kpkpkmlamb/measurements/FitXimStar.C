#include "gxana/common/Paths.h"
#include "gxana/common/Style.h"
void RooFitHist(TH1* hist, const char* histTitle, bool tCut);
void setStyle();
using namespace RooFit;

// gxana: merges FitXimStarCuts.C (tCut = true: t_dist > 1 selection, its label and output name)
void FitXimStar( int n_threads = 4, bool tCut = false) {
	// Parallelize with n threads
	if(n_threads > 0.0)	ROOT::EnableImplicitMT(n_threads);
    // Initialize variables
    string tree_dir = gxana::EnvPath("GXANA_DATA", "flatTrees/");
    
	// make data frame
	// format : tree name, file name, branches to open
	// gxana: the merged GlueX-I tree (hadd of the three periods' _nominal_allCuts files) from $GXANA_DATA; legacy read it from cwd
	auto df = ROOT::RDataFrame("flatTree_kpkpkmlamb", gxana::EnvPath("GXANA_DATA", "kpkpkmlamb/flatTree_kpkpkmlamb_GlueX-I.root"))
        .Filter(tCut ? "kphigh_p4.Rapidity()>0&&t_dist>1" : "kphigh_p4.Rapidity()>0");
    
    // make histos and merge
    auto h = df.Histo1D({""," ; M(#LambdaK^{-}) (GeV/c^{2}); Counts", 150,1.6,2.6}, "ximstar_M");
    
    double binWidth = h->GetXaxis()->GetBinWidth(1);
    char histTitle[100];
    sprintf(histTitle, " ;M(#LambdaK^{-}) (GeV/c^{2}); Events / %.2f MeV", binWidth*1000);
    //add histos to list 
    gStyle->SetOptFit(1);
    h->GetXaxis()->SetTitleOffset(0.92);
    h->GetYaxis()->SetTitleOffset(0.73);
    //hist_merged->GetYaxis()->SetNdivisions(-5, kFALSE);
    h->GetYaxis()->SetMaxDigits(3);
                               
    RooFitHist(h.GetPtr(), histTitle, tCut);
}

void RooFitHist(TH1* hist,  const char* histTitle, bool tCut)
{
    RooMsgService::instance().setGlobalKillBelow(ERROR);
    RooWorkspace* w = new RooWorkspace(histTitle);
    RooRealVar mass("mass", "M(K^{-}#Lambda) (GeV/c^{2})", 1.6, 2.3);
    RooDataHist *data = new RooDataHist("data", "Dataset of mass", mass, hist);
    mass.setRange("signal", 1.62, 2.3);
    RooPlot* massframe = mass.frame(Title(histTitle));
    TCanvas *fitCan = new TCanvas("fitCan"," c", 700, 750);
    TPad* topPad = new TPad("topPad", "Top Pad", 0.0, 0.3, 1.0, 1.0);
    TPad* bottomPad = new TPad("bottomPad", "Bottom Pad", 0.0, 0.0, 1.0, 0.3);
    topPad->SetTopMargin(0.07);
    topPad->SetBottomMargin(0.017);
    topPad->SetLeftMargin(0.13);
    topPad->SetRightMargin(0.03);
    topPad->SetGrid();
    bottomPad->SetTopMargin(0.0);
    bottomPad->SetBottomMargin(0.35);
    bottomPad->SetLeftMargin(0.13);
    bottomPad->SetRightMargin(0.03);
    bottomPad->SetGrid(1,0);
  
    fitCan->cd();
    topPad->Draw();
    bottomPad->Draw();
  
    // Draw graphs in the top pad
    topPad->cd();
    //massframe->GetXaxis()->CenterTitle(true);
    //massframe->GetYaxis()->CenterTitle(true);
    //massframe->GetYaxis()->SetTitle("Events");
    massframe->GetXaxis()->SetTitleSize(0.065);
    massframe->GetYaxis()->SetTitleSize(0.065);
    massframe->GetXaxis()->SetLabelSize(0.0);
    massframe->GetYaxis()->SetLabelSize(0.042);
    massframe->GetXaxis()->SetTitleOffset(0.96);
    massframe->GetYaxis()->SetTitleOffset(0.89);
    massframe->GetYaxis()->SetNdivisions(510);
    massframe->GetYaxis()->SetMaxDigits(2);
    massframe->SetMarkerStyle(24);
    massframe->SetMarkerColor(kBlue);
    massframe->SetLineColor(kBlue);
    massframe->SetMarkerSize(0.5);

    w->import(RooArgSet(mass));
    w->factory("Chebychev::bkgd(mass,{a0[0.2, -1.e4,1.e4],a1[-0.6,-1.e4,1e4], a2[-0.1,-1.e4,1e4]})");//
    w->factory("BreitWigner::xim1820(mass,mean1820[1.82,1.818,1.828],gamma1820[0.02, 0.01, 0.08])");
    w->factory("BreitWigner::xim1690(mass,mean1690[1.69,1.68,1.71],gamma1690[0.02, 0.02, 0.06])");
    //w->factory("Gaussian::xim1690(mass,mean1690[1.690,1.68,1.71],sigma1690[0.02, 0.02, 0.025])");
    //w->factory("Gaussian::xim1820(mass,mean1820[1.823,1.818,1.828],sigma1820[0.01, 0.004, 0.05])");
    //w->factory("Voigtian::xim1820(mass,mean1820[1.823,1.818,1.828],sigma1820[0.01, 0.004, 0.08], gamma1820[0.015,0.005,0.025])");
    //w->factory("Voigtian::xim1690(mass,mean1690[1.690,1.68,1.73],sigma1690[0.01, 0.004, 0.03], gamma1690[0.02, 0.005,0.035])");
    w->factory("BreitWigner::xim1620(mass,mean1620[1.620,1.61,1.645],sigma1620[0.005, 0.004, 0.04])");
     
    //Create model and fit to data
    w->factory("SUM::model( nxim1820[1500,1,5000]*xim1820, nxim1690[500,1,1000]*xim1690, nxim1620[0]*xim1620, nbkgd[10000,1,1e6]*bkgd)");//nbkgd[200,1,1e6]*bkgd,
    w->pdf("model")->fitTo(*data,Extended(true),PrintLevel(-1),PrintEvalErrors(-1),Verbose(false),Warnings(false));
    //Plot model and data 
    data->plotOn(massframe, Name("data"), MarkerStyle(24), MarkerSize(0.9),MarkerColor(kBlue), LineColor(kBlue));
    //w->pdf("model")->paramOn(massframe, Format("NE",AutoPrecision(1)), Layout(0.57, 0.97, 0.94), Parameters(RooArgSet(*w->var("nxim1820"),*w->var("mean1820"),*w->var("gamma1820"), *w->var("nxim1690"),*w->var("mean1690"),*w->var("gamma1690"))));//Parameters(RooArgSet(*w->var("nxim1820"),*w->var("mean1820"),*w->var("gamma1820"), *w->var("nxim1690"),*w->var("mean1690"),*w->var("gamma1690")))
    w->pdf("model")->plotOn(massframe, LineWidth(2), Name("model"), Range("signal"));//,Range(1.68,2.3)
    w->pdf("xim1820")->plotOn(massframe,DrawOption("F"), FillColor(kBlue-9),FillStyle(3001), VLines(),MoveToBack(), Normalization(w->var("nxim1820")->getVal(), RooAbsReal::NumEvent), Name("xim1820"));
    w->pdf("xim1690")->plotOn(massframe,DrawOption("F"), FillColor(kMagenta),FillStyle(3001),VLines(),MoveToBack(), Normalization(w->var("nxim1690")->getVal(), RooAbsReal::NumEvent), Name("xim1690"));
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
  
    auto legend = new TLegend(0.67,0.66,0.94,0.92); //top right corner
    legend->SetTextSize(0.055);
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
    massframe->addObject(l1620, " ");

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
    legend->Draw("same");
    //draw words
    TLatex latex;
    if (!tCut) {
    latex.SetTextSize(0.05);
    latex.DrawLatex(1.7, 500, "#Xi(1820)^{-}");
    latex.DrawLatex(1.62, 300, "#Xi(1690)^{-}");
    } else {  // gxana: FitXimStarCuts.C labels as saved
    latex.SetTextSize(0.065);
    //latex.DrawLatex(1.7, 500, "#Xi(1820)^{-}");
    //latex.DrawLatex(1.62, 300, "#Xi(1690)^{-}");
    //latex.DrawLatex(1.63, 230, "#color[2]{#bf{t < 1 GeV^{2}}}");
    //latex.DrawLatex(1.63, 140, "#color[2]{#bf{t = [1,2] GeV^{2}}}");
    latex.DrawLatex(1.63, 180, "#color[2]{#bf{t > 2 GeV^{2}}}");
    }

    // Get the residual plot
    bottomPad->cd();
    bottomPad->SetGrid();
    RooHist* resPlot = massframe->residHist("data","model",true);
    resPlot->SetTitle(Form("; %s;#sigma_{#scale[0.7]{pull}}",massframe->GetXaxis()->GetTitle()));
    //resPlot->SetMarkerStyle(24);
    resPlot->GetYaxis()->CenterTitle(true);
    resPlot->GetYaxis()->SetNdivisions(505);
    resPlot->GetXaxis()->SetLabelSize(0.10);
    resPlot->GetYaxis()->SetLabelSize(0.10);
    resPlot->GetXaxis()->SetTitleSize(0.16);
    resPlot->GetYaxis()->SetTitleSize(0.16);
    resPlot->GetXaxis()->SetTitleOffset(0.9);
    resPlot->GetYaxis()->SetTitleOffset(0.3);
    resPlot->GetYaxis()->SetRangeUser(-5,5);
    resPlot->DrawClone("ap");

    double xmin = TMath::MinElement(resPlot->GetN(), resPlot->GetX());
    double xmax = TMath::MaxElement(resPlot->GetN(), resPlot->GetX());
    cout << xmin << "\t" << xmax << endl;
    TLine zeroline(xmin,0,xmax,0);
    zeroline.SetLineWidth(4);
    zeroline.SetLineStyle(1);
    //zeroline.DrawClone();
  
    // gxana: FitXimStarCuts.C had this Print commented out with the _TCut3 name
    fitCan->Print(tCut ? "Xi1820massFit_TCut3.pdf" : "Xi1820massFit.pdf");
}


void setStyle()
{
    gxana::StyleParams p = gxana::FitStyle();
    p.padLeftMargin = 0.2;
    gxana::ApplyStyle(p);
}
