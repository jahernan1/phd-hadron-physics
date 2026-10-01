#include "gxana/common/Overlay.h"
#include "gxana/common/Paths.h"

//functions
int make_plots(string input_file, string iter_file);
void compare_plot(vector<TH1D*> vec_hist, string save_name, string leg_title, string axis_title, Bool_t make_leg = true);

//main
int compare_iters_2D()
{
  make_plots("test_noac_ximVertexCut_hist2d.root", "data_ac_ximVertexCut_hist2d_YstarRest.root");
  
  return 0;
}

//get histos to plot
int make_plots(string input_file, string iter_file)
{
    string dir = gxana::EnvPath("GXANA_OUTPUT", "kpkpxim/");
    //
    TFile *inf = TFile::Open( ( dir + "MC/" + input_file ).c_str() );
    TFile *inf1 = TFile::Open( ( dir + "MC/" + iter_file ).c_str() );
    TH2D* hist_2d_ac = (TH2D*)inf->Get("ResMassVsCosTheta_Phase1_ac")->Clone();
    TH2D* hist_2d = (TH2D*)inf->Get("ResMassVsCosTheta_qval_Phase1")->Clone();
    TH2D* hist_2d_acceptance = (TH2D*)inf->Get("Fall_2018/costhetahf_ystar_acceptance")->Clone();
    //
    TH2D* hist_2d_ac_1 = (TH2D*)inf1->Get("ResMassVsCosTheta_Phase1_ac")->Clone();
    TH2D* hist_2d_1 = (TH2D*)inf1->Get("ResMassVsCosTheta_qval_Phase1")->Clone();
    TH2D* hist_2d_acceptance_1 = (TH2D*)inf1->Get("Fall_2018/costhetahf_ystar_acceptance")->Clone();
    //
    TH1D* costheta = (TH1D*)hist_2d->ProjectionY()->Clone("costheta");
    TH1D* costheta_ac = (TH1D*)hist_2d_ac->ProjectionY()->Clone("costheta_ac");
    TH1D* ystar_ac = (TH1D*)hist_2d_ac->ProjectionX()->Clone("ystar_ac");
    TH1D* cos_theta_acceptance = (TH1D*)hist_2d_acceptance->ProjectionY()->Clone("costheta_acceptance");
    TH1D* ystar_acceptance = (TH1D*)hist_2d_acceptance->ProjectionX()->Clone("ystar_acceptance");
    //
    TH1D* costheta_1 = (TH1D*)hist_2d_1->ProjectionY()->Clone("costheta_1");
    TH1D* costheta_ac_1 = (TH1D*)hist_2d_ac_1->ProjectionY()->Clone("costheta_ac_1");
    TH1D* ystar_ac_1 = (TH1D*)hist_2d_ac_1->ProjectionX()->Clone("ystar_ac_1");
    TH1D* cos_theta_acceptance_1 = (TH1D*)hist_2d_acceptance_1->ProjectionY()->Clone("costheta_acceptance_1");
    TH1D* ystar_acceptance_1 = (TH1D*)hist_2d_acceptance_1->ProjectionX()->Clone("ystar_acceptance_1");
    //TH1D* acceptance_1 = (TH1D*)hist_2d_acceptance_1->ProjectionX()->Clone("acceptance_1");
    
    gStyle->SetOptStat(0);
    compare_plot({ ystar_ac, ystar_ac_1},"in_iter_ystar","","",true);
    compare_plot({ costheta_ac, costheta_ac_1},"in_iter_costheta","","",false);
    compare_plot({ cos_theta_acceptance, cos_theta_acceptance_1},"in_iter_costheta_accept","","",true);
    compare_plot({ ystar_acceptance, ystar_acceptance_1},"in_iter_ystar_accept","","",true);
    compare_plot({ costheta, costheta_ac},"in_iter_costheta_noac","","",false);

    return 0;
}

void compare_plot(vector<TH1D*> vec_hist, string save_name, string leg_title, string axis_title, Bool_t make_leg = true)
{
  // Plot histogram(s) algorithm: gxana::DrawOverlay (gxana/common/Overlay.h)
  //style_format();
  gStyle->SetPadTopMargin   (0.08);
  gStyle->SetPadLeftMargin  (0.16); 
  gxana::OverlayOpts opts;
  opts.saveName = save_name;
  opts.legendTitle = leg_title;
  opts.axisTitle = axis_title;
  opts.label1 = "Originial";
  opts.label2 = "Iteration";
  opts.legendTopRight = make_leg;
  opts.firstAsPoints = true;
  gxana::DrawOverlay(vec_hist[0], vec_hist[1], opts);
}
