#include "gxana/common/Paths.h"
void GetWeightedHisto( string root_file_name="kpkpxim__M23_2017-01_ana56", string delim="_tCut", int n_threads = 16);
void setStyle();

int XimMassQVal()
{
  setStyle();
  GetWeightedHisto();
  GetWeightedHisto("kpkpxim__B4_M23_2018-01_ana03");
  GetWeightedHisto("kpkpxim__B4_M23_2018-08_ana02");
    
  return 0;
}

void GetWeightedHisto( string root_file_name="kpkpxim__M23_2017-01_ana56", string delim="_tCut", int n_threads = 16)
{
	// Parallelize with n threads
	if(n_threads > 0.0)	ROOT::EnableImplicitMT(n_threads);
	//setStyle();
    
	// Branches you want to use get
	std::vector<std::string> branches = {"decayxim_M",
                                         "qvalue_decayxim_M",
                                         "chiSqNdf_decayxim_M",
                                         "acc_weight"  
	};
    // make data frame
	// format : tree name, file name, branches to open
	auto df = ROOT::RDataFrame("flatTree_kpkpxim", (gxana::EnvPath("GXANA_OUTPUT", "kpkpxim/qfactors/")+root_file_name+"_nominal"+delim+"_1111111"+"/postQVal_flatTree_"+root_file_name+"_nominal"+delim+"_1111111.root").c_str(), branches)
      .Define("qvalue_acc", "acc_weight*qvalue_decayxim_M")
      .Define("qacc_bkg", "acc_weight*(1-qvalue_decayxim_M)");
    auto dfmc = ROOT::RDataFrame("flatTree_kpkpxim", (gxana::EnvPath("GXANA_DATA", "flatTrees/flatTree_")+root_file_name+"_gen_amp_V2_nominal"+delim+".root").c_str(), {"decayxim_M","acc_weight"});
    // Print Yield Value
	cout << "Sum of Weighted Q-Values: " << df.Sum("qvalue_acc").GetValue() << endl;
    Double_t qSum = df.Sum("qvalue_acc").GetValue();

    // Draw q-value subtracted mass distribution
	auto h = df.Histo1D({"data","; M(#Lambda#pi^{-}) (GeV); Events/ 3 MeV",60,1.27,1.45}, "decayxim_M","acc_weight");
    h->SetMarkerStyle(8);
    h->SetMarkerSize(1.1);
    h->GetYaxis()->SetMaxDigits(3);
    auto hmc = dfmc.Histo1D({*h}, "decayxim_M","acc_weight");
    hmc->SetLineColor(kAzure);
    hmc->SetLineWidth(2);
    auto h1 = df.Histo1D({*h},"decayxim_M", "qvalue_acc");
    h1->SetName("signal");
    h1->SetFillColor(38);
    h1->SetFillStyle(3002);
	auto h2 = df.Histo1D({*h},"decayxim_M", "qacc_bkg");
    h2->SetName("bkg");
    h2->SetLineWidth(3);
    h2->SetLineColor(46);
    
    TCanvas *c = new TCanvas(root_file_name.c_str(),root_file_name.c_str());
    gPad->SetGrid();
    h->DrawClone("e1");
    h2->DrawClone("hist same");
    h1->DrawClone("hist same");
    //hmc->Scale(h1->Integral("width")/hmc->Integral("width"));
    hmc->Scale(h1->GetMaximum()/hmc->GetMaximum());
    //hmc->DrawClone("hist same ");

    auto legend = new TLegend(0.7,0.7,0.92,0.93); //top right corner
    legend->SetTextSize(0.055);
    legend->SetBorderSize(0);
    legend->SetFillStyle(0);
    //legend->SetHeader("The Legend Title","C"); // option "C" allows to center the header
    //legend->AddEntry(h1,"Histogram filled with random numbers","f");
    
    legend->AddEntry("data","Data","lep");
    legend->AddEntry("signal","Q-Signal","f");
    legend->AddEntry("bkg","Q-Bkg","l");
    legend->Draw("same");
    
    TLatex latex;
    char str[100];
    std::sprintf(str, "#color[4]{N_{#Xi^{-}} = %.0f }", qSum);
    latex.SetTextSize(0.057);
    latex.DrawLatex(1.335, h1->GetMaximum()*0.95, str);

    //Draw chisqndf for all fits
    TCanvas *c1 = new TCanvas(("chisqndf"+root_file_name).c_str(),("chisqndf"+root_file_name).c_str());
    auto chisqhist = df.Histo1D({"","; #chi^{2}_{#nu}; Events",60,0,6}, "chiSqNdf_decayxim_M");
    chisqhist->GetYaxis()->SetMaxDigits(3);
    chisqhist->SetFillColorAlpha(kGray,0.8);
    gStyle->SetPadBottomMargin(0.18);
    chisqhist->Draw();
    //Save plots
    c->SaveAs(("xim_qacc_"+root_file_name+delim+".pdf").c_str());
    c1->SaveAs(("chisqndf_qvalue_"+root_file_name+delim+".pdf").c_str());
}

void setStyle()
{
  //gStyle->SetCanvasPreferGL(true);
  gStyle->SetCanvasColor(0);
  gStyle->SetCanvasBorderSize(10);
  gStyle->SetCanvasBorderMode(0);
  gStyle->SetCanvasDefH(600);
  gStyle->SetCanvasDefW(700);

  gStyle->SetPadColor       (0);
  gStyle->SetPadBorderSize  (10);
  gStyle->SetPadBorderMode  (0);
  gStyle->SetPadBottomMargin(0.15);
  gStyle->SetPadTopMargin   (0.06);
  gStyle->SetPadLeftMargin  (0.15);
  gStyle->SetPadRightMargin (0.05);
  gStyle->SetPadGridX       (0);
  gStyle->SetPadGridY       (0);
  gStyle->SetPadTickX       (0);
  gStyle->SetPadTickY       (0);

  gStyle->SetFrameFillStyle ( 0);
  gStyle->SetFrameFillColor ( 0);
  gStyle->SetFrameLineColor ( 1);
  gStyle->SetFrameLineStyle ( 0);
  gStyle->SetFrameLineWidth ( 1);
  gStyle->SetFrameBorderSize(10);
  gStyle->SetFrameBorderMode( 0);

  gStyle->SetNdivisions(505);

  gStyle->SetLineWidth(2);
  gStyle->SetHistLineWidth(2);
  gStyle->SetFrameLineWidth(2);
  //gStyle->SetLegendFillColor(1);
  
  gStyle->SetLegendBorderSize(0);
  gStyle->SetLegendFont(132);
  gStyle->SetLegendTextSize(0.06);
  gStyle->SetMarkerSize(1.2);
  gStyle->SetMarkerStyle(20);

  gStyle->SetLabelSize(0.055,"X");
  gStyle->SetLabelSize(0.055,"Y");

  gStyle->SetLabelOffset(0.010,"X");
  gStyle->SetLabelOffset(0.010,"Y");

  gStyle->SetLabelFont(132,"X");
  gStyle->SetLabelFont(132,"Y");
  gStyle->SetTitleBorderSize(0);
  gStyle->SetTitleFont(132);
  gStyle->SetTitleFont(132,"X");
  gStyle->SetTitleFont(132,"Y");

  gStyle->SetTitleSize(0.08,"X");
  gStyle->SetTitleSize(0.08,"Y");

  gStyle->SetTitleOffset(0.9,"X");
  gStyle->SetTitleOffset(0.9,"Y");

  gStyle->SetTextSize(0.08);
  gStyle->SetTextFont(132);

  gStyle->SetOptStat(0);

  gROOT->ForceStyle();

  TLatex* latex = new TLatex();
  latex->SetNDC();
  latex->SetTextFont(132);
  latex->SetTextSize(0.08);
  latex->SetTextAlign(32);
  gROOT->ForceStyle();
}

