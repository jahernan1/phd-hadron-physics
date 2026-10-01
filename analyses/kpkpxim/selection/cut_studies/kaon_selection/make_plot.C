#include "gxana/common/Style.h"
/*-------------------------------------------------------------------------------------------------
  [Jesse A. Hernandez]
  This macros will take a root file as an argument and plot it's content with user-specific format
--------------------------------------------------------------------------------------------------*/
//instantiate methods used in main
void make_kp1kp2_plot(vector<TH1D*> vec_hist, string save_name, string leg_title="GlueX-I");
void make_1d_plot(vector<TH1D*> vec_hist, string save_name, string leg_title, string delim, Double_t cutVal, string plotTitle);
void style_format();
TH1D* combine_hist(vector<TH1D*> vec_hist);
TH1D* combine_hist_proj(vector<TH2D*> vec_hist);
TH2D* combine_2dhist(vector<TH2D*> vec_hist);
void get_plots(string delim="_vertexCuts");

// main function (change main name when copying)
int make_plot()
{
    //style function
    style_format();
    //make the root trees with different cuts
    //get_plots("_tCut");  
    get_plots("_ximVertexCut");
    get_plots("_kphighrap");
  
    return 0;
}

void get_plots(string delim="_vertexCuts")
{
    //initiate variables
    vector<TH2D*> vect_histo_2017;
    vector<TH2D*> vect_histo_201801;
    vector<TH2D*> vect_histo_201808;
    vector<TH1D*> vect_histo_momsep;
    vector<TH1D*> vect_histo_momsep_mc;
    vector<string> vect_histos = {"kp12_p_qval", "kp12_p_mc", 
                                  "kp12_p3_theta_qval", "kp12_p3_theta_mc",
                                  "kphigh_p3_theta_qval", "kphigh_p3_theta_mc",
                                  "kplow_p3_theta_qval", "kplow_p3_theta_mc",
                                  "kphigh_truth_p3_mc", "kplow_truth_p3_mc",
                                  "kp1_p3_theta_thrown", "kp2_p3_theta_thrown",
                                  "kpHighLowSep_qval", "kpHighLowSep_mc"};
    TCanvas *arr_can[vect_histos.size()*3];
  
    // Load root tree of choice
    TFile *f = TFile::Open( ("data"+delim+"_RF.root").c_str());
  
    for(int i=0; i < vect_histos.size(); i++)
        {
            if(i<12)
                {
                    arr_can[3*i] = new TCanvas("arr_can"," ");
                    vect_histo_2017.push_back( (TH2D*)f->Get( ("Spring_2017/"+vect_histos[i]).c_str() )->Clone() );
                    vect_histo_2017[i]->Draw("colz");
          
                    arr_can[3*i+1] = new TCanvas("arr_can1"," ");
                    vect_histo_201801.push_back( (TH2D*)f->Get( ("Spring_2018/"+vect_histos[i]).c_str() )->Clone() );
                    vect_histo_201801[i]->Draw("colz");
          
                    arr_can[3*i+2] = new TCanvas("arr_can2"," ");
                    vect_histo_201808.push_back( (TH2D*)f->Get( ("Fall_2018/"+vect_histos[i]).c_str() )->Clone() );
                    vect_histo_201808[i]->Draw("colz");
        
      
                    if(i == 8 || i == 9)
                        {
                            arr_can[3*i]->SetLogz();
                            arr_can[3*i]->Update();
                            arr_can[3*i+1]->SetLogz();
                            arr_can[3*i+1]->Update();
                            arr_can[3*i+2]->SetLogz();
                            arr_can[3*i+2]->Update();
                            arr_can[3*i]->SaveAs( ("plots/"+vect_histos[i]+"_2017"+delim+".pdf").c_str() );
                            arr_can[3*i+1]->SaveAs( ("plots/"+vect_histos[i]+"_201801"+delim+".pdf").c_str() );
                            arr_can[3*i+2]->SaveAs( ("plots/"+vect_histos[i]+"_201808"+delim+".pdf").c_str() );
                        }
                    else
                        {
                            arr_can[3*i]->SaveAs( ("plots/"+vect_histos[i]+"_2017"+delim+".pdf").c_str() );
                            arr_can[3*i+1]->SaveAs( ("plots/"+vect_histos[i]+"_201801"+delim+".pdf").c_str() );
                            arr_can[3*i+2]->SaveAs( ("plots/"+vect_histos[i]+"_201808"+delim+".pdf").c_str() );
                        }
                }
            else if(i==12)
                {
                    vect_histo_momsep.push_back( (TH1D*)f->Get( ("Spring_2017/"+vect_histos[i]).c_str() )->Clone() );
                    vect_histo_momsep.push_back( (TH1D*)f->Get( ("Spring_2018/"+vect_histos[i]).c_str() )->Clone() );
                    vect_histo_momsep.push_back( (TH1D*)f->Get( ("Fall_2018/"+vect_histos[i]).c_str() )->Clone() );
                }
            else if(i==13)
                {
                    vect_histo_momsep_mc.push_back( (TH1D*)f->Get( ("Spring_2017/"+vect_histos[i]).c_str() )->Clone() );
                    vect_histo_momsep_mc.push_back( (TH1D*)f->Get( ("Spring_2018/"+vect_histos[i]).c_str() )->Clone() );
                    vect_histo_momsep_mc.push_back( (TH1D*)f->Get( ("Fall_2018/"+vect_histos[i]).c_str() )->Clone() );
                }
        }

    // Make plot kp1kp2_kphigh_kplow
    TH1D* hist_kp1kp2_p = (TH1D*) combine_hist_proj({vect_histo_2017[2],vect_histo_201801[2],vect_histo_201808[2]} )->Clone();
    TH1D* hist_kp1kp2_p_mc = (TH1D*) combine_hist_proj({vect_histo_2017[3],vect_histo_201801[3],vect_histo_201808[3]} )->Clone();
    TH1D* hist_kphigh_p = (TH1D*) combine_hist_proj({vect_histo_2017[4],vect_histo_201801[4],vect_histo_201808[4]} )->Clone();
    TH1D* hist_kphigh_p_mc = (TH1D*) combine_hist_proj({vect_histo_2017[5],vect_histo_201801[5],vect_histo_201808[5]} )->Clone();
    TH1D* hist_kplow_p = (TH1D*) combine_hist_proj({vect_histo_2017[6],vect_histo_201801[6],vect_histo_201808[6]} )->Clone();
    TH1D* hist_kplow_p_mc = (TH1D*) combine_hist_proj({vect_histo_2017[7],vect_histo_201801[7],vect_histo_201808[7]} )->Clone();
    make_kp1kp2_plot({hist_kp1kp2_p,hist_kphigh_p,hist_kplow_p},"kp1kp2_kphigh_kplow_p"+delim);
    make_kp1kp2_plot({hist_kp1kp2_p_mc,hist_kphigh_p_mc,hist_kplow_p_mc},"kp1kp2_kphigh_kplow_p_mc"+delim,"Simulated");
    // Make 2D plot with cuts for kp1_kp2
    TCanvas *can = new TCanvas("can","");
    TH2D* kp1kp2_combined = (TH2D*) combine_2dhist({vect_histo_2017[0],vect_histo_201801[0],vect_histo_201808[0]})->Clone();
    kp1kp2_combined->Draw("colz");
    can->SaveAs(("kp1_kp2_p3"+delim+"_phase1.pdf").c_str());
    //
    TCanvas *canMC = new TCanvas("canMC","");
    TH2D* kp1kp2_mc_combined = (TH2D*) combine_2dhist({vect_histo_2017[1],vect_histo_201801[1],vect_histo_201808[1]})->Clone();
    kp1kp2_mc_combined->Draw("colz");
    canMC->SaveAs(("kp1_kp2_p3_mc"+delim+"_phase1.pdf").c_str());
    //
    TCanvas *canHigh = new TCanvas("canHigh","");
    TH2D* kphigh_theta_combined = (TH2D*) combine_2dhist({vect_histo_2017[4],vect_histo_201801[4],vect_histo_201808[4]})->Clone();
    kphigh_theta_combined->Draw("colz");
    canHigh->SaveAs(("kphigh_theta"+delim+"_phase1.pdf").c_str());
    //
    TCanvas *canLow = new TCanvas("canLow","");
    TH2D* kplow_theta_combined = (TH2D*) combine_2dhist({vect_histo_2017[6],vect_histo_201801[6],vect_histo_201808[6]})->Clone();
    kplow_theta_combined->Draw("colz");
    canLow->SaveAs(("kplow_theta"+delim+"_phase1.pdf").c_str());
    //
    TCanvas *canHighMC = new TCanvas("canHighMC","");
    TH2D* kphigh_theta_mc_combined = (TH2D*) combine_2dhist({vect_histo_2017[5],vect_histo_201801[5],vect_histo_201808[5]})->Clone();
    kphigh_theta_mc_combined->Draw("colz");
    canHighMC->SaveAs(("kphigh_theta"+delim+"_phase1_mc.pdf").c_str());
    //
    TCanvas *canLowMC = new TCanvas("canLowMC","");
    TH2D* kplow_theta_mc_combined = (TH2D*) combine_2dhist({vect_histo_2017[7],vect_histo_201801[7],vect_histo_201808[7]})->Clone();
    kplow_theta_mc_combined->Draw("colz");
    canLowMC->SaveAs(("kplow_theta"+delim+"_phase1_mc.pdf").c_str());
    //
    TCanvas *canHighTruth = new TCanvas("canHighTruth","");
    TH2D* kphigh_truth_combined = (TH2D*) combine_2dhist({vect_histo_2017[8],vect_histo_201801[8],vect_histo_201808[8]})->Clone();
    kphigh_truth_combined->Draw("colz");
    canHighTruth->SaveAs(("kphigh_truth"+delim+"_phase1.pdf").c_str());
    //
    TCanvas *canLowTruth = new TCanvas("canLowTruth","");
    TH2D* kplow_truth_combined = (TH2D*) combine_2dhist({vect_histo_2017[9],vect_histo_201801[9],vect_histo_201808[9]})->Clone();
    kplow_truth_combined->Draw("colz");
    canLowTruth->SaveAs(("kplow_truth"+delim+"_phase1.pdf").c_str());
    //
    //Make 1d momentum seperation plot
    TH1D* hist_kpmomsep = (TH1D*)combine_hist( vect_histo_momsep )->Clone();
    TH1D* hist_kpmomsep_mc = (TH1D*)combine_hist( vect_histo_momsep_mc )->Clone();
    make_1d_plot({hist_kpmomsep,hist_kpmomsep_mc},"kp_momsep_gluex-I","GlueX#lower[-0.15]{-}#kern[0.1]{I}",delim,0,
                 "; #left|#vec{#bf{p}}(K^{+}_{#it{#lower[-0.3]{fast}}})#right|-#left|#vec{#bf{p}}(K^{+}_{#it{#lower[-0.4]{slow}}})#right| (GeV); arb. units");
    //
    make_1d_plot({hist_kphigh_p,hist_kphigh_p_mc},"kphigh_p_gluex-I","GlueX#lower[-0.15]{-}#kern[0.1]{I}",delim,0,
                 "; #left|#vec{#bf{p}}(K^{+}_{#it{#lower[-0.3]{fast}}})#right| (GeV); arb. units");
    make_1d_plot({hist_kplow_p,hist_kplow_p_mc},"kplow_p_gluex-I","GlueX#lower[-0.15]{-}#kern[0.1]{I}",delim,0,
                 "; #left|#vec{#bf{p}}(K^{+}_{#it{#lower[-0.4]{slow}}})#right| (GeV); arb. units");
  
    f->Close();

    return;  // gxana: ROOT 6.40 rejects `return 0;` in a void function
}

// make plot to be called by main function
/* Arguments Description:
   hist: vector of histograms to plot
   legend_title: title of legend (name of data set)
*/
void make_kp1kp2_plot(vector<TH1D*> vec_hist, string save_name, string leg_title="GlueX-I")
{
    // Plot histogram(s) algorithm 
    TCanvas *c = new TCanvas("leg_title","leg_title");
    c->SetGrid(1,1);
    c->SetRightMargin (0.05);
    c->SetLeftMargin (0.12);
    c->SetBottomMargin (0.16);

    //histogram styling
    vec_hist[0]->SetLineColor(kBlack);
    vec_hist[0]->SetLineWidth(3);
    vec_hist[0]->SetMarkerStyle(24);
    vec_hist[0]->SetTitle( "; #left|#vec{#bf{p}}(K^{+})#right| (GeV); Events" );
    vec_hist[0]->GetYaxis()->SetMaxDigits(3);
    vec_hist[0]->GetXaxis()->SetTitleOffset(1.);
    vec_hist[0]->GetYaxis()->SetTitleOffset(0.85);
    vec_hist[0]->SetMinimum(0);
    vec_hist[0]->Draw("e1");
    //
    vec_hist[1]->SetLineColor(38);
    vec_hist[1]->SetLineWidth(2);
    vec_hist[1]->SetFillStyle(3354);
    vec_hist[1]->SetFillColor(38);
    vec_hist[1]->Draw("hist same");
    //
    vec_hist[2]->SetLineColor(46);
    vec_hist[2]->SetLineWidth(2);
    vec_hist[2]->SetFillStyle(3345);
    vec_hist[2]->SetFillColor(46);
    vec_hist[2]->Draw("hist same");
  
    //Draw cut line and arrow
    auto l = new TLine(2.3,0,2.3,vec_hist[0]->GetMaximum()*1.07);
    l->SetLineColor(kBlack);
    l->SetLineWidth(2);
    l->SetLineStyle(2);
    l->Draw();

    auto lar = new TArrow(1,vec_hist[0]->GetMaximum(),2.3,vec_hist[0]->GetMaximum(),0.03,"<|");
    lar->SetLineColor(46);
    lar->SetLineWidth(3);
    lar->SetFillColor(46);
    //lar->Draw();
   
    auto lar1 = new TArrow(2.3,vec_hist[0]->GetMaximum()/3,5,vec_hist[0]->GetMaximum()/3,0.03,"|>");
    lar1->SetLineColor(38);
    lar1->SetLineWidth(3);
    lar1->SetFillColor(38);
    //lar1->Draw();

    // Draw legend
    auto legend = new TLegend(0.617,0.63,0.9,0.94); //top right corner
    legend->SetBorderSize(0);
    legend->SetFillStyle(0);
    legend->SetTextSize(0.065);
    legend->SetHeader(leg_title.c_str(),"L"); // option "C" allows to center the header
    legend->AddEntry(vec_hist[0], "K^{+}_{1,2}", "lep");
    legend->AddEntry(vec_hist[1], "K^{+}_{#it{f}}", "f");
    legend->AddEntry(vec_hist[2], "K^{+}_{#it{s}}", "f");
    legend->Draw("same");
  
    //Save plot
    gPad->Update();
    c->SaveAs((save_name+".pdf").c_str());
    c->Close();
}
//
// make plot to be called by main function
/* Arguments Description:
   hist: vector of histograms to plot
   legend_title: title of legend (name of data set)
*/
void make_1d_plot(vector<TH1D*> vec_hist, string save_name, string leg_title, string delim, Double_t cutVal, string plotTitle)
{
    // Plot histogram(s) algorithm 
    TCanvas *c = new TCanvas("save_name","save_name");
    THStack *hs = new THStack("save_name","");
  
    double pxmax = c->GetUxmax();
    double pxmin = c->GetUxmax();
    double pymax = c->GetUymax();
    double pymin = c->GetUymax();

    //vec_hist[0]->SetFillStyle(1001);
    vec_hist[0]->SetMarkerStyle(20);
    vec_hist[0]->SetMarkerColor(kBlack);
    vec_hist[0]->SetLineColor(kBlack);
    vec_hist[0]->RebinX();
  
    vec_hist[1]->SetFillStyle(1001);
    vec_hist[1]->SetFillColorAlpha(kAzure-9,0.6);
    vec_hist[1]->SetLineColor(kBlack);
    vec_hist[1]->RebinX();
    //double scale_factor = vec_hist[0]->GetMaximum() / vec_hist[1]->GetMaximum();
    double scale_factor = vec_hist[0]->Integral("width") / vec_hist[1]->Integral("width");
    vec_hist[1]->Scale(scale_factor);
    hs->Add(vec_hist[1],"hist");
    hs->Add(vec_hist[0],"e1");
  
    hs->Draw("nostack");
    hs->SetTitle(plotTitle.c_str());
    hs->GetYaxis()->SetMaxDigits(3);
    hs->SetMaximum(vec_hist[0]->GetMaximum()*1.1);

    // Draw legend
    auto legend = new TLegend(pxmin*0.59,pymin*0.74,pxmax*0.84,pymax*0.92); //top right corner
    legend->SetBorderSize(0);
    legend->SetFillStyle(0);
    legend->SetTextSize(0.06);
    legend->SetHeader(leg_title.c_str(),"C"); // option "C" allows to center the header
    legend->AddEntry(vec_hist[0], "Data","lep");
    //legend->AddEntry(vec_hist[0], "Data (Q-Value)", "f");
    legend->AddEntry(vec_hist[1], "Simulation", "f");
    legend->Draw("same");
  
    //Draw cut line and arrow
    if(cutVal>0)
        {
            auto l = new TLine(cutVal, 0, cutVal, vec_hist[1]->GetMaximum()*1.05);
            l->SetLineColor(kBlue);
            l->SetLineWidth(4);
            l->Draw("same");
            auto lar = new TArrow(cutVal ,vec_hist[1]->GetMaximum()*0.7, 4, vec_hist[1]->GetMaximum()*0.7, 0.05, "|>");
            lar->SetLineColor(kBlue);
            lar->SetLineWidth(4);
            lar->SetFillColor(kBlue);
            lar->Draw();
        }
    //Save plot
    gPad->Update();
    c->SaveAs((save_name+delim+".pdf").c_str());
    c->Close();
}
//
TH1D* combine_hist(vector<TH1D*> vec_hist)
{
    TH1D* combined_hist = (TH1D*)vec_hist[0]->Clone(); 
    for(Int_t loc_hist=1; loc_hist < vec_hist.size(); loc_hist++)
        {
            TH1D* temp = (TH1D*)vec_hist[loc_hist]->Clone();
            combined_hist->Add(temp);
        }
  
    return combined_hist;
}
//
TH1D* combine_hist_proj(vector<TH2D*> vec_hist)
{
    TH1D* combined_hist = (TH1D*)vec_hist[0]->ProjectionY()->Clone(); 
    for(Int_t loc_hist=1; loc_hist < vec_hist.size(); loc_hist++)
        {
            TH1D* temp = (TH1D*)vec_hist[loc_hist]->ProjectionY()->Clone();
            combined_hist->Add(temp);
        }
  
    return combined_hist;
}
//
TH2D* combine_2dhist(vector<TH2D*> vec_hist)
{
    TH2D* combined_hist = (TH2D*)vec_hist[0]->Clone(); 
    for(Int_t loc_hist=1; loc_hist < vec_hist.size(); loc_hist++)
        {
            TH2D* temp = (TH2D*)vec_hist[loc_hist]->Clone();
            combined_hist->Add(temp);
        }
  
    return combined_hist;
}
//
// input specific style formatting of user choice
void style_format()
{
    gxana::StyleParams p = gxana::DistributionStyle();
    p.padBottomMargin = 0.18;
    p.padTopMargin = 0.05;
    p.padLeftMargin = 0.16;
    p.padRightMargin = 0.12;
    gxana::ApplyStyle(p);
}
