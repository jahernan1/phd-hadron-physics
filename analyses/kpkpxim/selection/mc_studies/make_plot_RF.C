#include "gxana/common/Style.h"
/*-------------------------------------------------------------------------------------------------
  [Jesse A. Hernandez]
  This macros will take a root file as an argument and plot it's content with user-specific format
--------------------------------------------------------------------------------------------------*/
#include <stdio.h>

//instantiate methods used in main
void make_plot(vector<TH1D*> vec_hist, string save_name, string leg_title, Bool_t make_leg = true);
void merge_plot(vector<TH1D*> vec_hist, string save_name, string leg_title,string axis_title, Bool_t make_leg = true);
void compare_plot(vector<TH1D*> vec_hist, string save_name, string leg_title, string axis_title, Bool_t make_leg = true, vector<string> leg_entry={"Data (eff. corr.)", "Gen MC"});
void compare_plot_log(vector<TH1D*> vec_hist, string save_name, string leg_title, string axis_title, Bool_t make_leg = true);
void style_format();

// main function (change main name when copying)
int make_plot_RF()
{
    // call plot styling function
    style_format();
  
    //initiate variables
    vector<TH1D*> vect_histo_accept;
    vector<TH1D*> vect_histo_mass;
    vector<TH1D*> vect_histo_cos;
    
    // Load root tree of choice
    TFile *f = TFile::Open( "data_ac_hist2d_kphighrap_2d.root");
  
    // Get 2D distributions
    TH2D* costheta_2d_ac = (TH2D*)f->Get("CosThetaVsMass_qval_ac")->Clone();
    TH2D* costheta_2d_qval = (TH2D*)f->Get("CosThetaVsMass_qval")->Clone();
    TH2D* costheta_2d_mc = (TH2D*)f->Get("CosThetaVsMass_mc")->Clone();
    TH2D* costheta_2d_thrown = (TH2D*)f->Get("CosThetaVsMass_thrown")->Clone();
    //
    TH2D* costheta_ystarM_ac = (TH2D*)f->Get("costheta_ystarM_all_acceptcorr")->Clone();
    TH2D* costheta_ystarM_qval = (TH2D*)f->Get("costheta_ystar_all_qval")->Clone();
    TH2D* costheta_ystarM_mc = (TH2D*)f->Get("costheta_ystar_all_mc")->Clone();
    TH2D* costheta_ystarM_thrown = (TH2D*)f->Get("costheta_ystar_all_thrown")->Clone();
    //
    TH2D* hist2017 = (TH2D*)f->Get("Spring_2017/costheta_gen_amp_ystarM_qval")->Clone();
    TH2D* hist2017_mc = (TH2D*)f->Get("Spring_2017/costheta_gen_amp_ystarM_mc")->Clone();
    vect_histo_mass.push_back( (TH1D*)hist2017->ProjectionX() );
    vect_histo_mass.push_back( (TH1D*)hist2017_mc->ProjectionX() );
    vect_histo_cos.push_back( (TH1D*)hist2017->ProjectionY() );
    vect_histo_cos.push_back(  (TH1D*)hist2017->ProjectionY() );
    //
    TH2D* hist201801 = (TH2D*)f->Get("Spring_2018/costheta_gen_amp_ystarM_qval")->Clone();
    TH2D* hist201801_mc = (TH2D*)f->Get("Spring_2018/costheta_gen_amp_ystarM_mc")->Clone();
    vect_histo_mass.push_back( (TH1D*)hist201801->ProjectionX() );
    vect_histo_mass.push_back( (TH1D*)hist201801_mc->ProjectionX() );
    vect_histo_cos.push_back( (TH1D*)hist201801->ProjectionY() );
    vect_histo_cos.push_back( (TH1D*)hist201801_mc->ProjectionY() );
    //
    TH2D* hist201808 = (TH2D*)f->Get("Fall_2018/costheta_gen_amp_ystarM_qval")->Clone();
    TH2D* hist201808_mc = (TH2D*)f->Get("Fall_2018/costheta_gen_amp_ystarM_mc")->Clone();
    vect_histo_mass.push_back( (TH1D*)hist201808->ProjectionX() );
    vect_histo_mass.push_back( (TH1D*)hist201808_mc->ProjectionX() );
    vect_histo_cos.push_back( (TH1D*)hist201808->ProjectionY() );
    vect_histo_cos.push_back( (TH1D*)hist201808_mc->ProjectionY() );
    //
    TH1D* tdist_phase1_ac = (TH1D*)f->Get("tdist_all_acceptcorr")->Clone();
    TH1D* tdist_phase1_thrown = (TH1D*)f->Get("tdist_all_thrown")->Clone() ;
    TH1D* costheta_phase1_ac = (TH1D*)costheta_ystarM_ac->ProjectionX() ;
    TH1D* costheta_phase1_qval = (TH1D*)costheta_ystarM_qval->ProjectionX() ;
    TH1D* costheta_phase1_mc = (TH1D*)costheta_ystarM_mc->ProjectionX() ;
    TH1D* costheta_phase1_thrown = (TH1D*)costheta_ystarM_thrown->ProjectionX() ;
    TH1D* ystarM_phase1_ac = (TH1D*)costheta_ystarM_ac->ProjectionY() ;
    TH1D* ystarM_phase1_qval = (TH1D*)costheta_ystarM_qval->ProjectionY() ;
    TH1D* ystarM_phase1_mc = (TH1D*)costheta_ystarM_mc->ProjectionY() ;
    TH1D* ystarM_phase1_thrown = (TH1D*)costheta_ystarM_thrown->ProjectionY() ;
   
    TH1D* costheta_phase1_ac_py = (TH1D*)costheta_ystarM_ac->ProjectionY()->Clone() ;
    TH1D* costheta_phase1_py = (TH1D*)costheta_ystarM_qval->ProjectionY()->Clone() ;
    TH1D* costheta_phase1_thrown_py = (TH1D*)costheta_ystarM_thrown->ProjectionY()->Clone() ;
    TH1D* costheta_phase1_mc_py = (TH1D*)costheta_ystarM_mc->ProjectionY()->Clone() ;
    TH1D* ystar_phase1_ac_px = (TH1D*)costheta_ystarM_ac->ProjectionX()->Clone() ;
    TH1D* ystar_phase1_px = (TH1D*)costheta_ystarM_qval->ProjectionX()->Clone() ;
    TH1D* ystar_phase1_thrown_px = (TH1D*)costheta_ystarM_thrown->ProjectionX()->Clone() ;
    TH1D* ystar_phase1_mc_px = (TH1D*)costheta_ystarM_mc->ProjectionX()->Clone() ;
  
    // Call function to plot
    //make_plot(vect_histo_accept, "t_dist_accept_phase1", "GlueX-I");
    //merge_plot(vect_histo_mass, "ystar_M_phase1_mc_data_ac_kphighrap", "GlueX#lower[-0.15]{-}#kern[0.1]{I}", "; M(#Xi^{-}K^{+}_{#it{slow}}) (GeV) ; arb. units");
    //merge_plot(vect_histo_cos, "costheta_gen_amp_phase1_mc_data_ac_kphighrap", "GlueX#lower[-0.15]{-}#kern[0.1]{I}", "; cos#vartheta_{#it{h^{#lower[0.1]{*}}}}^{#Xi^{-}} ; arb. units",false);
    compare_plot({costheta_phase1_ac,costheta_phase1_thrown}, "costheta_gen_amp_phase1_data_thrown_ac_2d_kphighrap","GlueX#lower[-0.15]{-}#kern[0.1]{I}", "; cos#vartheta_{#it{h^{#lower[0.1]{*}}}}^{#Xi^{-}} ;arb. units",false);
    compare_plot({ystarM_phase1_ac,ystarM_phase1_thrown}, "ystarM_phase1_data_thrown_ac_2d_kphighrap","GlueX#lower[-0.15]{-}#kern[0.1]{I}", "; M(#Xi^{-}K^{+}_{#it{slow}}) (GeV) ; arb. units");
    compare_plot_log({tdist_phase1_ac,tdist_phase1_thrown}, "tdist_phase1_data_thrown_ac_2d_kphighrap","GlueX#lower[-0.15]{-}#kern[0.1]{I}", "; -t_{#lower[-0.2]{#gammaK^{+}_{#it{#lower[-0.3]{fast}}}}} (GeV^{2} ) ;arb. units");
    compare_plot({costheta_phase1_qval,costheta_phase1_mc}, "costheta_gen_amp_phase1_data_mc_2d_kphighrap","GlueX#lower[-0.15]{-}#kern[0.1]{I}", "; cos#vartheta_{#it{h^{#lower[0.1]{*}}}}^{#Xi^{-}} ; arb. units", false, {"Data","Recon MC"});
    compare_plot({ystarM_phase1_qval,ystarM_phase1_mc}, "ystarM_phase1_data_mc_2d_kphighrap","GlueX#lower[-0.15]{-}#kern[0.1]{I}", "; M(#Xi^{-}K^{+}_{#it{slow}}) (GeV) ; arb. units", true, {"Data","Recon MC"});
    compare_plot({costheta_phase1_ac_py,costheta_phase1_thrown_py}, "costheta_gen_amp_phase1_input_thrown_ac_2d_kphighrap","GlueX#lower[-0.15]{-}#kern[0.1]{I}", "; cos#vartheta_{#it{h^{#lower[0.1]{*}}}}^{#Xi^{-}} ;arb. units",false);
    compare_plot({ystar_phase1_ac_px,ystar_phase1_thrown_px}, "ystarM_phase1_input_thrown_ac_2d_kphighrap","GlueX#lower[-0.15]{-}#kern[0.1]{I}", "; M(#Xi^{-}K^{+}_{#it{slow}}) (GeV) ;arb. units");
  
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
    hs->SetTitle("; -t_{#lower[-0.2]{#gammaK^{+}_{#it{#lower[-0.3]{fast}}}}} (GeV^{2} ); Acceptance, #varepsilon");
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
    c->SaveAs((save_name+".pdf").c_str());
    //c->Print((save_name+".svg").c_str());
}

void merge_plot(vector<TH1D*> vec_hist, string save_name, string leg_title, string axis_title, Bool_t make_leg = true)
{
    // Plot histogram(s) algorithm 
    TH1D* merged_hist = (TH1D*)vec_hist[0]->Clone();
    TH1D* merged_hist_mc = (TH1D*)vec_hist[1]->Clone();
    merged_hist->Reset();
    merged_hist_mc->Reset();
  
    TCanvas *c = new TCanvas(leg_title.c_str(),leg_title.c_str()); 
    //c->SetGrid();

    double pxmax = c->GetUxmax();double pxmin = c->GetUxmax();
    double pymax = c->GetUymax();double pymin = c->GetUymax();

    TList *lst = new TList();
    TList *lstmc = new TList();
    for(Int_t i = 0; i < vec_hist.size()/2;i++ )
        {lst->Add(vec_hist[2*i]); lstmc->Add(vec_hist[2*i+1]);}

    merged_hist->Merge(lst);
    //merged_hist->SetFillColorAlpha(kGray,0.9);
    merged_hist->SetMarkerStyle(20);
    merged_hist->SetMarkerColor(kBlack);
    //merged_hist->SetMarkerSize(0.9);
    merged_hist->SetLineColor(kBlack);
    merged_hist_mc->GetYaxis()->SetMaxDigits(2);
  
    merged_hist_mc->Merge(lstmc);
    merged_hist_mc->SetFillColorAlpha(kAzure-9,0.6);
    merged_hist_mc->SetLineColor(kBlack);
    merged_hist_mc->Scale(merged_hist->Integral("width")/merged_hist_mc->Integral("width"));
    merged_hist_mc->SetMaximum(merged_hist->GetMaximum()*1.2);

    merged_hist_mc->GetYaxis()->SetRangeUser(0,merged_hist->GetMaximum()*1.1);
    merged_hist_mc->SetTitle(axis_title.c_str());
    merged_hist_mc->SetTitleOffset(0.9,"Y");
    merged_hist_mc->Draw( "hist ");
    merged_hist->Draw("e1 same");
    // Draw legend
    TLegend *legend;
    if(make_leg)
        legend = new TLegend(0.66,0.7,0.9,0.9); //top right corner
    else
        legend = new TLegend(0.20,0.7,0.44,0.9); //top right corner
  
    legend->SetBorderSize(0);
    legend->SetFillStyle(0);
    legend->SetTextSize(0.065);
    legend->SetHeader(leg_title.c_str(),"C"); // option "C" allows to center the header
    legend->AddEntry(merged_hist, "Data", "lep");
    //legend->AddEntry(vec_hist[0], "Data (Q-Value)", "f");
    legend->AddEntry(merged_hist_mc, "Recon MC", "f");
    legend->Draw("same");
    //Fit the merged hist

    //Save plot
    gPad->Update();
    c->SaveAs((save_name+".pdf").c_str());
}


void compare_plot(vector<TH1D*> vec_hist, string save_name, string leg_title, string axis_title, Bool_t make_leg = true, vector<string> leg_entry={})
{
    // Plot histogram(s) algorithm 
    TH1D* merged_hist = (TH1D*)vec_hist[0]->Clone();
    TH1D* merged_hist_mc = (TH1D*)vec_hist[1]->Clone();

    TCanvas *c = new TCanvas(leg_title.c_str(),leg_title.c_str());
  
    //c->SetGrid();
  
    //merged_hist->SetFillColorAlpha(kGray,0.9);
    merged_hist->SetMarkerStyle(20);
    merged_hist->SetMarkerColor(kBlack);
    //merged_hist->SetMarkerSize(0.9);
    merged_hist->SetLineColor(kBlack);
    merged_hist_mc->GetYaxis()->SetMaxDigits(4);

    if(leg_entry[0].compare("Data"))
        merged_hist_mc->SetFillColorAlpha(46,0.6);
    else
        merged_hist_mc->SetFillColorAlpha(kAzure-9,0.6);
    merged_hist_mc->SetLineColor(kBlack);
    merged_hist_mc->Scale(merged_hist->Integral("width")/merged_hist_mc->Integral("width"));
    //merged_hist_mc->SetMaximum(merged_hist->GetMaximum()*1.2);

    if(merged_hist->GetMaximum() > merged_hist_mc->GetMaximum())
        merged_hist_mc->GetYaxis()->SetRangeUser(0,merged_hist->GetMaximum()*1.1);
  
    merged_hist_mc->SetTitle(axis_title.c_str());
    merged_hist_mc->SetTitleOffset(1.02,"Y");

    merged_hist_mc->Draw( "hist ");
    merged_hist->Draw("e1 same");
  
    // Draw legend
    TLegend *legend;
    if(make_leg)
        legend = new TLegend(0.60,0.7,0.84,0.9); //top right corner
    else
        legend = new TLegend(0.20,0.7,0.44,0.9); //top right corner
  
    legend->SetBorderSize(0);
    legend->SetFillStyle(0);
    legend->SetTextSize(0.065);
    legend->SetHeader(leg_title.c_str(),"C"); // option "C" allows to center the header
    legend->AddEntry(merged_hist, leg_entry[0].c_str(), "lep");
    //legend->AddEntry(vec_hist[0], "Data (Q-Value)", "f");
    legend->AddEntry(merged_hist_mc, leg_entry[1].c_str(), "f");
    legend->Draw("same");
    //Fit the merged hist

    //Save plot
    gPad->Update();
    c->SaveAs((save_name+".pdf").c_str());
}

void compare_plot_log(vector<TH1D*> vec_hist, string save_name, string leg_title, string axis_title, Bool_t make_leg = true)
{
    // Plot histogram(s) algorithm 
    TH1D* merged_hist = (TH1D*)vec_hist[0]->Clone();
    TH1D* merged_hist_mc = (TH1D*)vec_hist[1]->Clone();
  
    TCanvas *c = new TCanvas(leg_title.c_str(),leg_title.c_str());
  
    c->SetGrid();
    c->SetLogy();
  
    merged_hist->SetMarkerStyle(20);
    merged_hist->SetLineColor(kBlack);
    merged_hist->GetYaxis()->SetMaxDigits(3);
    merged_hist->GetYaxis()->SetRangeUser(1,5e6);
  
    merged_hist_mc->SetMarkerStyle(24);
    merged_hist_mc->SetMarkerColor(46);
    merged_hist_mc->SetLineColor(46);
    merged_hist_mc->Scale(merged_hist->Integral("width")/merged_hist_mc->Integral("width"));
    //merged_hist->SetMaximum(merged_hist->GetMaximum()*2);

    //merged_hist->GetYaxis()->SetRangeUser(1,merged_hist->GetMaximum()*1.2);
    merged_hist->SetTitle(axis_title.c_str());
    merged_hist->SetTitleOffset(1.,"Y");
    merged_hist->Draw("e1");
    merged_hist_mc->Draw( "e1 same");

    //Fit the merged hist
    TF1 *fit = new TF1("fit","expo",0.3,5);
    fit->SetLineColor(kAzure);
    fit->SetLineStyle(kDashed);
    merged_hist_mc->Fit("fit","R");
    char str[100];
    sprintf(str,"Fit (b = %.2f)", -fit->GetParameter(1));
  
    // Draw legend
    TLegend *legend;
    if(make_leg)
        legend = new TLegend(0.58,0.64,0.89,0.91); //top right corner
    else
        legend = new TLegend(0.18,0.73,0.42,0.91); //top right corner

    legend->SetBorderSize(0);
    legend->SetFillStyle(0);
    legend->SetTextSize(0.065);
    legend->SetHeader(leg_title.c_str(),"C"); // option "C" allows to center the header
    legend->AddEntry(merged_hist, "Data (eff. corr.)", "lep");
    //legend->AddEntry(vec_hist[0], "Data (Q-Value)", "f");
    legend->AddEntry(merged_hist_mc, "Gen MC", "lep");
    legend->AddEntry(fit, str, "l");
    legend->Draw("same");
  
    TLatex* latex = new TLatex();
    latex->SetNDC();
    latex->SetTextFont(132);
    latex->SetTextSize(0.08);
    latex->SetTextAlign(32);

  
    // auto l = new TLine(2.4,0,2.4,merged_hist->GetMaximum()*1.9);
    // l->SetLineColor(kBlue);
    // l->SetLineWidth(3);
    // //l->SetLineStyle(2);
    // l->Draw();

    // auto lar = new TArrow(2.4,merged_hist->GetMaximum(),1,merged_hist->GetMaximum(),0.03,"|>");
    // lar->SetLineColor(kBlue);
    // lar->SetLineWidth(3);
    // lar->SetFillColor(kBlue);
    // lar->Draw();
  
    //Save plot
    gPad->Update();
    c->SaveAs((save_name+"_log.pdf").c_str());
}

// input specific style formatting of user choice
void style_format()
{
    gxana::StyleParams p = gxana::CutStudyStyle();
    p.padBottomMargin = 0.2;
    p.padRightMargin = 0.02;
    p.legendFillColor = 0;
    p.markerSize = 1.0;
    p.titleOffsetY = 0.8;
    gxana::ApplyStyle(p);
}
