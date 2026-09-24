void RooFitHist(TH1* hist,  const char* histTitle);

void MakeXim1320_IM(int n_threads = 4, string root_file_name="kpkpxim_2017-01_ana-45") {
	// Parallelize with n threads
	if(n_threads > 0.0)	ROOT::EnableImplicitMT(n_threads);
	
	// Initialize variables
    string tree_dir = "/d/grid17/hjesse/AnalysisNote/flatTrees/";
	// Branches you want to use get
    std::vector<std::string> branches = {"decayxim_M",
                                         "acc_weight"
	};

	// make data frame
	// format : tree name, file name, branches to open
	auto df = ROOT::RDataFrame("flatTree_kpkpxim", (tree_dir+"flatTree_kpkpxim__M23_2017-01_ver56_nominal_vertexCuts.root").c_str(), branches);
    auto df1 = ROOT::RDataFrame("flatTree_kpkpxim", (tree_dir+"flatTree_kpkpxim__B4_M23_2018-01_ver03_nominal_vertexCuts.root").c_str(), branches);
	auto df2 = ROOT::RDataFrame("flatTree_kpkpxim", (tree_dir+"flatTree_kpkpxim__B4_M23_2018-08_ver02_nominal_vertexCuts.root").c_str(), branches);

    // make histos and merge
    auto h = df.Histo1D({""," ; M(#Lambda#pi^{-}) (GeV/c^{2}); Counts", 50,1.27,1.47}, "decayxim_M");
    auto h1 = df1.Histo1D({""," ; M(#Lambda#pi^{-}) (GeV/c^{2}); Counts", 50,1.27,1.47}, "decayxim_M");
    auto h2 = df2.Histo1D({""," ; M(#Lambda#pi^{-}) (GeV/c^{2}); Counts", 50,1.27,1.47}, "decayxim_M");
    double binWidth = h->GetXaxis()->GetBinWidth(1);
    char histTitle[100];
    sprintf(histTitle, " ;M(#Lambda#pi^{-}) (GeV); Events / %.0f MeV", binWidth*1000);
    
    TList *list = new TList;
    list->Add(h.GetPtr());
    list->Add(h1.GetPtr());
    list->Add(h2.GetPtr());
    
    TH1F *hist_merged = (TH1F*)h1->Clone("XiMinus_PhaseI");
    hist_merged->Reset();
    hist_merged->Merge(list);
    gStyle->SetOptFit(1);
    //hist_merged->GetXaxis()->CenterTitle(true);
    //hist_merged->GetYaxis()->CenterTitle(true);
    hist_merged->GetXaxis()->SetTitleSize(0.065);
    hist_merged->GetYaxis()->SetTitleSize(0.065);
    hist_merged->GetXaxis()->SetLabelSize(0.05);
    hist_merged->GetYaxis()->SetLabelSize(0.05);
    hist_merged->GetXaxis()->SetTitleOffset(0.92);
    hist_merged->GetYaxis()->SetTitleOffset(0.73);
    hist_merged->GetYaxis()->SetNdivisions(510);
    hist_merged->GetYaxis()->SetMaxDigits(2);
    hist_merged->SetMarkerStyle(24);
    hist_merged->SetMarkerColor(kBlue);
    hist_merged->SetLineColor(kBlue);
    hist_merged->SetMarkerSize(0.9);
    
    
    RooFitHist(hist_merged,histTitle);
    // // Draw options
    // TCanvas *c = new TCanvas("","",700,500);
    // c->SetFrameBorderMode(0);
    // c->SetFrameLineWidth(0);
    // c->SetBottomMargin(0.125);
    // gPad->SetRightMargin(0.025);
    // gPad->SetFrameLineWidth(0);
    // //hist_merged->Draw("e1");
        
    // TLatex latex;
    // latex.SetTextSize(0.04);
    // latex.SetTextAlign(13);  //align at top
    // latex.DrawLatex(1.34,4200,"Created by: Jesse A. Hernandez");
    
    // c->Print("Xi1320mass.pdf");
}

void RooFitHist(TH1* hist,  const char* histTitle)
{
  
  RooWorkspace* w = new RooWorkspace(histTitle);
  RooRealVar mass("mass", "M(#Lambda#pi^{-}) (GeV/c^{2})", 1.262, 1.45);
  RooDataHist *data = new RooDataHist("data", "Dataset of mass", mass, hist);
  RooPlot* massframe = mass.frame(RooFit::Title(histTitle));
  TCanvas *fitCan = new TCanvas("fitCan"," c", 700, 500);
  //fitCan->SetFrameBorderMode(0);
  //fitCan->SetFrameLineWidth(0);
  //gPad->SetFrameLineWidth(0);
  //gPad->SetTopMargin(0.02);
  gPad->SetTopMargin(0.055);
  gPad->SetRightMargin(0.025);
  gPad->SetLeftMargin(0.1);
  gPad->SetBottomMargin(0.15);
  //massframe->GetXaxis()->CenterTitle(true);
  //massframe->GetYaxis()->CenterTitle(true);
  //massframe->GetYaxis()->SetTitle("Events");
  massframe->GetXaxis()->SetTitleSize(0.065);
  massframe->GetYaxis()->SetTitleSize(0.065);
  massframe->GetXaxis()->SetLabelSize(0.05);
  massframe->GetYaxis()->SetLabelSize(0.05);
  massframe->GetXaxis()->SetTitleOffset(0.96);
  massframe->GetYaxis()->SetTitleOffset(0.65);
  massframe->GetYaxis()->SetNdivisions(510);
  massframe->GetYaxis()->SetMaxDigits(3);
  massframe->SetMarkerStyle(24);
  massframe->SetMarkerColor(kBlue);
  massframe->SetLineColor(kBlue);
  massframe->SetMarkerSize(0.5);

  w->import(RooArgSet(mass));
 
  w->factory("Chebychev::bkgd(mass,{a0[0.8,1.e-4,1],a1[-0.1,-1.e2,-1e-2]})");//,a1[-0.1,-1.e2,-1e-2]
  //w->factory("Voigtian::sigma(mass,mean[1.385,1.382,1.388],sig[0.0055, 0.001, 0.06], width[0.0394])");
  w->factory("Gaussian::sigma(mass,mean[1.385,1.383,1.388],sig[0.0394,0.01,0.04])");
  w->factory("Johnson::xigaus(mass,mu[1.322,1.31,1.33],lambda[0.0055,0.004,0.006], gamma[0], delta[1.1,0.1,10])");
   
  //Create model and fit to data
  w->factory("SUM::model( nsigma[200,1,1e6]*sigma, nbkgd[200,1,1e6]*bkgd, nxi[100,1,1e5]*xigaus)");//nbkgd[200,1,1e6]*bkgd,
  w->pdf("model")->fitTo(*data,RooFit::Extended(true),RooFit::SumW2Error(true),RooFit::PrintLevel(-1),RooFit::PrintEvalErrors(-1),RooFit::Verbose(false),RooFit::Warnings(false));
  //Plot model and data 
  data->plotOn(massframe, RooFit::Name("data"), RooFit::MarkerStyle(24), RooFit::MarkerSize(0.9),RooFit::MarkerColor(kBlue), RooFit::LineColor(kBlue));
  w->pdf("model")->paramOn(massframe, RooFit::Format("NE",RooFit::AutoPrecision(1)), RooFit::Layout(0.55, 0.95, 0.92) );
  w->pdf("model")->plotOn(massframe, RooFit::LineWidth(2), RooFit::Name("model"));
  w->pdf("xigaus")->plotOn(massframe,RooFit::DrawOption("F"), RooFit::FillColor(kBlue-9),RooFit::FillStyle(3001),RooFit::MoveToBack(), RooFit::Normalization(w->var("nxi")->getVal(), RooAbsReal::NumEvent), RooFit::Name("xigaus"));
  w->pdf("sigma")->plotOn(massframe, RooFit::LineStyle(kDotted) , RooFit::LineColor(kBlue-9), RooFit::Normalization(w->var("nsigma")->getVal(), RooAbsReal::NumEvent), RooFit::Name("sigma"));
  w->pdf("bkgd")->plotOn(massframe, RooFit::LineStyle(kDotted), RooFit::Normalization(w->var("nbkgd")->getVal(), RooAbsReal::NumEvent), RooFit::Name("bkgd"));
  massframe->Draw();
  //Get approxmate chisq of model and data fit
  RooChi2Var chi2 (" ", " ", *w->pdf("model"), *data, RooFit::Extended());
  Double_t chi2_val = chi2.getVal()/(45+7);
  double chiSqNdf = massframe->chiSquare("model","data",7);
  printf("ChiSqNdf: %f\n",  chiSqNdf);
  
  //Get yield and error
  double yield = w->var("nxi")->getVal();
  //*yield_err = TMath::Sqrt(*yield);
  double yield_err = w->var("nxi")->getError();
  //Get mean and variance
  double xiMu = w->var("mu")->getVal();
  double xiMuErr = w->var("mu")->getError();
  double xiLambda = w->var("lambda")->getVal();
  double xiDelta = w->var("delta")->getVal();
  double xiGamma = w->var("gamma")->getVal();
  double xiMean = xiMu - xiLambda * exp(1 / (2*pow(xiDelta,2)) ) * sinh(xiGamma/xiDelta);
  double xiSigma = sqrt( pow(xiLambda, 2)/2*(exp(pow(xiDelta, -2) ) - 1 )*(exp(pow(xiDelta, -2) )*cosh(2*xiGamma/xiDelta )+1));
  printf("Mean and Sigma: %f, %f \n", xiMean, xiSigma);

  double pxmax = fitCan->GetUxmax();
  double pxmin = fitCan->GetUxmax();
  double pymax = fitCan->GetUymax();
  double pymin = fitCan->GetUymax();

  printf("Yield for kpkpxim hist: %s\n", hist->GetName());
  printf("%f +/- %f\n\n", yield, yield_err);

  auto legend = new TLegend(pxmin*0.185,pymin*0.67,pxmax*0.38,pymax*0.97); //top left corner
  legend->SetTextSize(0.06);
  legend->SetBorderSize(0);
  legend->SetFillStyle(0);
    
  legend->AddEntry(massframe->findObject("data") ,"GlueX","lep");
  legend->AddEntry(massframe->findObject("model"),"Fit","l");
  // legend->AddEntry(massframe->findObject("xigaus"),"Johnson","f");
  // legend->AddEntry(massframe->findObject("sigma"),"Voigtian","l");
  legend->AddEntry(massframe->findObject("bkgd"), "Background", "l");
 

  TLatex latex;
  // char str[100];
  // char str_mean[100];
  // char str_chisq[100];
  // std::sprintf(str, "#color[4]{N_{#Xi^{-}} = %.0f #pm %.0f}", yield, yield_err);
  // printf("Mass: %f +/- %f\n", xiMu*1e3, xiMuErr*1e3);
  // std::sprintf(str_mean, "#color[4]{M_{#Xi^{-}} = %.2f(%0.0f) MeV/c^{2}}", xiMu*1e3, xiMuErr*1e5);
  // std::sprintf(str_chisq, "#color[4]{#chi^{2}/ndf = %.2f}",chiSqNdf );
  latex.SetTextSize(0.06);
  // latex.DrawLatex(1.338, 2540, "GlueX Phase I");
  // latex.DrawLatex(1.338, 2315, str);
  // latex.DrawLatex(1.338, 2090, str_mean);
  // latex.DrawLatex(1.338, 1865, str_chisq);
  latex.DrawLatex(1.33, 6000, "#Xi(1320)^{-}");
  
  legend->Draw("same");
  //fitCan->SetGrid();  
  fitCan->Print("Xi1320massFit_annotated.pdf");
  //fitCan->SaveAs("/d/grid17/hjesse/macros/yields/Xim_InvariantMassFit_Phase1.png");
}

