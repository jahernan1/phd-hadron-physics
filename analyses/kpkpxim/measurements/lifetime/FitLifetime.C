#include "../common/XimInputs.h"

// Lifetime fit: expo fit (0.02-0.7 ns, "WLR") of the acceptance-corrected Xi- rest-frame
// lifetime, tau = -1/slope. Reads xim_lifetime.root written by PrepLifetime.C; writes
// $GXANA_OUTPUT/kpkpxim/prod_plots/accepted_ximlifetime_<stem>.pdf.
// The PDG-minus-MC-mean shift below is computed as in the original but never applied.
void GetLifetimeAnalysis(std::vector<TH1D*> hists, std::string name)
{
    double pdg_lifetime = 0.1639;//(ns) lifetime_err=0.0015
    double thrown_lifetime = hists[2]->GetMean();
    double mc_lifetime = hists[1]->GetMean();
    double correction = pdg_lifetime - mc_lifetime;
    double binWidth = hists[0]->GetXaxis()->GetBinWidth(1);
    char histTitle[100];
    sprintf(histTitle, " ;#Xi^{-} Lifetime; arb. units  / %.3f ns", binWidth);
    cout << "Thrown Value: " << thrown_lifetime << " ns" << endl;

    //
    cout << "Thrown Mean: " << hists[2]->GetMean() << endl;
    cout << "MC Mean: " << hists[1]->GetMean() << endl;

    TH1D* acceptance = (TH1D*)hists[3]->Clone();   // cascade_lifetime_accept, from PrepLifetime.C
    TH1D* accepted_hist = (TH1D*)hists[0]->Clone(); // cascade_lifetime_accCorr
    acceptance->SetTitle(" ; ; acceptance, #varepsilon");
    acceptance->GetYaxis()->RotateTitle(true);
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
       
    accepted_hist->GetYaxis()->SetMaxDigits(3);
    accepted_hist->GetYaxis()->SetNdivisions(505);
    accepted_hist->SetTitle(histTitle);
    
    TF1* fit = new TF1("fit","expo",0.02,0.7);
    fit->SetLineColor(kAzure);
    fit->SetLineWidth(4);
    TCanvas *c = new TCanvas("c",name.c_str());
    TPad *pad1 = new TPad((name+"_Pad1").c_str(),"",0,0,1,1);
    TPad *pad2 = new TPad((name+"_Pad2").c_str(),"",0,0,1,1);
    pad2->SetFillStyle(4000); //will be transparent
    pad2->SetFrameFillStyle(0);
    pad1->SetRightMargin(0.15);
    pad2->SetRightMargin(0.15);
    //pad1->SetLogx();
    //pad2->SetLogx();
    //pad2->SetFrameLineColor(kRed+1);
    //pad2->GetFrame()->SetLineWidth(10);
    pad1->Draw();
    pad1->cd();
    accepted_hist->Draw("PL");
    accepted_hist->Fit("fit","WLR");
    double slope = -1/fit->GetParameter(1);
    double chisqndf = fit->GetChisquare() / fit->GetNDF();
    TLatex l;
    l.SetTextSize(0.06);
    printf("FITRESULT lifetime %s tau=%.8f slope_err=%.8f chi2ndf=%.6f ndf=%d\n", name.c_str(), slope, fit->GetParError(1), chisqndf, fit->GetNDF());
    l.DrawLatex(0.45,accepted_hist->GetMaximum(),Form("#tau_{#Xi} = %.4f ns",slope));
    //l.DrawLatex(0.3,accepted_hist->GetMaximum()*0.85,Form("Correction = %.4f ns",correction));
    l.DrawLatex(0.45,accepted_hist->GetMaximum()*0.85,Form("#chi^{2}_{#nu} = %.2f",chisqndf));
    
    pad2->Draw();
    pad2->cd();
    acceptance->GetYaxis()->RotateTitle(true);
    acceptance->Draw("histY+");
    
    string plotDir = gxana::EnvPath("GXANA_OUTPUT", "kpkpxim/prod_plots/");
    c->SaveAs( (plotDir+"accepted_ximlifetime_"+name+".pdf").c_str());    
    delete c, pad1, pad2;
}

int FitLifetime(int n_threads = 4)
{
    // The original fitted in the same process as the histogram filling, with implicit
    // multithreading on; the last digits of the fit errors depend on it.
    if (n_threads > 0) ROOT::EnableImplicitMT(n_threads);
    setStyle();
    TFile* f = XimOpen("xim_lifetime.root", "READ");
    for (const auto& p : XimPeriods()) {
        auto get = [&](const char* n) {
            auto h = (TH1D*)f->Get((p.dir + "/" + n).c_str());
            if (!h) throw std::runtime_error("xim_lifetime.root: missing " + p.dir + "/" + n);
            return h;
        };
        GetLifetimeAnalysis({get("cascade_lifetime_accCorr"), get("cascade_lifetime_mc"),
                             get("cascade_lifetime_thrown"), get("cascade_lifetime_accept")}, p.stem);
    }
    return 0;
}
