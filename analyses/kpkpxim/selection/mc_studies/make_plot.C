/*-------------------------------------------------------------------------------------------------
  [Jesse A. Hernandez]
  This macros will take a root file as an argument and plot it's content with user-specific format
--------------------------------------------------------------------------------------------------*/
//instantiate methods used in main
void make_plot(vector<TH1D*> vec_hist, string save_name, string leg_title, Bool_t make_leg = true);
void merge_plot(vector<TH1D*> vec_hist, string save_name, string leg_title, Bool_t make_leg = true);
void style_format();

// main function (change main name when copying)
int make_plot()
{
  // call plot styling function
  style_format();
  
  //initiate variables
  vector<TH1D*> vect_histo_accept;
  vector<TH1D*> vect_histo_accept_weighted;
  vector<TH1D*> vect_histo_mass;
  vector<TH1D*> vect_histo_mass_weighted;
  vector<TH1D*> vect_histo_cos_weighted;
  // Load root tree of choice
  TFile *f = TFile::Open( "data.root");
  vect_histo_accept.push_back( (TH1D*)f->Get("Spring_2017/t_dist_acceptance")->Clone() );
  vect_histo_accept_weighted.push_back( (TH1D*)f->Get("Spring_2017/t_dist_acceptance_weighted")->Clone() );
  TH1D* hist2017 = (TH1D*)f->Get("Spring_2017/ystar_M_qval")->Clone();
  vect_histo_mass.push_back( hist2017 );
  vect_histo_mass.push_back( (TH1D*)f->Get("Spring_2017/ystar_M_mc")->Clone() );
  vect_histo_mass_weighted.push_back( hist2017 );
  vect_histo_mass_weighted.push_back( (TH1D*)f->Get("Spring_2017/ystar_M_mc_weighted")->Clone() );
  TH1D* histcos2017 = (TH1D*)f->Get("Spring_2017/xim_costheta_hf_qval")->Clone();
  vect_histo_cos_weighted.push_back(histcos2017);
  vect_histo_cos_weighted.push_back( (TH1D*)f->Get("Spring_2017/xim_costheta_hf_mc_weighted")->Clone() );
  
  vect_histo_accept.push_back( (TH1D*)f->Get("Spring_2018/t_dist_acceptance")->Clone() );
  vect_histo_accept_weighted.push_back( (TH1D*)f->Get("Spring_2018/t_dist_acceptance_weighted")->Clone() );
  TH1D* hist201801 = (TH1D*)f->Get("Spring_2018/ystar_M_qval")->Clone();
  vect_histo_mass.push_back( hist201801 );
  vect_histo_mass.push_back( (TH1D*)f->Get("Spring_2018/ystar_M_mc")->Clone() );
  vect_histo_mass_weighted.push_back( hist201801 );
  vect_histo_mass_weighted.push_back( (TH1D*)f->Get("Spring_2018/ystar_M_mc_weighted")->Clone() );
  TH1D* histcos201801 = (TH1D*)f->Get("Spring_2018/xim_costheta_hf_qval")->Clone();
  vect_histo_cos_weighted.push_back(histcos201801);
  vect_histo_cos_weighted.push_back( (TH1D*)f->Get("Spring_2018/xim_costheta_hf_mc_weighted")->Clone() );
  
  vect_histo_accept.push_back( (TH1D*)f->Get("Fall_2018/t_dist_acceptance")->Clone() );
  vect_histo_accept_weighted.push_back( (TH1D*)f->Get("Fall_2018/t_dist_acceptance_weighted")->Clone() );
  TH1D* hist201808 = (TH1D*)f->Get("Fall_2018/ystar_M_qval")->Clone();
  vect_histo_mass.push_back( hist201808 );
  vect_histo_mass.push_back( (TH1D*)f->Get("Fall_2018/ystar_M_mc")->Clone() );
  vect_histo_mass_weighted.push_back( hist201808 );
  vect_histo_mass_weighted.push_back( (TH1D*)f->Get("Fall_2018/ystar_M_mc_weighted")->Clone() );
  TH1D* histcos201808 = (TH1D*)f->Get("Spring_2018/xim_costheta_hf_qval")->Clone();
  vect_histo_cos_weighted.push_back(histcos201808);
  vect_histo_cos_weighted.push_back( (TH1D*)f->Get("Fall_2018/xim_costheta_hf_mc_weighted")->Clone() );
  
  // Call function to plot
  make_plot(vect_histo_accept, "t_dist_accept_phase1", "GlueX-I");
  make_plot(vect_histo_accept_weighted, "t_dist_accept_phase1_weighted", "GlueX-I");
  merge_plot(vect_histo_mass, "decayxim_M_phase1_mc_data", "GlueX-I");
  merge_plot(vect_histo_mass_weighted, "decayxim_M_phase1_mc_data_weighted", "GlueX-I");
  merge_plot(vect_histo_cos_weighted, "costheta_hf_phase1_mc_data_weighted", "GlueX-I");

  return 0;
}

// make plot to be called by main function
/* Arguments Description:
hist: vector of histograms to plot
legend_title: title of legend (name of data set)
*/
void make_plot(vector<TH1D*> vec_hist, string save_name, string leg_title, Bool_t make_leg = true)
{
  // Plot histogram(s) algorithm 
  TCanvas *c = new TCanvas(leg_title.c_str(),leg_title.c_str(),800,700);
  THStack *hs = new THStack(leg_title.c_str(),"");
  
  double pxmax = c->GetUxmax();
  double pxmin = c->GetUxmax();
  double pymax = c->GetUymax();
  double pymin = c->GetUymax();
  
  vec_hist[0]->SetFillColorAlpha(kGray,0.9);
  vec_hist[0]->SetLineColor(kBlack);
  //vec_hist[0]->RebinX();
 
  vec_hist[1]->SetFillColorAlpha(kPink-9,0.6);
  vec_hist[1]->SetLineColor(kBlack);
  //vec_hist[1]->RebinX();

  vec_hist[2]->SetFillColorAlpha(kAzure-9,0.6);
  vec_hist[2]->SetLineColor(kBlack);
  //vec_hist[2]->RebinX();
   
  hs->Add(vec_hist[0],"hist");
  hs->Add(vec_hist[2],"hist");
  hs->Add(vec_hist[1],"hist");
  
  //hs->SetTitle(stack_title.c_str());
  hs->Draw("no stack");
  hs->SetTitle("; -t (GeV)^{2}; Acceptance, #epsilon");
  hs->GetYaxis()->SetTitleOffset(0.8);
  hs->SetMaximum(0.016);
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
  //c->Print((save_name+".svg").c_str());
}

void merge_plot(vector<TH1D*> vec_hist, string save_name, string leg_title, Bool_t make_leg = true)
{
  // Plot histogram(s) algorithm 
  TH1D* merged_hist = (TH1D*)vec_hist[0]->Clone();
  TH1D* merged_hist_mc = (TH1D*)vec_hist[1]->Clone();
  merged_hist->Reset();
  merged_hist_mc->Reset();

  style_format();
  TCanvas *c = new TCanvas(leg_title.c_str(),leg_title.c_str());
  
  c->SetGrid();
  double pxmax = c->GetUxmax();
  double pxmin = c->GetUxmax();
  double pymax = c->GetUymax();
  double pymin = c->GetUymax();

  TList *lst = new TList();
  TList *lstmc = new TList();
  for(Int_t i = 0; i < vec_hist.size()/2;i++ )
    {lst->Add(vec_hist[2*i]); lstmc->Add(vec_hist[2*i+1]);}

  merged_hist->Merge(lst);
  merged_hist->SetFillColorAlpha(kGray,0.9);
  merged_hist->SetLineColor(kBlack);
  merged_hist_mc->Merge(lstmc);
  merged_hist_mc->SetFillColorAlpha(kAzure-9,0.6);
  merged_hist_mc->SetLineColor(kBlack);
  merged_hist_mc->Scale(merged_hist->Integral("width")/merged_hist_mc->Integral("width"));
  merged_hist_mc->SetMaximum(merged_hist->GetMaximum()*1.2);

  merged_hist->GetYaxis()->SetRangeUser(0,merged_hist->GetMaximum()*1.2);
  merged_hist->SetTitle("; M(#Xi^{-}K^{+}_{slow}) (GeV) ;arb. units");
  merged_hist->SetTitleOffset(0.8,"Y");
  merged_hist->Draw("hist");
  merged_hist_mc->Draw( "hist same");
  
  // Draw legend
  auto legend = new TLegend(pxmin*0.68,pymin*0.73,pxmax*0.92,pymax*0.91); //top right corner
  legend->SetBorderSize(0);
  legend->SetTextSize(0.065);
  legend->SetHeader(leg_title.c_str(),"C"); // option "C" allows to center the header
  legend->AddEntry(merged_hist, "Data", "f");
  //legend->AddEntry(vec_hist[0], "Data (Q-Value)", "f");
  legend->AddEntry(merged_hist_mc, "Simulation", "f");
  legend->Draw("same");
  //Fit the merged hist

  //Save plot
  gPad->Update();
  //c->Print((save_name+".pdf").c_str());
}

// input specific style formatting of user choice
void style_format()
{
  //gStyle->SetCanvasPreferGL(true);
  gStyle->SetCanvasColor(0);
  gStyle->SetCanvasBorderSize(10);
  gStyle->SetCanvasBorderMode(0);
  gStyle->SetCanvasDefH(500);
  gStyle->SetCanvasDefW(800);

  gStyle->SetPadColor       (0);
  gStyle->SetPadBorderSize  (10);
  gStyle->SetPadBorderMode  (0);
  gStyle->SetPadBottomMargin(0.18);
  gStyle->SetPadTopMargin   (0.03);
  gStyle->SetPadLeftMargin  (0.13);
  gStyle->SetPadRightMargin (0.02);
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
  gStyle->SetLegendFillColor(0);
  
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
  gStyle->SetTitleOffset(0.8,"Y");

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
