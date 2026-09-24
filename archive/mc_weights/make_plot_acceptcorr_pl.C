/*-------------------------------------------------------------------------------------------------
  [Jesse A. Hernandez]
  This macros will take a root file as an argument and plot it's content with user-specific format
--------------------------------------------------------------------------------------------------*/
//instantiate methods used in main
void make_plot(vector<TH1D*> vec_hist, string save_name, string leg_title, Bool_t make_leg = true);
void merge_fit(vector<TH1D*> vec_hist, string save_name, string leg_title, Bool_t make_leg = true);
void style_format();

// main function (change main name when copying)
int make_plot_acceptcorr_pl()
{
  //initiate variables
  vector<TH1D*> vect_histo_acceptcorr;
  // Load root tree of choice
  TFile *f = TFile::Open( "data_allKaonSep.root");
  // get histograms to plot
  vect_histo_acceptcorr.push_back( (TH1D*)f->Get("Spring_2017/xim_costheta_hf_acceptcorr")->Clone() );
  vect_histo_acceptcorr.push_back( (TH1D*)f->Get("Spring_2018/xim_costheta_hf_acceptcorr")->Clone() );
  vect_histo_acceptcorr.push_back( (TH1D*)f->Get("Fall_2018/xim_costheta_hf_acceptcorr")->Clone() );
  
  // Call function to plot
  style_format();
  //make_plot(vect_histo_acceptcorr, "xim_costheta_hf_acceptcorr_phase1", "",false);
  merge_fit(vect_histo_acceptcorr, "xim_costheta_hf_acceptcorr_pl_phase1_mergedfit", "");
    
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
  TCanvas *c = new TCanvas(leg_title.c_str(),leg_title.c_str());
  THStack *hs = new THStack(leg_title.c_str(),"");
  c->SetGrid();
  
  double pxmax = c->GetUxmax();
  double pxmin = c->GetUxmax();
  double pymax = c->GetUymax();
  double pymin = c->GetUymax();
  
  vec_hist[0]->SetFillColorAlpha(kGray,0.9);
  vec_hist[0]->SetLineColor(kBlack);
  vec_hist[0]->RebinX();
  hs->Add(vec_hist[0],"hist");

  vec_hist[1]->SetFillColorAlpha(kPink-9,0.6);
  vec_hist[1]->SetLineColor(kBlack);
  vec_hist[1]->RebinX();
  vec_hist[1]->Scale(vec_hist[0]->GetMaximum()/vec_hist[1]->GetMaximum());
  hs->Add(vec_hist[1],"hist");

  vec_hist[2]->SetFillColorAlpha(kAzure-9,0.6);
  vec_hist[2]->SetLineColor(kBlack);
  vec_hist[2]->RebinX();
  vec_hist[2]->Scale(vec_hist[0]->GetMaximum()/vec_hist[2]->GetMaximum());
  hs->Add(vec_hist[2],"hist");

  //hs->SetTitle(stack_title.c_str());
  hs->Draw("");
  hs->SetTitle("; cos#Theta_{#bf{H}}^{Y#Xi}; a.u. / #epsilon");
  //hs->SetMaximum(2e5);
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
//merge and fit 
void merge_fit(vector<TH1D*> vec_hist, string save_name, string leg_title, Bool_t make_leg = true)
{
  // Plot histogram(s) algorithm 
  TH1D* merged_hist = (TH1D*)vec_hist[0]->Clone();
  merged_hist->Reset();
  TCanvas *c = new TCanvas(leg_title.c_str(),leg_title.c_str());
  c->SetGrid();
  double pxmax = c->GetUxmax();
  double pxmin = c->GetUxmax();
  double pymax = c->GetUymax();
  double pymin = c->GetUymax();

  TList *lst = new TList();
  for(Int_t i = 0; i < vec_hist.size();i++ )
    lst->Add(vec_hist[i]);

  merged_hist->Merge(lst);
  merged_hist->SetMarkerColor(kAzure);
  merged_hist->SetMarkerStyle(24);
  merged_hist->GetYaxis()->SetMaxDigits(3);

  //Fit the merged hist
  // TF1* fit = new TF1("fit","[zc]*([s0]+[p0]*x-[p1]*sqrt(1-x**2)+[d0]*0.5*(3*x**2-1)-[d1]*3*x*sqrt(1-x**2)+[d2]*3*(1-x**2)+[f0]*0.5*(5*x**3-3*x))**2",-1,1);//+[f0]*0.5*(5*x**3-3*x)+[g0]*0.125*(35*x**4-30*x**2+3)-[p1]*sqrt(1-x**2)-[d1]*3*x*sqrt(1-x**2)+[d2]*3*(1-x**2)-[p1]*sqrt(1-x**2)-[d1]*3*x*sqrt(1-x**2)
  // fit->SetParameters(160,1,100,50,80,100);
  TF1* fit = new TF1("fit","[zc]*([s0]+[p0]*x+[d0]*0.5*(3*x**2-1)+[d2]*3*(1-x**2)+[f0]*0.5*(5*x**3-3*x)+[f1]*(1.5)*(1-5*x**2)*sqrt(1-x**2)+[f2]*15*x*(1-x**2)+[g0]*0.125*(35*x**4-30*x**2+3))**2",-1,1);//+[f0]*0.5*(5*x**3-3*x)+[g0]*0.125*(35*x**4-30*x**2+3)-[p1]*sqrt(1-x**2)-[d1]*3*x*sqrt(1-x**2)+[d2]*3*(1-x**2)-[p1]*sqrt(1-x**2)-[d1]*3*x*sqrt(1-x**2)+[f1]*(3/2)*(1-5*x**2)*sqrt(1-x**2)+
  //TF1* fit = new TF1("fit","[zc]*([s0]+[p0]*x-[p1]*sqrt(1-x**2)+[d0]*0.5*(3*x**2-1)-[d1]*3*x*sqrt(1-x**2)+[f0]*0.5*(5*x**3-3*x)+[g0]*0.125*(35*x**4-30*x**2+3))**2",-1,1);//+[f0]*0.5*(5*x**3-3*x)+[g0]*0.125*(35*x**4-30*x**2+3)-[p1]*sqrt(1-x**2)-[d1]*3*x*sqrt(1-x**2)+[d2]*3*(1-x**2)-[p1]*sqrt(1-x**2)-[d1]*3*x*sqrt(1-x**2)+[f1]*(3/2)*(1-5*x**2)*sqrt(1-x**2)+
  fit->SetParameters(80,30,100,1,1,100,1,1,40,100,1);
  for(int i=0;i<11;i++)
    {  //fit->SetParameter(i,200);
      fit->SetParLimits(i,0,200);}
  gStyle->SetOptFit(1);
  TF1* expo1 = new TF1("expo1","expo");
  TF1* expo2 = new TF1("expo2","expo");
  fit->SetLineColor(kAzure);
  fit->SetLineWidth(3);
  expo1->SetLineColor(kAzure);
  expo1->SetLineStyle(2);
  expo1->SetLineWidth(2);
  expo2->SetLineColor(kAzure);
  expo2->SetLineStyle(2);
  expo2->SetLineWidth(2);
  //fit and plot
  merged_hist->Fit("fit", "RL");
  Double_t *params = fit->GetParameters();
  expo1->SetParameters(params[0],params[1]);
  expo2->SetParameters(params[2],params[3]);
  //expo1->Draw("same");
  //expo2->Draw("same");
  
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
  gStyle->SetPadLeftMargin  (0.13);
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
