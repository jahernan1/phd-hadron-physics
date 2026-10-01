#ifndef GXANA_SYSTEMATICS_TRACKHISTS_H
#define GXANA_SYSTEMATICS_TRACKHISTS_H

#include <TFile.h>
#include <TH1D.h>
#include <TH2D.h>

#include <string>
#include <vector>

namespace gxana {
namespace systematics {

// One reconstructed track: the theta/p histograms are <name>_kin_{qval,mc,thrown}.
// "NAME:P4:THROWNP4:NT,TLO,THI:NP,PLO,PHI:TITLE" on the command line.
struct TrackParticle {
    std::string name;
    std::string p4;        // reconstructed TLorentzVector branch (data and MC trees)
    std::string thrownP4;  // TLorentzVector branch of the thrown tree
    int nTheta = 0;
    double thetaLo = 0, thetaHi = 0;
    int nP = 0;
    double pLo = 0, pHi = 0;
    std::string title;     // Histo2D title ("title;x;y")
};

// One run period: the directory NAME in particle_kinematics.root and its three trees.
struct TrackPeriod {
    std::string name, data, mc, thrown;
};

struct TrackSpec {
    std::string outDir;
    std::string tree, thrownTree;
    std::string dataWeight, mcWeight; // expressions; empty = unweighted
    double thetaCut = 20;             // degrees
    double low = 0.03, high = 0.05;   // per-track efficiency below / above thetaCut (annotation)
    std::string legendHeader;
    std::vector<TrackPeriod> periods;
    std::vector<TrackParticle> particles;
};

// get_hists.C save_from_flattrees: <name>_kin<kind> (kind _qval, _mc or _thrown) for every
// particle of the tree at path, written to the current directory.
void FillPeriod(const TrackSpec& spec, const std::string& path, const std::string& kind, int n_threads = 16);

// get_hists.C GetAcceptanceHist2D / GetAcceptanceCorrHist2D.
TH2D* GetAcceptanceHist2D(TH2D* hist_genr, TH2D* hist_recon);
TH2D* GetAcceptanceCorrHist2D(std::vector<TH2D*> vec_hist, TFile* save_file, Bool_t weighted = false);

// get_hists.C get_hists(): <outDir>/particle_kinematics.root, one directory per period plus the
// merged <name>_kin_phase1{,_mc,_acccorr}.
void MakeKinematics(const TrackSpec& spec);

// get_track_efficiency.C get_track_efficiency(): the data/MC theta counts below and above
// thetaCut (<outDir>/track_counts.txt) and <outDir>/<name>_kin_angle_phase1_mc_data_mc.pdf.
void CountAndDraw(const TrackSpec& spec);

} // namespace systematics
} // namespace gxana

#endif
