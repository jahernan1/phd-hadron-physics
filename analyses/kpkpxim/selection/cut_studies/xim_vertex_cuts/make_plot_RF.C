/*-------------------------------------------------------------------------------------------------
  [Jesse A. Hernandez]
  This macros will take a root file as an argument and plot it's content with user-specific format
--------------------------------------------------------------------------------------------------*/
//instantiate methods used in main
void makePlot(string delim="_allKaonSep");
void get_plot(vector<TH1D*> vec_hist, string save_name, string leg_title, Double_t cut_val = 2);
void load_histos(TFile *f, string dir, vector<TH1D*> vect_histo);
void style_format();

// main function (change main name when copying)
int make_plot_RF()
{
  //makePlot("_vertexCuts");
  makePlot("_kphighrap");
  return 0;
}
//
void makePlot(string delim="_allCuts")  
{
  // call plot styling function
  style_format();
  
  //initiate variables
  vector<TH1D*> vect_histo_2017;
  vector<TH1D*> vect_histo_201801;
  vector<TH1D*> vect_histo_201808;

  // Load root tree of choice
  TFile *f = TFile::Open( ("data"+delim+".root").c_str());
  vect_histo_2017.push_back( (TH1D*)f->Get("Spring_2017/xim_pathlensig_novertex_qval")->Clone() );
  vect_histo_2017.push_back( (TH1D*)f->Get("Spring_2017/xim_pathlensig_novertex_mc" )->Clone() );
   
  vect_histo_2017.push_back( (TH1D*)f->Get("Spring_2017/xim_pathlen_diff_qval")->Clone() );
  vect_histo_2017.push_back( (TH1D*)f->Get("Spring_2017/xim_pathlen_diff_mc" )->Clone() );
  vect_histo_2017.push_back( (TH1D*)f->Get("Spring_2017/xim_pathlen_diff_novertex_qval")->Clone() );
  vect_histo_2017.push_back( (TH1D*)f->Get("Spring_2017/xim_pathlen_diff_novertex_mc")->Clone() );
  //
  vect_histo_201801.push_back( (TH1D*)f->Get("Spring_2018/xim_pathlensig_novertex_qval")->Clone() );
  vect_histo_201801.push_back( (TH1D*)f->Get("Spring_2018/xim_pathlensig_novertex_mc" )->Clone() );
   
  vect_histo_201801.push_back( (TH1D*)f->Get("Spring_2018/xim_pathlen_diff_qval")->Clone() );
  vect_histo_201801.push_back( (TH1D*)f->Get("Spring_2018/xim_pathlen_diff_mc" )->Clone() );
  vect_histo_201801.push_back( (TH1D*)f->Get("Spring_2018/xim_pathlen_diff_novertex_qval")->Clone() );
  vect_histo_201801.push_back( (TH1D*)f->Get("Spring_2018/xim_pathlen_diff_novertex_mc")->Clone() );
  //
  vect_histo_201808.push_back( (TH1D*)f->Get("Fall_2018/xim_pathlensig_novertex_qval")->Clone() );
  vect_histo_201808.push_back( (TH1D*)f->Get("Fall_2018/xim_pathlensig_novertex_mc" )->Clone() );
   
  vect_histo_201808.push_back( (TH1D*)f->Get("Fall_2018/xim_pathlen_diff_qval")->Clone() );
  vect_histo_201808.push_back( (TH1D*)f->Get("Fall_2018/xim_pathlen_diff_mc" )->Clone() );
  vect_histo_201808.push_back( (TH1D*)f->Get("Fall_2018/xim_pathlen_diff_novertex_qval")->Clone() );
  vect_histo_201808.push_back( (TH1D*)f->Get("Fall_2018/xim_pathlen_diff_novertex_mc")->Clone() );

  // Call function to plot 
  get_plot({vect_histo_2017.begin(),vect_histo_2017.begin() + 2},
           "xim_pathlensig_data_mc"+delim+"_2017", "Spring 2017");
  get_plot({vect_histo_2017.begin() + 2,vect_histo_2017.begin() + 4},
           "xim_pathlendiff_data_mc"+delim+"_2017", "Spring 2017",0);
  get_plot({vect_histo_2017.begin() + 4,vect_histo_2017.end()},
           "xim_pathlendiff_nocut_data_mc"+delim+"_2017", "Spring 2017",0);
  //
  get_plot({vect_histo_201801.begin(),vect_histo_201801.begin() + 2},
           "xim_pathlensig_data_mc"+delim+"_201801", "Spring 2018");
  get_plot({vect_histo_201801.begin() + 2,vect_histo_201801.begin() + 4},
           "xim_pathlendiff_data_mc"+delim+"_201801", "Spring 2018",0);
  get_plot({vect_histo_201801.begin() + 4,vect_histo_201801.end()},
           "xim_pathlendiff_nocut_data_mc"+delim+"_201801", "Spring 2018",0);
  //
  get_plot({vect_histo_201808.begin(),vect_histo_201808.begin() + 2},
           "xim_pathlensig_data_mc"+delim+"_201808", "Fall 2018");
  get_plot({vect_histo_201808.begin() + 2,vect_histo_201808.begin() + 4 },
           "xim_pathlendiff_data_mc"+delim+"_201808", "Fall 2018",0);
  get_plot({vect_histo_201808.begin() + 4,vect_histo_201808.end()},
           "xim_pathlendiff_nocut_data_mc"+delim+"_201808", "Fall 2018",0);
  
  // TCanvas* c = new TCanvas("c","c");
  // vect_histo_201808[5]->SetFillColorAlpha(kGray,0.9);
  // vect_histo_201808[5]->GetYaxis()->SetMaxDigits(3);
  // vect_histo_201808[5]->GetXaxis()->SetRangeUser(-5,30);
  // //vect_histo_201808[5]->RebinX();
  // vect_histo_201808[5]->Draw();
  // c->SaveAs("xim_pathlenndiff_nocut_201808.pdf");
}

// make plot to be called by main function
/* Arguments Description:
hist: vector of histograms to plot
legend_title: title of legend (name of data set)
*/
void get_plot(vector<TH1D*> vec_hist, string save_name, string leg_title, Double_t cut_val = 2)
{
  // Plot histogram(s) algorithm 
  TCanvas *c = new TCanvas("leg_title","leg_title");
  THStack *hs = new THStack("leg_title","");
  
  //Set up Legend
  double pxmax = c->GetUxmax();
  double pxmin = c->GetUxmax();
  double pymax = c->GetUymax();
  double pymin = c->GetUymax();
  double scale_factor;
  auto legend = new TLegend(pxmin*0.55,pymin*0.73,pxmax*0.9,pymax*0.91); //top right corner
  legend->SetBorderSize(0);
  legend->SetTextSize(0.065);
  legend->SetHeader(leg_title.c_str(),"C"); // option "C" allows to center the header
  
  //mc data
  vec_hist[1]->SetFillColorAlpha(kAzure-9,0.6);
  vec_hist[1]->SetLineColor(kBlack);
  //data 
  //vec_hist[0]->SetFillColorAlpha(kGray,0.9);
  vec_hist[0]->SetLineColor(kBlack);
  //scale_factor = vec_hist[0]->GetMaximum() / vec_hist[1]->GetMaximum();
  scale_factor = vec_hist[0]->Integral("width") / vec_hist[1]->Integral("width");
  vec_hist[1]->Scale(scale_factor);
  
  hs->Add(vec_hist[1],"hist");
  hs->Add(vec_hist[0],"e1");
  //hs->SetTitle(stack_title.c_str());
  hs->Draw("nostack");
  hs->SetTitle("; Z_{#Xi^{-}} - Z_{#lower[-0.2]{#it{prod}}} (cm); arb. units");
  //hs->GetXaxis()->SetLimits(0,10);
  hs->GetYaxis()->SetMaxDigits(3);
  hs->SetMaximum(vec_hist[0]->GetMaximum()*1.15);
  // Draw legend
  legend->AddEntry(vec_hist[0], "Data", "lep");
  legend->AddEntry(vec_hist[1], "Recon MC", "f");
  legend->Draw("same");

  if(cut_val>0)
    {
      hs->SetTitle(";#Xi^{-} Flight Significance; arb. units");
      
      //Draw cut line and arrow
      auto l1 = new TLine(cut_val,0,cut_val,vec_hist[0]->GetMaximum()*1.15);
      l1->SetLineColor(kBlue);
      l1->SetLineWidth(4);
      auto lar = new TArrow(cut_val,vec_hist[0]->GetMaximum(),7,vec_hist[0]->GetMaximum(),0.045,"|>");
      lar->SetLineColor(kBlue);
      lar->SetLineWidth(4);
      lar->SetFillColor(kBlue);
    
      l1->Draw("same");
      lar->Draw();     
    }
  //Save plot
  gPad->Update();
  c->SaveAs((save_name+".pdf").c_str());
  //c->SaveAs((save_name+".png").c_str());
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
  gStyle->SetMarkerSize(0.8);
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
