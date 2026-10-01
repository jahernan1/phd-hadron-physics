#include "gxana/common/Paths.h"
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
    gStyle->SetPadLeftMargin  (0.12);
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
    gStyle->SetTitleOffset(0.65,"Y");

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

void GetSpinAnalysis(std::vector<TH1D*> hists, std::string name)
{
    double binWidth = hists[0]->GetXaxis()->GetBinWidth(1);
    char histTitle[100];
    sprintf(histTitle, " ;cos#vartheta^{#pi^{-}}_{#it{h}} ; arb. units  / %.3f ", binWidth);

    TH1D *acceptance = (TH1D*)hists[1]->Clone();
    TH1D *accepted_hist = (TH1D*)hists[0]->Clone();
    
        
    acceptance->SetTitle(" ; ; acceptance, #varepsilon");
    acceptance->SetMarkerStyle(24);
    acceptance->SetMarkerSize(1.1);
    acceptance->SetMarkerColor(kRed+1);
    acceptance->SetLineColor(kRed+1);
    acceptance->SetLineWidth(2);
    acceptance->SetFillStyle(0);  
    acceptance->GetYaxis()->SetAxisColor(kRed+1);
    acceptance->GetYaxis()->SetTitleColor(kRed+1);
    acceptance->GetYaxis()->SetLabelColor(kRed+1);
    acceptance->GetYaxis()->SetMaxDigits(2);
    acceptance->GetYaxis()->SetNdivisions(505);
    //acceptance->GetYaxis()->CenterTitle(true);
    acceptance->GetYaxis()->SetTitleOffset(0.9);

    accepted_hist->GetYaxis()->SetTitleOffset(0.9);
    accepted_hist->GetYaxis()->SetMaxDigits(3);
    accepted_hist->GetYaxis()->SetNdivisions(505);
    accepted_hist->SetTitle(histTitle);

    TCanvas *c = new TCanvas("c",name.c_str());
    TPad *pad1 = new TPad((name+"_Pad1").c_str(),"",0,0,1,1);
    TPad *pad2 = new TPad((name+"_Pad2").c_str(),"",0,0,1,1);
    pad2->SetFillStyle(4000); //will be transparent
    pad2->SetFrameFillStyle(0);
    pad1->SetRightMargin(0.15);
    pad2->SetRightMargin(0.15);
    pad1->SetBottomMargin(0.17);
    pad2->SetBottomMargin(0.17);
    pad1->SetLeftMargin(0.15);
    pad2->SetLeftMargin(0.15);
    pad1->SetLogy();
    //pad2->SetLogy();
    //pad2->SetFrameLineColor(kRed+1);
    //pad2->GetFrame()->SetLineWidth(10);
    pad1->Draw();
    pad1->cd();
    accepted_hist->Draw("PL");

    //no beta hypothesis
    TF1* fit = new TF1("fit","pol0",-1,1);
    fit->SetLineColor(kAzure);
    fit->SetLineWidth(4);
    fit->SetLineStyle(kDashed);
    accepted_hist->Fit("fit","WLR");
    //beta hypothesis spin-1/2
    TF1* fit_beta = new TF1("fit_beta","[Const]*(1+[Beta]*x)",-1,1);
    fit_beta->SetParameter(1,fit->GetParameter(0));
    fit_beta->SetParLimits(0,0,1);
    fit_beta->SetLineColor(kAzure);
    fit_beta->SetLineWidth(4);    
    accepted_hist->Fit("fit_beta","WLR+");

    
    double beta = fit_beta->GetParameter(0);
    double beta_err = fit_beta->GetParError(0);

    //no beta spin-3/2
    TF1* fit1 = new TF1("fit1","[C]*(1+3*x^2)",-1,1);
    fit1->SetLineColor(kMagenta);
    fit1->SetLineWidth(4);
    fit1->SetLineStyle(kDashed);
    //beta spin-3/2
    TF1* fit1_beta = new TF1("fit1_beta","[C]*(1 + 3*x^2 + [B]*x*(5-9*x^2))",-1,1);
    fit1_beta->SetParameter(1,fit->GetParameter(0));
    fit1_beta->SetParLimits(0,0,1);
    fit1_beta->SetLineColor(kSpring);
    fit1_beta->SetLineWidth(4);    

    accepted_hist->Fit("fit1_beta","WLN0");
    double beta1 = fit1_beta->GetParameter(0);
    double beta1_err = fit1_beta->GetParError(0);
    
    //fix beta for 3/2 from 1/2
    fit1_beta->FixParameter(0, beta);
    fit1_beta->SetParError(0, beta_err);
    fit1_beta->SetLineColor(kMagenta);
    fit1_beta->SetLineWidth(4);
    accepted_hist->Fit("fit1","WLR+");
    accepted_hist->Fit("fit1_beta","BWLR+");
    double chisqndf = fit->GetChisquare() / fit->GetNDF();
    double chisqndf_beta = fit_beta->GetChisquare() / fit_beta->GetNDF();
    double chisqndf1 = fit1_beta->GetChisquare() / fit1_beta->GetNDF();
    printf("FITRESULT spin %s beta=%.8f beta_err=%.8f chi2_nobeta=%.6f chi2_j12=%.6f chi2_j32=%.6f beta1=%.8f beta1_err=%.8f\n", name.c_str(), beta, beta_err, chisqndf, chisqndf_beta, chisqndf1, beta1, beta1_err);

    auto legend = new TLegend(0.2,0.19,0.825,0.355);
    legend->SetNColumns(3);
    legend->SetBorderSize(1);
    legend->SetTextSize(0.05);
    legend->SetTextFont(132);
    //legend->SetFillColor(0);
    //legend->SetFillStyle(0);
    
    //legend->SetHeader("The Legend Title","C"); // option "C" allows to center the header
    legend->AddEntry(accepted_hist,Form("#splitline{GlueX#lower[-0.15]{-}#kern[0.2]{I}}{#scale[0.9]{#beta = %0.3f}}",beta),"lep");
    legend->AddEntry(fit_beta,Form("#splitline{Fit J = 1/2}{#scale[0.8]{(#color[4]{#chi^{2}_{#nu} = %0.2f)}}}",chisqndf_beta),"l");
    legend->AddEntry(fit1_beta,Form("#splitline{Fit J = 3/2}{#scale[0.8]{(#color[6]{#chi^{2}_{#nu} = %0.2f)}}}",chisqndf1),"l");
    legend->AddEntry((TObject*)0,"#scale[0.9]{#beta = 0}","");
    legend->AddEntry(fit,Form("#scale[0.8]{(#color[4]{#chi^{2}_{#nu} = %0.2f)}}",chisqndf),"l");
    
    legend->Draw();

    // TLatex l;
    // l.SetTextSize(0.055);
    // l.DrawLatex(0.35,accepted_hist->GetMinimum()*0.85,Form("#bf{#color[4]{#chi^{2}_{#nu} = %.2f}}",chisqndf_beta));
    // l.DrawLatex(0.35,accepted_hist->GetMinimum()*0.63,Form("#color[4]{#chi^{2}_{#nu} = %.2f}",chisqndf));
    // l.DrawLatex(0.35,accepted_hist->GetMinimum()*0.46,Form("#color[6]{#chi^{2}_{#nu} = %.2f}",chisqndf1));
    
    pad2->Draw();
    pad2->cd();
    acceptance->GetYaxis()->RotateTitle(true);
    acceptance->Draw("histY+");

    cout << "Beta-1/2,3/2 =  (" << beta << ", " << beta1 << ")" << endl;
    string plotDir = gxana::EnvPath("GXANA_OUTPUT", "kpkpxim/prod_plots/");
    c->SaveAs( (plotDir+"accepted_pimcostheta_"+name+".pdf").c_str());    
    //delete c, pad1, pad2, acceptance, accepted_hist;
}

// Per-period spin fit and plot (accepted_pimcostheta_<stem>.pdf): the original per-period copy of
// GetSpinAnalysis from GetXimProperties.C. It differs from GetSpinAnalysis above: no free J=3/2 fit
// (beta1), the J=3/2 fit with beta fixed starts from default parameters without limits, and the
// chi2 values are drawn with TLatex instead of the legend.
void GetSpinAnalysisPeriod(std::vector<TH1D*> hists, std::string name)
{
    double binWidth = hists[0]->GetXaxis()->GetBinWidth(1);
    char histTitle[100];
    sprintf(histTitle, " ;cos#vartheta^{#pi^{-}}_{#it{h}} ; arb. units  / %.3f ", binWidth);
    TH1D *acceptance, *accepted_hist;

    // gxana: the original built the acceptance and the corrected histogram here from {data, MC, thrown};
    // PrepSpinData.C does that now, so this function always receives {accCorr, accept} and only that path is kept.
    acceptance = (TH1D*)hists[1]->Clone();
    accepted_hist = (TH1D*)hists[0]->Clone();
        
    acceptance->SetTitle(" ; ; acceptance, #varepsilon");
    acceptance->SetMarkerStyle(24);
    acceptance->SetMarkerSize(1.1);
    acceptance->SetMarkerColor(kRed+1);
    acceptance->SetLineColor(kRed+1);
    acceptance->SetLineWidth(2);
    acceptance->SetFillStyle(0);  
    acceptance->GetYaxis()->SetAxisColor(kRed+1);
    acceptance->GetYaxis()->SetTitleColor(kRed+1);
    acceptance->GetYaxis()->SetLabelColor(kRed+1);
    acceptance->GetYaxis()->SetMaxDigits(2);
    acceptance->GetYaxis()->SetNdivisions(505);
    //acceptance->GetYaxis()->CenterTitle(true);
    acceptance->GetYaxis()->SetTitleOffset(0.9);

    accepted_hist->GetYaxis()->SetTitleOffset(0.9);
    accepted_hist->GetYaxis()->SetMaxDigits(3);
    accepted_hist->GetYaxis()->SetNdivisions(505);
    accepted_hist->SetTitle(histTitle);

    TCanvas *c = new TCanvas("c",name.c_str());
    TPad *pad1 = new TPad((name+"_Pad1").c_str(),"",0,0,1,1);
    TPad *pad2 = new TPad((name+"_Pad2").c_str(),"",0,0,1,1);
    pad2->SetFillStyle(4000); //will be transparent
    pad2->SetFrameFillStyle(0);
    pad1->SetRightMargin(0.15);
    pad2->SetRightMargin(0.15);
    pad1->SetBottomMargin(0.17);
    pad2->SetBottomMargin(0.17);
    pad1->SetLeftMargin(0.15);
    pad2->SetLeftMargin(0.15);
    pad1->SetLogy();
    //pad2->SetLogy();
    //pad2->SetFrameLineColor(kRed+1);
    //pad2->GetFrame()->SetLineWidth(10);
    pad1->Draw();
    pad1->cd();
    accepted_hist->Draw("PL");

    TF1* fit = new TF1("fit","pol0",-1,1);
    fit->SetLineColor(kAzure);
    fit->SetLineWidth(4);
    fit->SetLineStyle(kDashed);
    accepted_hist->Fit("fit","WLR");
    
    TF1* fit_beta = new TF1("fit_beta","[Const]*(1+[Beta]*x)",-1,1);
    fit_beta->SetParameter(1,fit->GetParameter(0));
    fit_beta->SetParLimits(0,0,1);
    fit_beta->SetLineColor(kAzure);
    fit_beta->SetLineWidth(4);    
    accepted_hist->Fit("fit_beta","WLR+");

    double beta = fit_beta->GetParameter(0);
    double beta_err = fit_beta->GetParError(0);

    TF1* fit1 = new TF1("fit1","[C]*(1+3*x^2)",-1,1);
    fit1->SetLineColor(kMagenta);
    fit1->SetLineWidth(4);
    fit1->SetLineStyle(kDashed);
    
    TF1* fit1_beta = new TF1("fit1_beta","[C]*(1 + 3*x^2 + [B]*x*(5-9*x^2))",-1,1);
    fit1_beta->FixParameter(0, beta);
    fit1_beta->SetParError(0, beta_err);
    fit1_beta->SetLineColor(kMagenta);
    fit1_beta->SetLineWidth(4);
    accepted_hist->Fit("fit1","WLR+");
    accepted_hist->Fit("fit1_beta","BWLR+");
    double chisqndf = fit->GetChisquare() / fit->GetNDF();
    double chisqndf_beta = fit_beta->GetChisquare() / fit_beta->GetNDF();
    double chisqndf1 = fit1_beta->GetChisquare() / fit1_beta->GetNDF();
    printf("FITRESULT spin %s beta=%.8f beta_err=%.8f chi2_nobeta=%.6f chi2_j12=%.6f chi2_j32=%.6f\n", name.c_str(), beta, beta_err, chisqndf, chisqndf_beta, chisqndf1);
    TLatex l;
    l.SetTextSize(0.055);
    l.DrawLatex(0.35,accepted_hist->GetMinimum()*0.85,Form("#bf{#color[4]{#chi^{2}_{#nu} = %.2f}}",chisqndf_beta));
    l.DrawLatex(0.35,accepted_hist->GetMinimum()*0.63,Form("#color[4]{#chi^{2}_{#nu} = %.2f}",chisqndf));
    l.DrawLatex(0.35,accepted_hist->GetMinimum()*0.46,Form("#color[6]{#chi^{2}_{#nu} = %.2f}",chisqndf1));
    
    pad2->Draw();
    pad2->cd();
    acceptance->GetYaxis()->RotateTitle(true);
    acceptance->Draw("histY+");

    cout << "Beta= " << beta << endl;
    string plotDir = gxana::EnvPath("GXANA_OUTPUT", "kpkpxim/prod_plots/");
    c->SaveAs( (plotDir+"accepted_pimcostheta_"+name+".pdf").c_str());    
    //delete c, pad1, pad2, acceptance, accepted_hist;
}

int PlotGlueXSpin(int n_threads = 4){
    TFile *f =  TFile::Open("xim_spin.root", "READ");
    if (!f || f->IsZombie()) { printf("cannot open xim_spin.root (run PrepSpinData.C first)\n"); return 1; }
    TH1D* merged_spin = (TH1D*)f->Get( "pim_costheta_hf_phase1")->Clone();
    TH1D* merged_spin_accept = (TH1D*)f->Get( "pim_costheta_hf_avg_accept_phase1")->Clone();

    setStyle();
    GetSpinAnalysis({merged_spin,merged_spin_accept}, "gluex_phase1");

    // Per-period fits. The original ran them inside GetXimProperties.C, after
    // ROOT::EnableImplicitMT(4) (the merged fit above ran in a process without it); the last
    // digits of the fit errors depend on that call. n_threads = 0 leaves implicit MT off.
    if (n_threads > 0) ROOT::EnableImplicitMT(n_threads);
    std::vector<std::pair<std::string, std::string>> periods = {
        {"Spring_2017", "kpkpxim__M23_2017-01_ana56"},
        {"Spring_2018", "kpkpxim__B4_M23_2018-01_ana03"},
        {"Fall_2018", "kpkpxim__B4_M23_2018-08_ana02"}};
    for (const auto& p : periods) {
        TH1D* data_accCorr = (TH1D*)f->Get((p.first + "/piminus_costheta_hf_accCorr").c_str())->Clone();
        TH1D* accept = (TH1D*)f->Get((p.first + "/piminus_costheta_hf_accept").c_str())->Clone();
        GetSpinAnalysisPeriod({data_accCorr, accept}, p.second);
    }
    return 0;
}
