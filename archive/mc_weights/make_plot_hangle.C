/*-------------------------------------------------------------------------------------------------
  [Jesse A. Hernandez]
  This macros will take a root file as an argument and plot it's content with user-specific format
--------------------------------------------------------------------------------------------------*/
//instantiate methods used in main
void make_plot(vector<TH1D*> vec_hist, string save_name, string leg_title, Bool_t make_leg = true);
void load_histos(TFile *f, string dir, vector<TH1D*> vect_histo);
void style_format();

// main function (change main name when copying)
int make_plot_hangle()
{
  //initiate variables
  vector<TH1D*> vect_histo_qval;
  vector<TH1D*> vect_histo_mc;
  vector<TH1D*> vect_histo_mcw;
  vector<TH1D*> vect_histo_thrown;
  vector<TH1D*> vect_histo_thrownw;
  // Load root tree of choice
  TFile *f = TFile::Open( "data_allKaonSep.root");
  vect_histo_qval.push_back( (TH1D*)f->Get("Spring_2017/xim_costheta_hf_qval")->Clone() );
  vect_histo_mc.push_back( (TH1D*)f->Get("Spring_2017/xim_costheta_hf_mc" )->Clone() );
  vect_histo_thrown.push_back( (TH1D*)f->Get("Spring_2017/xim_costheta_hf_thrown" )->Clone() );
  vect_histo_mcw.push_back( (TH1D*)f->Get("Spring_2017/xim_costheta_hf_mc_weighted" )->Clone() );
  vect_histo_thrownw.push_back( (TH1D*)f->Get("Spring_2017/xim_costheta_hf_thrown_weighted" )->Clone() );
      
  vect_histo_qval.push_back( (TH1D*)f->Get("Spring_2018/xim_costheta_hf_qval")->Clone() );
  vect_histo_mc.push_back( (TH1D*)f->Get("Spring_2018/xim_costheta_hf_mc" )->Clone() );
  vect_histo_thrown.push_back( (TH1D*)f->Get("Spring_2018/xim_costheta_hf_thrown" )->Clone() );
  vect_histo_mcw.push_back( (TH1D*)f->Get("Spring_2018/xim_costheta_hf_mc_weighted" )->Clone() );
  vect_histo_thrownw.push_back( (TH1D*)f->Get("Spring_2018/xim_costheta_hf_thrown_weighted" )->Clone() );
      
  vect_histo_qval.push_back( (TH1D*)f->Get("Fall_2018/xim_costheta_hf_qval")->Clone() );
  vect_histo_mc.push_back( (TH1D*)f->Get("Fall_2018/xim_costheta_hf_mc" )->Clone() );
  vect_histo_thrown.push_back( (TH1D*)f->Get("Fall_2018/xim_costheta_hf_thrown" )->Clone() );
  vect_histo_mcw.push_back( (TH1D*)f->Get("Fall_2018/xim_costheta_hf_mc_weighted" )->Clone() );
  vect_histo_thrownw.push_back( (TH1D*)f->Get("Fall_2018/xim_costheta_hf_thrown_weighted" )->Clone() );
      
  // Call function to plot 
  make_plot(vect_histo_qval, "xim_costheta_qval_phase1", "Glue-X Data");
  make_plot(vect_histo_mc, "xim_costheta_mc_phase1", "MC Reconstructed", false);
  make_plot(vect_histo_thrown, "xim_costheta_thrown_phase1", "MC Generated", false);
  make_plot(vect_histo_mcw, "xim_costheta_mc_weighted_phase1", "MC Reconstructed", false);
  make_plot(vect_histo_thrownw, "xim_costheta_thrown_weighted_phase1", "MC Generated", false);

  return 0;
}

// make plot to be called by main function
/* Arguments Description:
hist: vector of histograms to plot
legend_title: title of legend (name of data set)
*/
void make_plot(vector<TH1D*> vec_hist, string save_name, string leg_title, Bool_t make_leg = true)
{
  // call plot styling function
  style_format();
  
  // Plot histogram(s) algorithm 
  TCanvas *c = new TCanvas(leg_title.c_str(),leg_title.c_str());
  THStack *hs = new THStack(leg_title.c_str(),"");
  c->SetGrid();
  
  double pxmax = c->GetUxmax();
  double pxmin = c->GetUxmax();
  double pymax = c->GetUymax();
  double pymin = c->GetUymax();
  
  vec_hist[0]->SetFillColorAlpha(kGray,0.9);
  vec_hist[0]->SetLineColor(kGray);
  vec_hist[0]->RebinX();
  hs->Add(vec_hist[0],"hist");

  vec_hist[1]->SetFillColorAlpha(kPink-9,0.6);
  vec_hist[1]->SetLineColor(0);
  vec_hist[1]->RebinX();
  hs->Add(vec_hist[1],"hist");

  vec_hist[2]->SetFillColorAlpha(kAzure-9,0.6);
  vec_hist[2]->SetLineColor(0);
  vec_hist[2]->RebinX();
  hs->Add(vec_hist[2],"hist");
  
  //hs->SetTitle(stack_title.c_str());
  hs->Draw("");
  hs->SetTitle("; cos#Theta^{Y#Xi}_{#bf{H}}; Events");
  //hs->GetXaxis()->SetLimits(-1.05,1.05);
  hs->GetYaxis()->SetMaxDigits(3);
  // Draw legend
  if(make_leg)
    {
      auto legend = new TLegend(0.2,0.65,0.4,0.9); //top left corner
      legend->SetBorderSize(0);
      legend->SetFillStyle(0);
      legend->SetTextSize(0.062);
      legend->SetHeader(leg_title.c_str(),"L"); // option "C" allows to center the header
      legend->AddEntry(vec_hist[0], "Spring 2017", "f");
      //legend->AddEntry(vec_hist[0], "Data (Q-Value)", "f");
      legend->AddEntry(vec_hist[1], "Spring 2018", "f");
      legend->AddEntry(vec_hist[2], "Fall 2018", "f");
      legend->Draw("same");
    }
  else
    {
      auto legend = new TLegend(0.2,0.7,0.4,0.9); //top left corner
      legend->SetBorderSize(0);
      legend->SetFillStyle(0);
      legend->SetTextSize(0.062);
      legend->SetHeader(leg_title.c_str(),"L"); // option "C" allows to center the header
      legend->Draw("same");
      
    }
  
  //Save plot
  gPad->Update();
  c->Print((save_name+".pdf").c_str());
}

// input specific style formatting of user choice
void style_format()
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
  gStyle->SetPadBottomMargin(0.18);
  gStyle->SetPadTopMargin   (0.08);
  gStyle->SetPadLeftMargin  (0.16);
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

  //gStyle->SetLineWidth(1);
  //gStyle->SetHistLineWidth(1);
  //gStyle->SetFrameLineWidth(2);
  //gStyle->SetLegendFillColor(1);
  
  gStyle->SetLegendBorderSize(0);
  gStyle->SetLegendFont(132);
  gStyle->SetLegendTextSize(0.06);
  gStyle->SetMarkerSize(1.2);
  gStyle->SetMarkerStyle(20);

  gStyle->SetLabelSize(0.07,"X");
  gStyle->SetLabelSize(0.07,"Y");

  gStyle->SetLabelOffset(0.008,"X");
  gStyle->SetLabelOffset(0.008,"Y");

  gStyle->SetLabelFont(132,"X");
  gStyle->SetLabelFont(132,"Y");
  gStyle->SetTitleBorderSize(0);
  gStyle->SetTitleFont(132,"T");
  gStyle->SetTitleFont(132,"X");
  gStyle->SetTitleFont(132,"Y");
  
  gStyle->SetTitleSize(0.07,"T");
  gStyle->SetTitleSize(0.08,"X");
  gStyle->SetTitleSize(0.08,"Y");
  
  gStyle->SetTitleOffset(1.0,"X");
  gStyle->SetTitleOffset(1.0,"Y");

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
