// Track-efficiency kinematics and figures: the legacy macros
// archive/systematics_legacy/track_efficiency/get_hists.C and get_track_efficiency.C,
// copied verbatim; the hardcoded trees, branches, histograms, weights, periods, theta cut
// and output paths come from TrackSpec.
#include "gxana/systematics/TrackHists.h"

#include "gxana/common/AcceptanceCorrect.h"
#include "gxana/common/Style.h"

#include <ROOT/RDataFrame.hxx>
#include <RConfigure.h>
#include <TCanvas.h>
#include <TDirectory.h>
#include <THStack.h>
#include <TLegend.h>
#include <TLine.h>
#include <TMath.h>
#include <TPad.h>
#include <TROOT.h>

#include <cstdio>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace gxana {
namespace systematics {

// The legacy bodies are interpreted-macro code (implicit std).
using std::cout;
using std::endl;
using std::string;
using std::vector;

namespace {

void style_format();
void MakeStackedAngleHist(const TrackSpec& spec, vector<TH1D*> arr_hist, double eff, string leg_pos = "tl",
                          string leg_title = "GlueX#lower[-0.15]{-}#kern[0.1]{I}");

TH2D* GetHist(TFile* f, const string& path)
{
    auto* hist = dynamic_cast<TH2D*>(f->Get(path.c_str()));
    if (!hist)
        throw std::runtime_error(string(f->GetName()) + ": no TH2D " + path);
    return hist;
}

} // namespace

// get_hists.C:13-96 (get_hists): the three hardcoded period directories and files are the
// spec's periods (directory = period name), the particles are the spec's particles.
void MakeKinematics(const TrackSpec& spec)
{
    //set up root file with directories
    TFile *f =  TFile::Open( (spec.outDir + "/particle_kinematics.root").c_str(), "RECREATE");
    if (!f || f->IsZombie())
        throw std::runtime_error("cannot create " + spec.outDir + "/particle_kinematics.root");
    for (const auto& period : spec.periods) {
        f->cd();
        gDirectory->mkdir(period.name.c_str());
    }

    //initiate variables
    const size_t nPeriods = spec.periods.size();
    vector<TH2D*> hist_all_kin(spec.particles.size());
    vector<string> vec_delim = {"_qval","_mc","_thrown"};
    vector<string> particles;
    for (const auto& p : spec.particles)
        particles.push_back(p.name + "_kin");

    //perform actions
    for (const auto& period : spec.periods) {
        f->cd();
        f->cd(period.name.c_str());
        FillPeriod(spec, period.data, "_qval");
        FillPeriod(spec, period.mc, "_mc");
        FillPeriod(spec, period.thrown, "_thrown");
    }

    //get histos from root tree
    for(size_t j = 0; j < particles.size();j++)
        {
            vector<vector<TH2D*>> vect_histo(nPeriods);
            f->ReOpen("READ");
            for(size_t i = 0; i < vec_delim.size(); i++)
                {
                    // the first period's clones carry the histogram name, as Spring_2017's did
                    vect_histo[0].push_back( (TH2D*)GetHist(f, spec.periods[0].name+"/"+particles[j]+vec_delim[i])->Clone( (particles[j]+vec_delim[i]).c_str()));
                    for (size_t k = 1; k < nPeriods; k++)
                        vect_histo[k].push_back( (TH2D*)GetHist(f, spec.periods[k].name+"/"+particles[j]+vec_delim[i])->Clone() );
                }
            f->ReOpen("UPDATE");
            //Perform acceptance correction
            f->cd(spec.periods[0].name.c_str());
            hist_all_kin[j] = (TH2D*)GetAcceptanceCorrHist2D(vect_histo[0], f)->Clone( (particles[j]+"_phase1_acccorr").c_str());
            //
            for (size_t k = 1; k < nPeriods; k++) {
                f->cd(spec.periods[k].name.c_str());
                TH2D* hist_tmp = (TH2D*)GetAcceptanceCorrHist2D(vect_histo[k], f)->Clone();
                hist_all_kin[j]->Add( hist_tmp );
            }
            //
            f->cd();
            hist_all_kin[j]->Write(hist_all_kin[j]->GetName(),TObject::kOverwrite);

            //merge all the kinematics plots without correction
            TH2D* merged_kin = (TH2D*)vect_histo[0][0]->Clone( (particles[j]+"_phase1").c_str());
            for (size_t k = 1; k < nPeriods; k++)
                merged_kin->Add( (TH2D*)vect_histo[k][0]->Clone() );
            merged_kin->Write(merged_kin->GetName(),TObject::kOverwrite);
            TH2D* merged_kin_mc = (TH2D*)vect_histo[0][1]->Clone((particles[j]+"_phase1_mc").c_str());
            for (size_t k = 1; k < nPeriods; k++)
                merged_kin_mc->Add( (TH2D*)vect_histo[k][1]->Clone() );
            merged_kin_mc->Write(merged_kin_mc->GetName(),TObject::kOverwrite);
        }

    f->Close();
}

// get_hists.C:98-106
TH2D* GetAcceptanceHist2D(TH2D* hist_genr, TH2D* hist_recon)
{
    TH2D* hist_accept = (TH2D*)gxana::Acceptance(*hist_genr, *hist_recon, "acceptance", gxana::AccErrors::Plain);

    printf("acceptance bins: %d\n",hist_accept->GetNbinsX());
    return hist_accept;
}

// get_hists.C:108-122
//`vect_hist` [0](data),[1](recon),[2](generated)
TH2D* GetAcceptanceCorrHist2D(vector<TH2D*> vec_hist, TFile *save_file)
{
    (void)save_file;
    //get the acceptance
    string name = vec_hist[0]->GetName();
    TH2D* hist_accept = (TH2D*)GetAcceptanceHist2D(vec_hist[2],vec_hist[1])->Clone();
    hist_accept->Write( (name+"_acceptance").c_str(),TObject::kOverwrite);

    TH2D* hist_data_acccorr = (TH2D*)gxana::AcceptanceCorrect(*vec_hist[0], *hist_accept, vec_hist[0]->GetName(), gxana::AccErrors::Plain);
    hist_data_acccorr->Write( (name+"_acceptcorr").c_str(),TObject::kOverwrite);

    return hist_data_acccorr;
}

// get_hists.C:127-218 (save_from_flattrees): the three per-kind blocks are one loop over the
// spec's particles; data (_qval) is weighted by spec.dataWeight, MC (_mc) by spec.mcWeight,
// thrown unweighted.
void FillPeriod(const TrackSpec& spec, const std::string& root_file_path, const std::string& hist_name, int n_threads)
{
#ifdef R__USE_IMT
  if(n_threads > 0.0)	ROOT::EnableImplicitMT(n_threads);
#else
  (void)n_threads;
#endif
  // make data frame and braches for histograms from 4 vectors
  // format : tree name, file name, branches to open
  const bool thrown = hist_name == "_thrown";
  const string weight = thrown ? "" : (hist_name == "_mc" ? spec.mcWeight : spec.dataWeight);
  ROOT::RDF::RNode df = ROOT::RDataFrame(thrown ? spec.thrownTree : spec.tree, root_file_path);
  for (const auto& p : spec.particles) {
      const string p4 = thrown ? p.thrownP4 : p.p4;
      df = df.Define(p.name+"_p", p4+".P()")
             .Define(p.name+"_theta", p4+".Theta()*180/TMath::Pi()");
  }
  if (!weight.empty())
      df = df.Define("syst_weight", weight);

  //make histograms and add to tfile
  vector<ROOT::RDF::RResultPtr<TH2D>> hists;
  for (const auto& p : spec.particles) {
      ROOT::RDF::TH2DModel model("", p.title.c_str(), p.nTheta, p.thetaLo, p.thetaHi, p.nP, p.pLo, p.pHi);
      if (weight.empty())
          hists.push_back(df.Histo2D(model, p.name+"_theta", p.name+"_p"));
      else
          hists.push_back(df.Histo2D(model, p.name+"_theta", p.name+"_p", "syst_weight"));
  }
  //
  for (size_t i = 0; i < spec.particles.size(); i++)
      hists[i]->Write((spec.particles[i].name+"_kin"+hist_name).c_str(),TObject::kOverwrite);
}

// get_track_efficiency.C:18-74 (get_track_efficiency): the particles are the spec's, the
// cut is spec.thetaCut; the counts go to <outDir>/track_counts.txt (the efficiencies are
// computed from them by gxana_systematics.track; eff/effMC here are printed and annotated).
void CountAndDraw(const TrackSpec& spec)
{
  // call plot styling function
  style_format();

  //initiate variables
  vector<TH1D*> hist;
  vector<TH1D*> hist_particle_angle;
  vector<TH1D*> hist_particle_angle_mc;

  vector<TH1D*> hist_particle_angle_corr;
  vector<string> particles;
  for (const auto& p : spec.particles)
      particles.push_back(p.name + "_kin");

  // Load root tree of choice
  TFile *f = TFile::Open( (spec.outDir + "/particle_kinematics.root").c_str());
  if (!f || f->IsZombie())
      throw std::runtime_error("cannot open " + spec.outDir + "/particle_kinematics.root");
  const string countsPath = spec.outDir + "/track_counts.txt";
  std::ofstream counts(countsPath);
  if (!counts)
      throw std::runtime_error("cannot write " + countsPath);
  counts << "particle nlow_data nhigh_data nlow_mc nhigh_mc\n";
  //Get Angular Histograms
  double tot_eff = 0; double tot_eff_mc = 0;
  for(size_t ip = 0; ip < particles.size(); ip++)
      {
          const auto& part = particles[ip];
          TH2D* hist_tmp = (TH2D*)GetHist(f, part+"_phase1")->Clone();
          TH2D* hist_tmp_mc = (TH2D*)GetHist(f, part+"_phase1_mc")->Clone();
          TH1D* hist_angle = (TH1D*)hist_tmp->ProjectionX()->Clone( (part+"_angle_phase1_mc").c_str());
          TH1D* hist_angle_mc = (TH1D*)hist_tmp_mc->ProjectionX();
          TH1D* hist_mom = (TH1D*)hist_tmp->ProjectionY()->Clone( (part+"_mom_phase1_mc").c_str());
          TH1D* hist_mom_mc = (TH1D*)hist_tmp_mc->ProjectionY();
          (void)hist_mom;
          (void)hist_mom_mc;

          hist_particle_angle.push_back( hist_angle);
          hist_particle_angle_mc.push_back( hist_angle_mc);

          double lowBin = hist_angle->FindBin(spec.thetaCut);
          double lowBinMC = hist_angle_mc->FindBin(spec.thetaCut);
          double Nlow = hist_angle->Integral(0,lowBin);
          double NlowMC = hist_angle_mc->Integral(0,lowBinMC);
          double Nhigh = hist_angle->Integral(lowBin+1, hist_angle->GetNbinsX());
          double NhighMC = hist_angle_mc->Integral(lowBinMC+1, hist_angle_mc->GetNbinsX());
          double eff = (spec.low*Nlow + spec.high*Nhigh) / (Nlow + Nhigh);
          double effMC = (spec.low*NlowMC + spec.high*NhighMC) / (NlowMC + NhighMC);
          tot_eff += eff;
          tot_eff_mc += effMC;
          cout << part << ": (Data, MC)\n"
               << "\tNlow (" << Nlow << ", " << NlowMC << ")\n"
               << "\tNhigh (" << Nhigh << ", " << NhighMC << ")\n"
               << "\tTrack Eff (" << eff << ", " << effMC << ")"
               << endl;
          counts << spec.particles[ip].name << Form(" %.6g %.6g %.6g %.6g\n", Nlow, Nhigh, NlowMC, NhighMC);

          MakeStackedAngleHist(spec, {hist_angle,hist_angle_mc},effMC, "tr", spec.legendHeader);
          //MakeStackedHist({hist_angle,hist_angle_mc}, "tr");
          // MakeStackedHist({hist_mom,hist_mom_mc}, "tr");
      }

  cout << "Total Track Efficiency: (" << tot_eff << ", "
       << tot_eff_mc << ")"<< endl;
  counts.close();
  if (!counts)
      throw std::runtime_error("cannot write " + countsPath);
  cout << "wrote " << countsPath << endl;
}

namespace {

// get_track_efficiency.C:76-155: saved to <outDir>/<histName>_data_mc.pdf; the dashed line
// marks spec.thetaCut.
void MakeStackedAngleHist(const TrackSpec& spec, vector<TH1D*> arr_hist, double eff, string leg_pos, string leg_title)
{
  string histName = arr_hist[0]->GetName();
  string saveName = spec.outDir + "/" + histName + "_data_mc.pdf";
  char entries[4][100];
  vector<Color_t> arr_colors = {kBlue, kMagenta, kRed, kCyan, kYellow, kOrange};
  vector<Style_t> arr_marker_style = {kFullCircle, kFullSquare, kFullTriangleUp, kFullStar, kFullTriangleDown, kFullDiamond};
  TCanvas *c = new TCanvas((histName).c_str(), (histName+"_").c_str());
  THStack *hs = new THStack("hs","");

  double scale_factor;
  //arr_hist[0]->SetFillColorAlpha(kGray,0.9);
  arr_hist[0]->SetMarkerStyle(20);
  arr_hist[0]->SetMarkerSize(0.9);
  arr_hist[0]->SetMarkerColor(kBlack);
  arr_hist[0]->SetLineColor(kBlack);

  for(size_t i = 1; i < arr_hist.size(); i++){
    //arr_hist[i]->RebinX();
    cout << "Bin Width Data: " << arr_hist[0]->GetBinWidth(1) << endl;
    cout << "Bin Width MC: " << arr_hist[i]->GetBinWidth(1) << endl;

    // scale the histos to area
    if(arr_hist[i]->Integral("width") < arr_hist[0]->Integral("width"))
      scale_factor = arr_hist[0]->Integral("width") / arr_hist[i]->Integral("width");
    else
      scale_factor = arr_hist[i]->Integral("width") / arr_hist[0]->Integral("width") ;

    if(arr_hist[i]->GetMaximum() < arr_hist[0]->GetMaximum())
      arr_hist[i]->Scale(scale_factor );
    else
      arr_hist[i]->Scale(1/scale_factor );

    arr_hist[i]->SetLineColor(kBlack);
    arr_hist[i]->SetFillColorAlpha(kAzure-9,0.6);
    //arr_hist[i]->SetFillStyle(4010);
    snprintf(entries[i], sizeof(entries[i]), "%.0f arb. units", arr_hist[i]->GetEntries());

    hs->Add(arr_hist[i],"hist");
  }
  (void)arr_colors;
  (void)arr_marker_style;

  //arr_hist[0]->RebinX();
  hs->Add(arr_hist[0],"e1");

  string title = Form("#bf{#color[2]{#varepsilon = %.2f%%}}; %s; arb. units", eff*100, arr_hist[0]->GetXaxis()->GetTitle());
  hs->SetTitle(title.c_str());
  hs->Draw("nostack");
  hs->GetYaxis()->SetMaxDigits(2);
  //hs->GetXaxis()->SetLimits(xmin,xmax);

  // Draw legend
  TLegend* legend;
  if(leg_pos=="tl")
    legend = new TLegend(0.16,0.75,0.4,0.94); //top left corner
  else
    legend = new TLegend(0.66,0.71,0.86,0.88); //top right corner

  legend->SetBorderSize(0);
  legend->SetTextSize(0.065);
  legend->SetHeader(leg_title.c_str(),"C"); // option "C" allows to center the header

  legend->AddEntry(arr_hist[0], "Data", "lep");
  legend->AddEntry(arr_hist[1], "Recon MC", "f");
  legend->Draw("same");

  auto l = new TLine(spec.thetaCut, 0, spec.thetaCut, arr_hist[1]->GetMaximum()*1.05);
  l->SetLineColor(kBlue);
  l->SetLineStyle(kDashed);
  l->SetLineWidth(4);
  l->Draw("same");

  gPad->Update();
  c->SaveAs(saveName.c_str());
  //c->Close();
}

// get_track_efficiency.C:232-309
// input specific style formatting of user choice
void style_format()
{
  gxana::ApplyStyle(gxana::TrackStyle());
}

} // namespace

} // namespace systematics
} // namespace gxana
