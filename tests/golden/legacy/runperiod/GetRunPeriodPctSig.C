// FROZEN copy of the original run-period check AnalysisNote/systematics/GetRunPeriodPctSig.C, the
// reference of tests/golden/test_runperiod_golden.py. Only change: its two directory literals are the
// placeholders @DATA_DIR@ (the per-period cross-section tables) and @SAVE_DIR@ (where the
// ratio/significance PDFs go), which the test fills in. Do not edit.
// kpkpxim xsection clas and gluex data

#include "TF1.h"
#include "TH1.h"
#include "TH1D.h"
#include "TFile.h"
#include "TCanvas.h"
#include <stdio.h>
#include "RooPlot.h"

void SetStyle();
void make_plot(string delim, vector<string> rootFile, vector<string> setStr, vector<string> binStr, TH1D* hist_signif=nullptr);
void make_signif(TH1D* hist_signif, string hist_title, string save_name);
TGraphErrors* calc_signif(TGraphErrors *nominal, TGraphErrors *variation, TH1D* hist_signif=nullptr);
TGraphErrors* calc_pct(TGraphErrors *nominal, TGraphErrors *variation, TH1D* hist_signif);

//main  
int GetRunPeriodPctSig()
{
    SetStyle();
    vector<string> setStr = {"Spring-2017","Spring-2018", "Fall-2018", "GlueX-I"};
    vector<vector<double>> arrEnBins{{6.40,7.40},{7.40,7.86},{7.86,8.19},{8.19,8.45},{8.45,8.68},{8.68,9.26},{9.26,10.18},{10.18,11.40}};
    TH1D* hist_signif_s17_s18 = new TH1D("signif_s17",";#sigma_{sig};",14,-4,4);
    TH1D* hist_signif_s17_f18 = new TH1D("signif_s18",";#sigma_{sig};",14,-4,4);
    TH1D* hist_signif_s18_f18 = new TH1D("signif_f18",";#sigma_{sig};",14,-4,4);
    TH1D* hist_ratio_s17_s18 = new TH1D("ratio_s17",";ratio;",25,0.,2);
    TH1D* hist_ratio_s17_f18 = new TH1D("ratio_s18",";ratio;",25,0.,2);
    TH1D* hist_ratio_s18_f18 = new TH1D("ratio_f18",";ratio;",25,0.,2);
    
    //
    for(int i=0; i<arrEnBins.size(); i++)
        { 
            //Get barlow for variations for 3 run periods
            string xmin = to_string(arrEnBins[i][0]); string xmax = to_string(arrEnBins[i][1]);
            string xminCut = xmin.substr(0,xmin.find_first_of(".")+3);
            string xmaxCut = xmax.substr(0,xmax.find_first_of(".")+3);
            make_plot("_emin_"+xminCut+"_emax_"+xmaxCut, {"_flatTree_kpkpxim__M23_2017-01_ana56","_flatTree_kpkpxim__B4_M23_2018-01_ana03"}, {setStr[0],setStr[1]}, {xminCut, xmaxCut},hist_ratio_s17_s18);
            make_plot("_emin_"+xminCut+"_emax_"+xmaxCut, {"_flatTree_kpkpxim__M23_2017-01_ana56","_flatTree_kpkpxim__B4_M23_2018-08_ana02"}, {setStr[0],setStr[2]}, {xminCut, xmaxCut},hist_ratio_s17_f18);
            make_plot("_emin_"+xminCut+"_emax_"+xmaxCut, {"_flatTree_kpkpxim__B4_M23_2018-01_ana03","_flatTree_kpkpxim__B4_M23_2018-08_ana02"}, {setStr[1],setStr[2]}, {xminCut, xmaxCut},hist_ratio_s18_f18);
        }
    gStyle->SetOptFit(1);
    make_signif(hist_ratio_s17_s18,"Spring '17: Spring '18;ratio; counts","ratio_s17_s18_gaus.pdf");
    make_signif(hist_ratio_s17_f18,"Spring '17: Fall '18;ratio; counts","ratio_s17_f18_gaus.pdf");
    make_signif(hist_ratio_s18_f18,"Spring '18: Fall '18;ratio; counts","ratio_s18_f18_gaus.pdf");
    
    return 0;
}

void make_signif(TH1D* hist_signif, string hist_title, string save_name)
{
    TCanvas *c = new TCanvas("c");
    TF1* fit = new TF1("fit","gaus",-4,4);
    fit->SetLineWidth(4); fit->SetLineColor(kAzure);
    hist_signif->SetTitle(hist_title.c_str());
    hist_signif->Draw("e1");
    hist_signif->Fit("fit","");
    double chisqndf = fit->GetChisquare() / fit->GetNDF();
    double mean = fit->GetParameter("Mean");
    double sigma = fit->GetParameter("Sigma");
    TLatex l;
    l.SetTextSize(0.06);
    l.DrawLatex(1.4,hist_signif->GetMaximum()*1.2,Form("#bf{#color[4]{#chi^{2}_{#nu} = %.2f}}",chisqndf));
    l.DrawLatex(1.4,hist_signif->GetMaximum()*1.1,Form("#bf{#color[4]{#mu = %.3f}}",mean));
    l.DrawLatex(1.4,hist_signif->GetMaximum(),Form("#bf{#color[4]{#sigma = %.3f}}",sigma));
    
    c->SaveAs(save_name.c_str());
}
    
void make_plot(string delim, vector<string> rootFile, vector<string> setStr, vector<string> binStr, TH1D* hist_signif)
{
    gStyle->SetTitleAlign(33);
    gStyle->SetTitleX(.95);    
    string dataNomPath = "@DATA_DIR@"; 
    string saveDir = "@SAVE_DIR@";
    string title = "#bf{E_{#gamma} (GeV): ("+binStr[0]+", "+binStr[1]+")}";
  
    //Make Canvas 
    TCanvas *c = new TCanvas("c", "c");

    auto *p3 = new TPad("p3","p4",0.,0.,1.,0.3); p3->Draw();
    p3->SetTopMargin(0.001);
    p3->SetBottomMargin(0.3);
    //p3->SetLogx ();
    p3->SetGrid(0,1);

    auto *p2 = new TPad("p2","p2",0.,0.30,1.,0.5); p2->Draw();
    p2->SetTopMargin(0.001);
    p2->SetBottomMargin(0.001);
    //p2->SetLogx ();
    p2->SetGrid(0,1);

    auto *p1 = new TPad("p1","p1",0.,0.5,1.,1.);  p1->Draw();
    p1->SetBottomMargin(0.001);
    p1->SetLogy();
    p1->cd();
    p1->SetGrid(0,1);
   
    TGraphErrors *g1 = new TGraphErrors((dataNomPath+"diffxsec"+rootFile[0]+delim+".txt").c_str(), "%lg %lg %lg %lg");//, option=" \t,;");
    g1->SetTitle(title.c_str());
    g1->SetMarkerStyle(21);
    g1->SetMarkerSize(1.2);
    g1->SetDrawOption("AP");
    if(setStr[0]=="Spring-2017")
        {  g1->SetMarkerColor(kSpring);g1->SetLineColor(kSpring);}
    else 
        {  g1->SetMarkerColor(kPink);g1->SetLineColor(kPink);}
    g1->SetLineWidth(3);
    g1->SetFillStyle(0);  
    //g1->Draw("ap");
  
    TGraphErrors *g2 = new TGraphErrors(  (dataNomPath+"diffxsec"+rootFile[1]+delim+".txt").c_str(), "%lg %lg %lg %lg");//, option=" \t,;");
    g2->SetTitle(title.c_str());
    g2->SetMarkerStyle(20);
    g2->SetMarkerSize(1.2);
    g2->SetDrawOption("AP");
    if(setStr[1]=="Spring-2018")
        {g2->SetMarkerColor(kPink); g2->SetLineColor(kPink);}
    else
        {g2->SetMarkerColor(kAzure); g2->SetLineColor(kAzure);}
    g2->SetLineWidth(3);
    g2->SetFillStyle(0);
    //g2->Draw("ap same");
  
    TMultiGraph *mg = new TMultiGraph();
    mg->SetTitle((title+"; -t (GeV^{2} );#frac{d#sigma(#gammap #rightarrow K^{+}K^{+}#Xi^{-})}{dt} (nb/GeV^{2} )").c_str() );//=(p^{#mu}_{#gamma}-p^{#mu}_{K^{+}_{fast}})^{2}
    mg->Add(g1);
    mg->Add(g2);

    mg->Draw("ap");
    mg->GetXaxis()->SetLimits(0,2.5);
    //mg->GetYaxis()->SetRangeUser(0.3,11);
    //
    auto legend = new TLegend(0.26,0.03,0.59,0.40);
    legend->SetBorderSize(0);
    legend->SetTextSize(0.08);
    legend->SetTextFont(132);
    legend->SetFillColor(0);
    legend->SetFillStyle(0);
    //legend->SetHeader(legTitle[0].c_str(),"C"); // option "C" allows to center the header
    //legend->AddEntry((TObject*)0,legTitle[1].c_str(),"");
    legend->AddEntry(g1,setStr[0].c_str(),"lep");
    legend->AddEntry(g2,setStr[1].c_str(),"lep");
    //
    legend->Draw();
    //ratio plot
    p2->cd();
    TGraphErrors *sig = (TGraphErrors*)calc_signif(g1,g2);
    double ymin = TMath::MaxElement(sig->GetN(),sig->GetY());
    double ymax = TMath::MinElement(sig->GetN(),sig->GetY());
    double ysym = abs(ymax) > abs(ymin) ? abs(ymax) : abs(ymin);
    if(ysym<5) ysym=5;
  
    sig->SetTitle(" ; -t (GeV^{2} ); #sigma_{sig}");
    sig->GetYaxis()->CenterTitle(true);
    sig->GetXaxis()->SetRangeUser(0,2.5);
    sig->GetYaxis()->SetRangeUser(-ysym-1,ysym+1);
    sig->GetYaxis()->SetNdivisions(505);
    sig->GetYaxis()->SetLabelSize(0.16);
    sig->GetYaxis()->SetTitleSize(0.19);
    sig->GetXaxis()->SetTitleOffset(0.9);
    sig->GetYaxis()->SetTitleOffset(0.3);
    sig->Draw("ap");
    // Draw box in balow 4sigma range
    TBox *box = new TBox(0,-2,2.5,2);
    box->SetFillColorAlpha(kAzure,0.1);
    box->Draw();

    p3->cd();
    TGraphErrors *pct = (TGraphErrors*)calc_pct(g1,g2, hist_signif);
    double ypctmin = TMath::MaxElement(pct->GetN(),pct->GetY());
    double ypctmax = TMath::MinElement(pct->GetN(),pct->GetY());
    double ypctsym = abs(ypctmax) > abs(ypctmin) ? abs(ypctmax) : abs(ypctmin);
      
    pct->SetTitle(" ; -t (GeV^{2} ); ratio");
    pct->GetYaxis()->CenterTitle(true);
    pct->GetXaxis()->SetRangeUser(0,2.5);
    pct->GetYaxis()->SetRangeUser(0.5,1.5);
    pct->GetYaxis()->SetNdivisions(505);
    pct->GetXaxis()->SetLabelSize(0.1);
    pct->GetYaxis()->SetLabelSize(0.1);
    pct->GetXaxis()->SetTitleSize(0.13);
    pct->GetYaxis()->SetTitleSize(0.12);
    pct->GetXaxis()->SetTitleOffset(0.9);
    pct->GetYaxis()->SetTitleOffset(0.5);
    pct->Draw("ap");
  
    c->SaveAs((saveDir+"signif_ratio"+delim+"_"+setStr[0]+"_"+setStr[1]+".pdf").c_str());
}

TGraphErrors* calc_pct(TGraphErrors *nominal, TGraphErrors *variation,  TH1D* hist_pct)
{
    TGraphErrors *graph = new TGraphErrors();
    double Delta, sigma, ratio;
    for(int i = 0; i < nominal->GetN(); i++) {
        // Delta = nominal->GetPointY(i) - variation->GetPointY(i);
        // sigma = nominal->GetPointY(i) + variation->GetPointY(i);
        ratio = nominal->GetPointY(i) / variation->GetPointY(i);
        //cout << Delta << "    " << sigma << "     " << Delta / sigma << endl;
        if(ratio != 0.0)    graph->SetPointY( i, ratio );
        else                graph->SetPointY( i, 0.0 );
        graph->SetPointX( i, nominal->GetPointX(i) );
        graph->SetPointError( i, variation->GetErrorX(i), 0 );
        // cout << i << "   " << Delta / sigma << endl;
        // cout << "    "<< sigma << endl;
        hist_pct->Fill(ratio);
    }
 
    return graph;
}


TGraphErrors* calc_signif(TGraphErrors *nominal, TGraphErrors *variation, TH1D* hist_signif)
{
    TGraphErrors *graph = new TGraphErrors();
    double Delta, sigma, signif;
    for(int i = 0; i < nominal->GetN(); i++) {
        Delta = nominal->GetPointY(i) - variation->GetPointY(i);
        //sigma = nominal->GetPointY(i) + variation->GetPointY(i);
        //sigma = nominal->GetPointY(i);
        sigma = TMath::Sqrt( nominal->GetErrorY(i)*nominal->GetErrorY(i) + variation->GetErrorY(i)*variation->GetErrorY(i) );
        cout << Delta << "    " << sigma << "     " << Delta / sigma << endl;
        if(sigma != 0.0)    {
            graph->SetPointY( i, Delta/sigma );
            signif=Delta/sigma;}
        else {
            graph->SetPointY( i, 0.0 );
            signif = 0.0;}
        
        graph->SetPointX( i, nominal->GetPointX(i) );
        graph->SetPointError( i, variation->GetErrorX(i), 0 );
        //hist_signif->Fill(signif);
      
        // cout << i << "   " << Delta / sigma << endl;
        // cout << "    "<< sigma << endl;
    }
    return graph;
}

void SetStyle()
{
    gStyle->SetCanvasColor(0);
    gStyle->SetCanvasBorderSize(5);
    gStyle->SetCanvasBorderMode(0);
    gStyle->SetCanvasDefH(800);
    gStyle->SetCanvasDefW(700);

    gStyle->SetPadColor       (0);
    gStyle->SetPadBorderSize  (10);
    gStyle->SetPadBorderMode  (0);
    gStyle->SetPadBottomMargin(0.16);
    gStyle->SetPadTopMargin   (0.08);
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
    gStyle->SetFrameLineWidth ( 2);
    gStyle->SetFrameBorderSize(10);
    gStyle->SetFrameBorderMode( 0);

    gStyle->SetNdivisions(505);

    gStyle->SetLineWidth(1);
    gStyle->SetHistLineWidth(1);
    //gStyle->SetLegendBorder(0);
    gStyle->SetFrameLineWidth(2);
    gStyle->SetLegendFillColor(0);
    gStyle->SetLegendFont(132);
    gStyle->SetLegendTextSize(0.06);
    gStyle->SetMarkerSize(1.0);
    gStyle->SetMarkerStyle(20);

    gStyle->SetLabelSize(0.07,"X");
    gStyle->SetLabelSize(0.07,"Y");

    gStyle->SetLabelOffset(0.008,"X");
    gStyle->SetLabelOffset(0.008,"Y");

    gStyle->SetLabelFont(132,"X");
    gStyle->SetLabelFont(132,"Y");
    gStyle->SetTitleBorderSize(0);
    gStyle->SetTitleFont(132);
    gStyle->SetTitleFont(132,"X");
    gStyle->SetTitleFont(132,"Y");

    gStyle->SetTitleSize(0.07,"T");
    gStyle->SetTitleSize(0.07,"X");
    gStyle->SetTitleSize(0.07,"Y");

    gStyle->SetTitleOffset(0,"T");
    gStyle->SetTitleOffset(1.,"X");
    gStyle->SetTitleOffset(0.9,"Y");

    gStyle->SetTextSize(0.06);
    gStyle->SetTextFont(132);

    gStyle->SetOptStat(0);

    gROOT->ForceStyle();

    TLatex* latex = new TLatex();
    latex->SetNDC();
    //latex->SetTextFont(42);
    latex->SetTextSize(0.04);
    latex->SetTextAlign(32);
}
