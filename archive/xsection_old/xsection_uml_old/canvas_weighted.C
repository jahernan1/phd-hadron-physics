void canvas_weighted()
{
//=========Macro generated from canvas: canvas/diff_xsection_weighted
//=========  (Thu Nov 21 11:30:35 2024) by ROOT version 6.24/04
   TCanvas *canvas = new TCanvas("canvas", "diff_xsection_weighted",0,0,1000,700);
   gStyle->SetOptStat(0);
   canvas->Range(0,0,1,1);
   canvas->SetFillColor(0);
   canvas->SetBorderMode(0);
   canvas->SetBorderSize(5);
   canvas->SetLeftMargin(0.16);
   canvas->SetRightMargin(0.04);
   canvas->SetTopMargin(0.08);
   canvas->SetBottomMargin(0.2);
   canvas->SetFrameFillStyle(0);
   canvas->SetFrameLineStyle(0);
   canvas->SetFrameLineWidth(2);
   canvas->SetFrameBorderMode(0);
   canvas->SetFrameBorderSize(10);
  
// ------------>Primitives in pad: pad
   TPad *pad = new TPad("pad", "pad",0.1,0.1,0.95,0.95);
   pad->Draw();
   pad->cd();
   pad->Range(0,0,1,1);
   pad->SetFillColor(0);
   pad->SetBorderMode(0);
   pad->SetBorderSize(10);
   pad->SetLeftMargin(0.16);
   pad->SetRightMargin(0.04);
   pad->SetTopMargin(0.08);
   pad->SetBottomMargin(0.2);
   pad->SetFrameFillStyle(0);
   pad->SetFrameLineStyle(0);
   pad->SetFrameLineWidth(2);
   pad->SetFrameBorderMode(0);
   pad->SetFrameBorderSize(10);
  
// ------------>Primitives in pad: pad_1
   TPad *pad_1 = new TPad("pad_1", "pad_1",1e-05,0.6666767,0.3333233,0.99999);
   pad_1->Draw();
   pad_1->cd();
   pad_1->Range(0.02627501,-0.82393,2.524825,1.301051);
   pad_1->SetFillColor(0);
   pad_1->SetFillStyle(4000);
   pad_1->SetBorderMode(0);
   pad_1->SetBorderSize(10);
   pad_1->SetLogy();
   pad_1->SetLeftMargin(1e-05);
   pad_1->SetRightMargin(1e-05);
   pad_1->SetTopMargin(1e-05);
   pad_1->SetBottomMargin(1e-05);
   pad_1->SetFrameFillStyle(0);
   pad_1->SetFrameLineStyle(0);
   pad_1->SetFrameLineWidth(2);
   pad_1->SetFrameBorderMode(0);
   pad_1->SetFrameBorderSize(10);
   pad_1->SetFrameFillStyle(0);
   pad_1->SetFrameLineStyle(0);
   pad_1->SetFrameLineWidth(2);
   pad_1->SetFrameBorderMode(0);
   pad_1->SetFrameBorderSize(10);
   
   Double_t Graph0_fx1001[7] = {
   0.225,
   0.44,
   0.62,
   0.815,
   1.055,
   1.36,
   1.965};
   Double_t Graph0_fy1001[7] = {
   4.7003,
   5.781739,
   5.26695,
   3.188557,
   2.446217,
   2.206484,
   0.6925566};
   Double_t Graph0_fex1001[7] = {
   0.125,
   0.09,
   0.09,
   0.105,
   0.135,
   0.17,
   0.435};
   Double_t Graph0_fey1001[7] = {
   0.3968126,
   0.4112757,
   0.3582654,
   0.264711,
   0.2007314,
   0.2007823,
   0.08718257};
   TGraphErrors *gre = new TGraphErrors(7,Graph0_fx1001,Graph0_fy1001,Graph0_fex1001,Graph0_fey1001);
   gre->SetName("Graph0");
   gre->SetTitle("#bf{E_{#gamma} (GeV): (6.4, 7.4)}");
   gre->SetFillStyle(0);

   Int_t ci;      // for color index setting
   TColor *color; // for color definition with alpha
   ci = TColor::GetColor("#0033ff");
   gre->SetLineColor(ci);
   gre->SetLineWidth(2);

   ci = TColor::GetColor("#0033ff");
   gre->SetMarkerColor(ci);
   gre->SetMarkerStyle(20);
   gre->SetMarkerSize(0.8);
   
   TH1F *Graph_Graph01001 = new TH1F("Graph_Graph01001","#bf{E_{#gamma} (GeV): (6.4, 7.4)}",100,0,2.63);
   Graph_Graph01001->SetMinimum(0.15);
   Graph_Graph01001->SetMaximum(20);
   Graph_Graph01001->SetDirectory(0);
   Graph_Graph01001->SetStats(0);

   ci = TColor::GetColor("#000099");
   Graph_Graph01001->SetLineColor(ci);
   Graph_Graph01001->SetMarkerStyle(20);
   Graph_Graph01001->GetXaxis()->SetTitle(" -t (GeV)^{2}");
   Graph_Graph01001->GetXaxis()->SetRange(2,96);
   Graph_Graph01001->GetXaxis()->SetNdivisions(505);
   Graph_Graph01001->GetXaxis()->SetLabelFont(132);
   Graph_Graph01001->GetXaxis()->SetLabelOffset(0.009);
   Graph_Graph01001->GetXaxis()->SetLabelSize(0.05);
   Graph_Graph01001->GetXaxis()->SetTitleSize(0.065);
   Graph_Graph01001->GetXaxis()->SetTitleOffset(1);
   Graph_Graph01001->GetXaxis()->SetTitleFont(132);
   Graph_Graph01001->GetYaxis()->SetTitle("d#sigma/dt (nb/GeV^{2} )");
   Graph_Graph01001->GetYaxis()->SetLabelFont(132);
   Graph_Graph01001->GetYaxis()->SetLabelOffset(0.009);
   Graph_Graph01001->GetYaxis()->SetLabelSize(0.05);
   Graph_Graph01001->GetYaxis()->SetTitleSize(0.065);
   Graph_Graph01001->GetYaxis()->SetTitleOffset(0.85);
   Graph_Graph01001->GetYaxis()->SetTitleFont(132);
   Graph_Graph01001->GetZaxis()->SetLabelFont(42);
   Graph_Graph01001->GetZaxis()->SetTitleOffset(1);
   Graph_Graph01001->GetZaxis()->SetTitleFont(42);
   gre->SetHistogram(Graph_Graph01001);
   
   gre->Draw("ap");
   
   TPaveText *pt = new TPaveText(0.2029523,0.8626391,0.7970477,0.995,"blNDC");
   pt->SetName("title");
   pt->SetBorderSize(0);
   pt->SetFillColor(0);
   pt->SetFillStyle(0);
   pt->SetTextFont(132);
   TText *pt_LaTex = pt->AddText("#bf{E_{#gamma} (GeV): (6.4, 7.4)}");
   pt->Draw();
   pad_1->Modified();
   pad->cd();
  
// ------------>Primitives in pad: pad_2
   TPad *pad_2 = new TPad("pad_2", "pad_2",0.3333433,0.6666767,0.6666567,0.99999);
   pad_2->Draw();
   pad_2->cd();
   pad_2->Range(0.02627501,-0.82393,2.524825,1.301051);
   pad_2->SetFillColor(0);
   pad_2->SetFillStyle(4000);
   pad_2->SetBorderMode(0);
   pad_2->SetBorderSize(10);
   pad_2->SetLogy();
   pad_2->SetLeftMargin(1e-05);
   pad_2->SetRightMargin(1e-05);
   pad_2->SetTopMargin(1e-05);
   pad_2->SetBottomMargin(1e-05);
   pad_2->SetFrameFillStyle(0);
   pad_2->SetFrameLineStyle(0);
   pad_2->SetFrameLineWidth(2);
   pad_2->SetFrameBorderMode(0);
   pad_2->SetFrameBorderSize(10);
   pad_2->SetFrameFillStyle(0);
   pad_2->SetFrameLineStyle(0);
   pad_2->SetFrameLineWidth(2);
   pad_2->SetFrameBorderMode(0);
   pad_2->SetFrameBorderSize(10);
   
   Double_t Graph0_fx1002[7] = {
   0.225,
   0.44,
   0.62,
   0.815,
   1.055,
   1.36,
   1.965};
   Double_t Graph0_fy1002[7] = {
   4.223153,
   5.224067,
   4.17754,
   3.206139,
   2.371134,
   1.642617,
   0.4594856};
   Double_t Graph0_fex1002[7] = {
   0.125,
   0.09,
   0.09,
   0.105,
   0.135,
   0.17,
   0.435};
   Double_t Graph0_fey1002[7] = {
   0.3905703,
   0.3834806,
   0.3353157,
   0.2663194,
   0.1944834,
   0.1545011,
   0.06287808};
   gre = new TGraphErrors(7,Graph0_fx1002,Graph0_fy1002,Graph0_fex1002,Graph0_fey1002);
   gre->SetName("Graph0");
   gre->SetTitle("#bf{E_{#gamma} (GeV): (7.4, 7.86)}");
   gre->SetFillStyle(0);

   ci = TColor::GetColor("#0033ff");
   gre->SetLineColor(ci);
   gre->SetLineWidth(2);

   ci = TColor::GetColor("#0033ff");
   gre->SetMarkerColor(ci);
   gre->SetMarkerStyle(20);
   gre->SetMarkerSize(0.8);
   
   TH1F *Graph_Graph01002 = new TH1F("Graph_Graph01002","#bf{E_{#gamma} (GeV): (7.4, 7.86)}",100,0,2.63);
   Graph_Graph01002->SetMinimum(0.15);
   Graph_Graph01002->SetMaximum(20);
   Graph_Graph01002->SetDirectory(0);
   Graph_Graph01002->SetStats(0);

   ci = TColor::GetColor("#000099");
   Graph_Graph01002->SetLineColor(ci);
   Graph_Graph01002->SetMarkerStyle(20);
   Graph_Graph01002->GetXaxis()->SetTitle(" -t (GeV)^{2}");
   Graph_Graph01002->GetXaxis()->SetRange(2,96);
   Graph_Graph01002->GetXaxis()->SetNdivisions(505);
   Graph_Graph01002->GetXaxis()->SetLabelFont(132);
   Graph_Graph01002->GetXaxis()->SetLabelOffset(0.009);
   Graph_Graph01002->GetXaxis()->SetLabelSize(0.05);
   Graph_Graph01002->GetXaxis()->SetTitleSize(0.065);
   Graph_Graph01002->GetXaxis()->SetTitleOffset(1);
   Graph_Graph01002->GetXaxis()->SetTitleFont(132);
   Graph_Graph01002->GetYaxis()->SetTitle("d#sigma/dt (nb/GeV^{2} )");
   Graph_Graph01002->GetYaxis()->SetLabelFont(132);
   Graph_Graph01002->GetYaxis()->SetLabelOffset(0.009);
   Graph_Graph01002->GetYaxis()->SetLabelSize(0.05);
   Graph_Graph01002->GetYaxis()->SetTitleSize(0.065);
   Graph_Graph01002->GetYaxis()->SetTitleOffset(0.85);
   Graph_Graph01002->GetYaxis()->SetTitleFont(132);
   Graph_Graph01002->GetZaxis()->SetLabelFont(42);
   Graph_Graph01002->GetZaxis()->SetTitleOffset(1);
   Graph_Graph01002->GetZaxis()->SetTitleFont(42);
   gre->SetHistogram(Graph_Graph01002);
   
   gre->Draw("ap");
   
   pt = new TPaveText(0.1852333,0.8626391,0.8147667,0.995,"blNDC");
   pt->SetName("title");
   pt->SetBorderSize(0);
   pt->SetFillColor(0);
   pt->SetFillStyle(0);
   pt->SetTextFont(132);
   pt_LaTex = pt->AddText("#bf{E_{#gamma} (GeV): (7.4, 7.86)}");
   pt->Draw();
   pad_2->Modified();
   pad->cd();
  
// ------------>Primitives in pad: pad_3
   TPad *pad_3 = new TPad("pad_3", "pad_3",0.6666767,0.6666767,0.99999,0.99999);
   pad_3->Draw();
   pad_3->cd();
   pad_3->Range(0.02627501,-0.82393,2.524825,1.301051);
   pad_3->SetFillColor(0);
   pad_3->SetFillStyle(4000);
   pad_3->SetBorderMode(0);
   pad_3->SetBorderSize(10);
   pad_3->SetLogy();
   pad_3->SetLeftMargin(1e-05);
   pad_3->SetRightMargin(1e-05);
   pad_3->SetTopMargin(1e-05);
   pad_3->SetBottomMargin(1e-05);
   pad_3->SetFrameFillStyle(0);
   pad_3->SetFrameLineStyle(0);
   pad_3->SetFrameLineWidth(2);
   pad_3->SetFrameBorderMode(0);
   pad_3->SetFrameBorderSize(10);
   pad_3->SetFrameFillStyle(0);
   pad_3->SetFrameLineStyle(0);
   pad_3->SetFrameLineWidth(2);
   pad_3->SetFrameBorderMode(0);
   pad_3->SetFrameBorderSize(10);
   
   Double_t Graph0_fx1003[7] = {
   0.225,
   0.44,
   0.62,
   0.815,
   1.055,
   1.36,
   1.965};
   Double_t Graph0_fy1003[7] = {
   4.571205,
   5.282614,
   3.784485,
   2.863539,
   2.104334,
   1.245826,
   0.4926457};
   Double_t Graph0_fex1003[7] = {
   0.125,
   0.09,
   0.09,
   0.105,
   0.135,
   0.17,
   0.435};
   Double_t Graph0_fey1003[7] = {
   0.3814202,
   0.3852845,
   0.2944375,
   0.2429044,
   0.1730979,
   0.1305461,
   0.0564776};
   gre = new TGraphErrors(7,Graph0_fx1003,Graph0_fy1003,Graph0_fex1003,Graph0_fey1003);
   gre->SetName("Graph0");
   gre->SetTitle("#bf{E_{#gamma} (GeV): (7.86, 8.19)}");
   gre->SetFillStyle(0);

   ci = TColor::GetColor("#0033ff");
   gre->SetLineColor(ci);
   gre->SetLineWidth(2);

   ci = TColor::GetColor("#0033ff");
   gre->SetMarkerColor(ci);
   gre->SetMarkerStyle(20);
   gre->SetMarkerSize(0.8);
   
   TH1F *Graph_Graph01003 = new TH1F("Graph_Graph01003","#bf{E_{#gamma} (GeV): (7.86, 8.19)}",100,0,2.63);
   Graph_Graph01003->SetMinimum(0.15);
   Graph_Graph01003->SetMaximum(20);
   Graph_Graph01003->SetDirectory(0);
   Graph_Graph01003->SetStats(0);

   ci = TColor::GetColor("#000099");
   Graph_Graph01003->SetLineColor(ci);
   Graph_Graph01003->SetMarkerStyle(20);
   Graph_Graph01003->GetXaxis()->SetTitle(" -t (GeV)^{2}");
   Graph_Graph01003->GetXaxis()->SetRange(2,96);
   Graph_Graph01003->GetXaxis()->SetNdivisions(505);
   Graph_Graph01003->GetXaxis()->SetLabelFont(132);
   Graph_Graph01003->GetXaxis()->SetLabelOffset(0.009);
   Graph_Graph01003->GetXaxis()->SetLabelSize(0.05);
   Graph_Graph01003->GetXaxis()->SetTitleSize(0.065);
   Graph_Graph01003->GetXaxis()->SetTitleOffset(1);
   Graph_Graph01003->GetXaxis()->SetTitleFont(132);
   Graph_Graph01003->GetYaxis()->SetTitle("d#sigma/dt (nb/GeV^{2} )");
   Graph_Graph01003->GetYaxis()->SetLabelFont(132);
   Graph_Graph01003->GetYaxis()->SetLabelOffset(0.009);
   Graph_Graph01003->GetYaxis()->SetLabelSize(0.05);
   Graph_Graph01003->GetYaxis()->SetTitleSize(0.065);
   Graph_Graph01003->GetYaxis()->SetTitleOffset(0.85);
   Graph_Graph01003->GetYaxis()->SetTitleFont(132);
   Graph_Graph01003->GetZaxis()->SetLabelFont(42);
   Graph_Graph01003->GetZaxis()->SetTitleOffset(1);
   Graph_Graph01003->GetZaxis()->SetTitleFont(42);
   gre->SetHistogram(Graph_Graph01003);
   
   gre->Draw("ap");
   
   pt = new TPaveText(0.1675143,0.8626391,0.8324857,0.995,"blNDC");
   pt->SetName("title");
   pt->SetBorderSize(0);
   pt->SetFillColor(0);
   pt->SetFillStyle(0);
   pt->SetTextFont(132);
   pt_LaTex = pt->AddText("#bf{E_{#gamma} (GeV): (7.86, 8.19)}");
   pt->Draw();
   pad_3->Modified();
   pad->cd();
  
// ------------>Primitives in pad: pad_4
   TPad *pad_4 = new TPad("pad_4", "pad_4",1e-05,0.3333433,0.3333233,0.6666567);
   pad_4->Draw();
   pad_4->cd();
   pad_4->Range(0.02627501,-0.82393,2.524825,1.301051);
   pad_4->SetFillColor(0);
   pad_4->SetFillStyle(4000);
   pad_4->SetBorderMode(0);
   pad_4->SetBorderSize(10);
   pad_4->SetLogy();
   pad_4->SetLeftMargin(1e-05);
   pad_4->SetRightMargin(1e-05);
   pad_4->SetTopMargin(1e-05);
   pad_4->SetBottomMargin(1e-05);
   pad_4->SetFrameFillStyle(0);
   pad_4->SetFrameLineStyle(0);
   pad_4->SetFrameLineWidth(2);
   pad_4->SetFrameBorderMode(0);
   pad_4->SetFrameBorderSize(10);
   pad_4->SetFrameFillStyle(0);
   pad_4->SetFrameLineStyle(0);
   pad_4->SetFrameLineWidth(2);
   pad_4->SetFrameBorderMode(0);
   pad_4->SetFrameBorderSize(10);
   
   Double_t Graph0_fx1004[7] = {
   0.225,
   0.44,
   0.62,
   0.815,
   1.055,
   1.36,
   1.965};
   Double_t Graph0_fy1004[7] = {
   3.974639,
   4.835848,
   3.782814,
   2.790717,
   2.082388,
   1.360184,
   0.3763131};
   Double_t Graph0_fex1004[7] = {
   0.125,
   0.09,
   0.09,
   0.105,
   0.135,
   0.17,
   0.435};
   Double_t Graph0_fey1004[7] = {
   0.367308,
   0.3740403,
   0.2820898,
   0.2278037,
   0.1691078,
   0.1228255,
   0.04839375};
   gre = new TGraphErrors(7,Graph0_fx1004,Graph0_fy1004,Graph0_fex1004,Graph0_fey1004);
   gre->SetName("Graph0");
   gre->SetTitle("#bf{E_{#gamma} (GeV): (8.19, 8.45)}");
   gre->SetFillStyle(0);

   ci = TColor::GetColor("#0033ff");
   gre->SetLineColor(ci);
   gre->SetLineWidth(2);

   ci = TColor::GetColor("#0033ff");
   gre->SetMarkerColor(ci);
   gre->SetMarkerStyle(20);
   gre->SetMarkerSize(0.8);
   
   TH1F *Graph_Graph01004 = new TH1F("Graph_Graph01004","#bf{E_{#gamma} (GeV): (8.19, 8.45)}",100,0,2.63);
   Graph_Graph01004->SetMinimum(0.15);
   Graph_Graph01004->SetMaximum(20);
   Graph_Graph01004->SetDirectory(0);
   Graph_Graph01004->SetStats(0);

   ci = TColor::GetColor("#000099");
   Graph_Graph01004->SetLineColor(ci);
   Graph_Graph01004->SetMarkerStyle(20);
   Graph_Graph01004->GetXaxis()->SetTitle(" -t (GeV)^{2}");
   Graph_Graph01004->GetXaxis()->SetRange(2,96);
   Graph_Graph01004->GetXaxis()->SetNdivisions(505);
   Graph_Graph01004->GetXaxis()->SetLabelFont(132);
   Graph_Graph01004->GetXaxis()->SetLabelOffset(0.009);
   Graph_Graph01004->GetXaxis()->SetLabelSize(0.05);
   Graph_Graph01004->GetXaxis()->SetTitleSize(0.065);
   Graph_Graph01004->GetXaxis()->SetTitleOffset(1);
   Graph_Graph01004->GetXaxis()->SetTitleFont(132);
   Graph_Graph01004->GetYaxis()->SetTitle("d#sigma/dt (nb/GeV^{2} )");
   Graph_Graph01004->GetYaxis()->SetLabelFont(132);
   Graph_Graph01004->GetYaxis()->SetLabelOffset(0.009);
   Graph_Graph01004->GetYaxis()->SetLabelSize(0.05);
   Graph_Graph01004->GetYaxis()->SetTitleSize(0.065);
   Graph_Graph01004->GetYaxis()->SetTitleOffset(0.85);
   Graph_Graph01004->GetYaxis()->SetTitleFont(132);
   Graph_Graph01004->GetZaxis()->SetLabelFont(42);
   Graph_Graph01004->GetZaxis()->SetTitleOffset(1);
   Graph_Graph01004->GetZaxis()->SetTitleFont(42);
   gre->SetHistogram(Graph_Graph01004);
   
   gre->Draw("ap");
   
   pt = new TPaveText(0.1675143,0.8626391,0.8324857,0.995,"blNDC");
   pt->SetName("title");
   pt->SetBorderSize(0);
   pt->SetFillColor(0);
   pt->SetFillStyle(0);
   pt->SetTextFont(132);
   pt_LaTex = pt->AddText("#bf{E_{#gamma} (GeV): (8.19, 8.45)}");
   pt->Draw();
   pad_4->Modified();
   pad->cd();
  
// ------------>Primitives in pad: pad_5
   TPad *pad_5 = new TPad("pad_5", "pad_5",0.3333433,0.3333433,0.6666567,0.6666567);
   pad_5->Draw();
   pad_5->cd();
   pad_5->Range(0.02627501,-0.82393,2.524825,1.301051);
   pad_5->SetFillColor(0);
   pad_5->SetFillStyle(4000);
   pad_5->SetBorderMode(0);
   pad_5->SetBorderSize(10);
   pad_5->SetLogy();
   pad_5->SetLeftMargin(1e-05);
   pad_5->SetRightMargin(1e-05);
   pad_5->SetTopMargin(1e-05);
   pad_5->SetBottomMargin(1e-05);
   pad_5->SetFrameFillStyle(0);
   pad_5->SetFrameLineStyle(0);
   pad_5->SetFrameLineWidth(2);
   pad_5->SetFrameBorderMode(0);
   pad_5->SetFrameBorderSize(10);
   pad_5->SetFrameFillStyle(0);
   pad_5->SetFrameLineStyle(0);
   pad_5->SetFrameLineWidth(2);
   pad_5->SetFrameBorderMode(0);
   pad_5->SetFrameBorderSize(10);
   
   Double_t Graph0_fx1005[7] = {
   0.225,
   0.44,
   0.62,
   0.815,
   1.055,
   1.36,
   1.965};
   Double_t Graph0_fy1005[7] = {
   4.098701,
   3.965903,
   3.425352,
   2.931407,
   1.867245,
   1.036552,
   0.4453709};
   Double_t Graph0_fex1005[7] = {
   0.125,
   0.09,
   0.09,
   0.105,
   0.135,
   0.17,
   0.435};
   Double_t Graph0_fey1005[7] = {
   0.3682863,
   0.3187697,
   0.2726526,
   0.2182471,
   0.1464603,
   0.1041863,
   0.04603017};
   gre = new TGraphErrors(7,Graph0_fx1005,Graph0_fy1005,Graph0_fex1005,Graph0_fey1005);
   gre->SetName("Graph0");
   gre->SetTitle("#bf{E_{#gamma} (GeV): (8.45, 8.68)}");
   gre->SetFillStyle(0);

   ci = TColor::GetColor("#0033ff");
   gre->SetLineColor(ci);
   gre->SetLineWidth(2);

   ci = TColor::GetColor("#0033ff");
   gre->SetMarkerColor(ci);
   gre->SetMarkerStyle(20);
   gre->SetMarkerSize(0.8);
   
   TH1F *Graph_Graph01005 = new TH1F("Graph_Graph01005","#bf{E_{#gamma} (GeV): (8.45, 8.68)}",100,0,2.63);
   Graph_Graph01005->SetMinimum(0.15);
   Graph_Graph01005->SetMaximum(20);
   Graph_Graph01005->SetDirectory(0);
   Graph_Graph01005->SetStats(0);

   ci = TColor::GetColor("#000099");
   Graph_Graph01005->SetLineColor(ci);
   Graph_Graph01005->SetMarkerStyle(20);
   Graph_Graph01005->GetXaxis()->SetTitle(" -t (GeV)^{2}");
   Graph_Graph01005->GetXaxis()->SetRange(2,96);
   Graph_Graph01005->GetXaxis()->SetNdivisions(505);
   Graph_Graph01005->GetXaxis()->SetLabelFont(132);
   Graph_Graph01005->GetXaxis()->SetLabelOffset(0.009);
   Graph_Graph01005->GetXaxis()->SetLabelSize(0.05);
   Graph_Graph01005->GetXaxis()->SetTitleSize(0.065);
   Graph_Graph01005->GetXaxis()->SetTitleOffset(1);
   Graph_Graph01005->GetXaxis()->SetTitleFont(132);
   Graph_Graph01005->GetYaxis()->SetTitle("d#sigma/dt (nb/GeV^{2} )");
   Graph_Graph01005->GetYaxis()->SetLabelFont(132);
   Graph_Graph01005->GetYaxis()->SetLabelOffset(0.009);
   Graph_Graph01005->GetYaxis()->SetLabelSize(0.05);
   Graph_Graph01005->GetYaxis()->SetTitleSize(0.065);
   Graph_Graph01005->GetYaxis()->SetTitleOffset(0.85);
   Graph_Graph01005->GetYaxis()->SetTitleFont(132);
   Graph_Graph01005->GetZaxis()->SetLabelFont(42);
   Graph_Graph01005->GetZaxis()->SetTitleOffset(1);
   Graph_Graph01005->GetZaxis()->SetTitleFont(42);
   gre->SetHistogram(Graph_Graph01005);
   
   gre->Draw("ap");
   
   pt = new TPaveText(0.1675143,0.8626391,0.8324857,0.995,"blNDC");
   pt->SetName("title");
   pt->SetBorderSize(0);
   pt->SetFillColor(0);
   pt->SetFillStyle(0);
   pt->SetTextFont(132);
   pt_LaTex = pt->AddText("#bf{E_{#gamma} (GeV): (8.45, 8.68)}");
   pt->Draw();
   pad_5->Modified();
   pad->cd();
  
// ------------>Primitives in pad: pad_6
   TPad *pad_6 = new TPad("pad_6", "pad_6",0.6666767,0.3333433,0.99999,0.6666567);
   pad_6->Draw();
   pad_6->cd();
   pad_6->Range(0.02627501,-0.82393,2.524825,1.301051);
   pad_6->SetFillColor(0);
   pad_6->SetFillStyle(4000);
   pad_6->SetBorderMode(0);
   pad_6->SetBorderSize(10);
   pad_6->SetLogy();
   pad_6->SetLeftMargin(1e-05);
   pad_6->SetRightMargin(1e-05);
   pad_6->SetTopMargin(1e-05);
   pad_6->SetBottomMargin(1e-05);
   pad_6->SetFrameFillStyle(0);
   pad_6->SetFrameLineStyle(0);
   pad_6->SetFrameLineWidth(2);
   pad_6->SetFrameBorderMode(0);
   pad_6->SetFrameBorderSize(10);
   pad_6->SetFrameFillStyle(0);
   pad_6->SetFrameLineStyle(0);
   pad_6->SetFrameLineWidth(2);
   pad_6->SetFrameBorderMode(0);
   pad_6->SetFrameBorderSize(10);
   
   Double_t Graph0_fx1006[7] = {
   0.225,
   0.44,
   0.62,
   0.815,
   1.055,
   1.36,
   1.965};
   Double_t Graph0_fy1006[7] = {
   3.527354,
   3.796487,
   3.169531,
   2.381799,
   1.754569,
   1.074088,
   0.3384508};
   Double_t Graph0_fex1006[7] = {
   0.125,
   0.09,
   0.09,
   0.105,
   0.135,
   0.17,
   0.435};
   Double_t Graph0_fey1006[7] = {
   0.3235466,
   0.2953325,
   0.2324031,
   0.1800136,
   0.1352136,
   0.09736657,
   0.0363803};
   gre = new TGraphErrors(7,Graph0_fx1006,Graph0_fy1006,Graph0_fex1006,Graph0_fey1006);
   gre->SetName("Graph0");
   gre->SetTitle("#bf{E_{#gamma} (GeV): (8.68, 9.26)}");
   gre->SetFillStyle(0);

   ci = TColor::GetColor("#0033ff");
   gre->SetLineColor(ci);
   gre->SetLineWidth(2);

   ci = TColor::GetColor("#0033ff");
   gre->SetMarkerColor(ci);
   gre->SetMarkerStyle(20);
   gre->SetMarkerSize(0.8);
   
   TH1F *Graph_Graph01006 = new TH1F("Graph_Graph01006","#bf{E_{#gamma} (GeV): (8.68, 9.26)}",100,0,2.63);
   Graph_Graph01006->SetMinimum(0.15);
   Graph_Graph01006->SetMaximum(20);
   Graph_Graph01006->SetDirectory(0);
   Graph_Graph01006->SetStats(0);

   ci = TColor::GetColor("#000099");
   Graph_Graph01006->SetLineColor(ci);
   Graph_Graph01006->SetMarkerStyle(20);
   Graph_Graph01006->GetXaxis()->SetTitle(" -t (GeV)^{2}");
   Graph_Graph01006->GetXaxis()->SetRange(2,96);
   Graph_Graph01006->GetXaxis()->SetNdivisions(505);
   Graph_Graph01006->GetXaxis()->SetLabelFont(132);
   Graph_Graph01006->GetXaxis()->SetLabelOffset(0.009);
   Graph_Graph01006->GetXaxis()->SetLabelSize(0.05);
   Graph_Graph01006->GetXaxis()->SetTitleSize(0.065);
   Graph_Graph01006->GetXaxis()->SetTitleOffset(1);
   Graph_Graph01006->GetXaxis()->SetTitleFont(132);
   Graph_Graph01006->GetYaxis()->SetTitle("d#sigma/dt (nb/GeV^{2} )");
   Graph_Graph01006->GetYaxis()->SetLabelFont(132);
   Graph_Graph01006->GetYaxis()->SetLabelOffset(0.009);
   Graph_Graph01006->GetYaxis()->SetLabelSize(0.05);
   Graph_Graph01006->GetYaxis()->SetTitleSize(0.065);
   Graph_Graph01006->GetYaxis()->SetTitleOffset(0.85);
   Graph_Graph01006->GetYaxis()->SetTitleFont(132);
   Graph_Graph01006->GetZaxis()->SetLabelFont(42);
   Graph_Graph01006->GetZaxis()->SetTitleOffset(1);
   Graph_Graph01006->GetZaxis()->SetTitleFont(42);
   gre->SetHistogram(Graph_Graph01006);
   
   gre->Draw("ap");
   
   pt = new TPaveText(0.1675143,0.8626391,0.8324857,0.995,"blNDC");
   pt->SetName("title");
   pt->SetBorderSize(0);
   pt->SetFillColor(0);
   pt->SetFillStyle(0);
   pt->SetTextFont(132);
   pt_LaTex = pt->AddText("#bf{E_{#gamma} (GeV): (8.68, 9.26)}");
   pt->Draw();
   pad_6->Modified();
   pad->cd();
  
// ------------>Primitives in pad: pad_7
   TPad *pad_7 = new TPad("pad_7", "pad_7",1e-05,1e-05,0.3333233,0.3333233);
   pad_7->Draw();
   pad_7->cd();
   pad_7->Range(0.02627501,-0.82393,2.524825,1.301051);
   pad_7->SetFillColor(0);
   pad_7->SetFillStyle(4000);
   pad_7->SetBorderMode(0);
   pad_7->SetBorderSize(10);
   pad_7->SetLogy();
   pad_7->SetLeftMargin(1e-05);
   pad_7->SetRightMargin(1e-05);
   pad_7->SetTopMargin(1e-05);
   pad_7->SetBottomMargin(1e-05);
   pad_7->SetFrameFillStyle(0);
   pad_7->SetFrameLineStyle(0);
   pad_7->SetFrameLineWidth(2);
   pad_7->SetFrameBorderMode(0);
   pad_7->SetFrameBorderSize(10);
   pad_7->SetFrameFillStyle(0);
   pad_7->SetFrameLineStyle(0);
   pad_7->SetFrameLineWidth(2);
   pad_7->SetFrameBorderMode(0);
   pad_7->SetFrameBorderSize(10);
   
   Double_t Graph0_fx1007[7] = {
   0.225,
   0.44,
   0.62,
   0.815,
   1.055,
   1.36,
   1.965};
   Double_t Graph0_fy1007[7] = {
   3.496028,
   3.425818,
   2.888063,
   1.995225,
   1.512402,
   0.8469987,
   0.3643416};
   Double_t Graph0_fex1007[7] = {
   0.125,
   0.09,
   0.09,
   0.105,
   0.135,
   0.17,
   0.435};
   Double_t Graph0_fey1007[7] = {
   0.3035691,
   0.2616314,
   0.2159268,
   0.15292,
   0.1170766,
   0.07726316,
   0.0345334};
   gre = new TGraphErrors(7,Graph0_fx1007,Graph0_fy1007,Graph0_fex1007,Graph0_fey1007);
   gre->SetName("Graph0");
   gre->SetTitle("#bf{E_{#gamma} (GeV): (9.26, 10.18)}");
   gre->SetFillStyle(0);

   ci = TColor::GetColor("#0033ff");
   gre->SetLineColor(ci);
   gre->SetLineWidth(2);

   ci = TColor::GetColor("#0033ff");
   gre->SetMarkerColor(ci);
   gre->SetMarkerStyle(20);
   gre->SetMarkerSize(0.8);
   
   TH1F *Graph_Graph01007 = new TH1F("Graph_Graph01007","#bf{E_{#gamma} (GeV): (9.26, 10.18)}",100,0,2.63);
   Graph_Graph01007->SetMinimum(0.15);
   Graph_Graph01007->SetMaximum(20);
   Graph_Graph01007->SetDirectory(0);
   Graph_Graph01007->SetStats(0);

   ci = TColor::GetColor("#000099");
   Graph_Graph01007->SetLineColor(ci);
   Graph_Graph01007->SetMarkerStyle(20);
   Graph_Graph01007->GetXaxis()->SetTitle(" -t (GeV)^{2}");
   Graph_Graph01007->GetXaxis()->SetRange(2,96);
   Graph_Graph01007->GetXaxis()->SetNdivisions(505);
   Graph_Graph01007->GetXaxis()->SetLabelFont(132);
   Graph_Graph01007->GetXaxis()->SetLabelOffset(0.009);
   Graph_Graph01007->GetXaxis()->SetLabelSize(0.05);
   Graph_Graph01007->GetXaxis()->SetTitleSize(0.065);
   Graph_Graph01007->GetXaxis()->SetTitleOffset(1);
   Graph_Graph01007->GetXaxis()->SetTitleFont(132);
   Graph_Graph01007->GetYaxis()->SetTitle("d#sigma/dt (nb/GeV^{2} )");
   Graph_Graph01007->GetYaxis()->SetLabelFont(132);
   Graph_Graph01007->GetYaxis()->SetLabelOffset(0.009);
   Graph_Graph01007->GetYaxis()->SetLabelSize(0.05);
   Graph_Graph01007->GetYaxis()->SetTitleSize(0.065);
   Graph_Graph01007->GetYaxis()->SetTitleOffset(0.85);
   Graph_Graph01007->GetYaxis()->SetTitleFont(132);
   Graph_Graph01007->GetZaxis()->SetLabelFont(42);
   Graph_Graph01007->GetZaxis()->SetTitleOffset(1);
   Graph_Graph01007->GetZaxis()->SetTitleFont(42);
   gre->SetHistogram(Graph_Graph01007);
   
   gre->Draw("ap");
   
   pt = new TPaveText(0.15,0.8626391,0.85,0.995,"blNDC");
   pt->SetName("title");
   pt->SetBorderSize(0);
   pt->SetFillColor(0);
   pt->SetFillStyle(0);
   pt->SetTextFont(132);
   pt_LaTex = pt->AddText("#bf{E_{#gamma} (GeV): (9.26, 10.18)}");
   pt->Draw();
   pad_7->Modified();
   pad->cd();
  
// ------------>Primitives in pad: pad_8
   TPad *pad_8 = new TPad("pad_8", "pad_8",0.3333433,1e-05,0.6666567,0.3333233);
   pad_8->Draw();
   pad_8->cd();
   pad_8->Range(0.02627501,-0.82393,2.524825,1.301051);
   pad_8->SetFillColor(0);
   pad_8->SetFillStyle(4000);
   pad_8->SetBorderMode(0);
   pad_8->SetBorderSize(10);
   pad_8->SetLogy();
   pad_8->SetLeftMargin(1e-05);
   pad_8->SetRightMargin(1e-05);
   pad_8->SetTopMargin(1e-05);
   pad_8->SetBottomMargin(1e-05);
   pad_8->SetFrameFillStyle(0);
   pad_8->SetFrameLineStyle(0);
   pad_8->SetFrameLineWidth(2);
   pad_8->SetFrameBorderMode(0);
   pad_8->SetFrameBorderSize(10);
   pad_8->SetFrameFillStyle(0);
   pad_8->SetFrameLineStyle(0);
   pad_8->SetFrameLineWidth(2);
   pad_8->SetFrameBorderMode(0);
   pad_8->SetFrameBorderSize(10);
   
   Double_t Graph0_fx1008[7] = {
   0.225,
   0.44,
   0.62,
   0.815,
   1.055,
   1.36,
   1.965};
   Double_t Graph0_fy1008[7] = {
   2.947881,
   2.714979,
   2.187341,
   2.057027,
   1.279618,
   0.7122751,
   0.2491067};
   Double_t Graph0_fex1008[7] = {
   0.125,
   0.09,
   0.09,
   0.105,
   0.135,
   0.17,
   0.435};
   Double_t Graph0_fey1008[7] = {
   0.2670893,
   0.2110182,
   0.167822,
   0.1414706,
   0.09837328,
   0.0642373,
   0.02555863};
   gre = new TGraphErrors(7,Graph0_fx1008,Graph0_fy1008,Graph0_fex1008,Graph0_fey1008);
   gre->SetName("Graph0");
   gre->SetTitle("#bf{E_{#gamma} (GeV): (10.18, 11.4)}");
   gre->SetFillStyle(0);

   ci = TColor::GetColor("#0033ff");
   gre->SetLineColor(ci);
   gre->SetLineWidth(2);

   ci = TColor::GetColor("#0033ff");
   gre->SetMarkerColor(ci);
   gre->SetMarkerStyle(20);
   gre->SetMarkerSize(0.8);
   
   TH1F *Graph_Graph01008 = new TH1F("Graph_Graph01008","#bf{E_{#gamma} (GeV): (10.18, 11.4)}",100,0,2.63);
   Graph_Graph01008->SetMinimum(0.15);
   Graph_Graph01008->SetMaximum(20);
   Graph_Graph01008->SetDirectory(0);
   Graph_Graph01008->SetStats(0);

   ci = TColor::GetColor("#000099");
   Graph_Graph01008->SetLineColor(ci);
   Graph_Graph01008->SetMarkerStyle(20);
   Graph_Graph01008->GetXaxis()->SetTitle(" -t (GeV)^{2}");
   Graph_Graph01008->GetXaxis()->SetRange(2,96);
   Graph_Graph01008->GetXaxis()->SetNdivisions(505);
   Graph_Graph01008->GetXaxis()->SetLabelFont(132);
   Graph_Graph01008->GetXaxis()->SetLabelOffset(0.009);
   Graph_Graph01008->GetXaxis()->SetLabelSize(0.05);
   Graph_Graph01008->GetXaxis()->SetTitleSize(0.065);
   Graph_Graph01008->GetXaxis()->SetTitleOffset(1);
   Graph_Graph01008->GetXaxis()->SetTitleFont(132);
   Graph_Graph01008->GetYaxis()->SetTitle("d#sigma/dt (nb/GeV^{2} )");
   Graph_Graph01008->GetYaxis()->SetLabelFont(132);
   Graph_Graph01008->GetYaxis()->SetLabelOffset(0.009);
   Graph_Graph01008->GetYaxis()->SetLabelSize(0.05);
   Graph_Graph01008->GetYaxis()->SetTitleSize(0.065);
   Graph_Graph01008->GetYaxis()->SetTitleOffset(0.85);
   Graph_Graph01008->GetYaxis()->SetTitleFont(132);
   Graph_Graph01008->GetZaxis()->SetLabelFont(42);
   Graph_Graph01008->GetZaxis()->SetTitleOffset(1);
   Graph_Graph01008->GetZaxis()->SetTitleFont(42);
   gre->SetHistogram(Graph_Graph01008);
   
   gre->Draw("ap");
   
   pt = new TPaveText(0.15,0.8626391,0.85,0.995,"blNDC");
   pt->SetName("title");
   pt->SetBorderSize(0);
   pt->SetFillColor(0);
   pt->SetFillStyle(0);
   pt->SetTextFont(132);
   pt_LaTex = pt->AddText("#bf{E_{#gamma} (GeV): (10.18, 11.4)}");
   pt->Draw();
   pad_8->Modified();
   pad->cd();
  
// ------------>Primitives in pad: pad_9
   TPad *pad_9 = new TPad("pad_9", "pad_9",0.6666767,1e-05,0.99999,0.3333233);
   pad_9->Draw();
   pad_9->cd();
   pad_9->Range(0,0,1,1);
   pad_9->SetFillColor(0);
   pad_9->SetBorderMode(0);
   pad_9->SetBorderSize(10);
   pad_9->SetLeftMargin(0.16);
   pad_9->SetRightMargin(0.04);
   pad_9->SetTopMargin(0.08);
   pad_9->SetBottomMargin(0.2);
   pad_9->SetFrameFillStyle(0);
   pad_9->SetFrameLineStyle(0);
   pad_9->SetFrameLineWidth(2);
   pad_9->SetFrameBorderMode(0);
   pad_9->SetFrameBorderSize(10);
   pad_9->Modified();
   pad->cd();
   pad->Modified();
   canvas->cd();
   TGaxis *gaxis = new TGaxis(0.1,0.6666667,0.1,0.95,0.15,20,510,"G");
   gaxis->SetLabelOffset(0.005);
   gaxis->SetLabelSize(0.04);
   gaxis->SetTickSize(0.03);
   gaxis->SetGridLength(0);
   gaxis->SetTitleOffset(1);
   gaxis->SetTitleSize(0.04);
   gaxis->SetTitleColor(1);
   gaxis->SetTitleFont(62);
   gaxis->Draw();
   gaxis = new TGaxis(0.1,0.3833333,0.1,0.6666667,0.15,20,510,"G");
   gaxis->SetLabelOffset(0.005);
   gaxis->SetLabelSize(0.04);
   gaxis->SetTickSize(0.03);
   gaxis->SetGridLength(0);
   gaxis->SetTitleOffset(1);
   gaxis->SetTitleSize(0.04);
   gaxis->SetTitleColor(1);
   gaxis->SetTitleFont(62);
   gaxis->Draw();
   gaxis = new TGaxis(0.1,0.1,0.1,0.3833333,0.15,20,510,"G");
   gaxis->SetLabelOffset(0.005);
   gaxis->SetLabelSize(0.04);
   gaxis->SetTickSize(0.03);
   gaxis->SetGridLength(0);
   gaxis->SetTitleOffset(1);
   gaxis->SetTitleSize(0.04);
   gaxis->SetTitleColor(1);
   gaxis->SetTitleFont(62);
   gaxis->Draw();
   gaxis = new TGaxis(0.3833333,0.1,0.6666667,0.1,0.05,2.5,205,"");
   gaxis->SetLabelOffset(0.005);
   gaxis->SetLabelSize(0.04);
   gaxis->SetTickSize(0.03);
   gaxis->SetGridLength(0);
   gaxis->SetTitleOffset(1);
   gaxis->SetTitleSize(0.04);
   gaxis->SetTitleColor(1);
   gaxis->SetTitleFont(62);
   gaxis->Draw();
   gaxis = new TGaxis(0.1,0.1,0.3833333,0.1,0.05,2.5,205,"");
   gaxis->SetLabelOffset(0.005);
   gaxis->SetLabelSize(0.04);
   gaxis->SetTickSize(0.03);
   gaxis->SetGridLength(0);
   gaxis->SetTitleOffset(1);
   gaxis->SetTitleSize(0.04);
   gaxis->SetTitleColor(1);
   gaxis->SetTitleFont(62);
   gaxis->Draw();
   gaxis = new TGaxis(0.6666667,0.3833333,0.95,0.3833333,0.05,2.5,205,"");
   gaxis->SetLabelOffset(0.005);
   gaxis->SetLabelSize(0.04);
   gaxis->SetTickSize(0.03);
   gaxis->SetGridLength(0);
   gaxis->SetTitleOffset(1);
   gaxis->SetTitleSize(0.04);
   gaxis->SetTitleColor(1);
   gaxis->SetTitleFont(62);
   gaxis->Draw();
   TLatex *   tex = new TLatex(0.5,0.01,"-t (GeV^{2} )");
   tex->SetTextFont(132);
   tex->SetTextSize(0.06);
   tex->SetLineWidth(2);
   tex->Draw();
      tex = new TLatex(0.05,0.55,"d#sigma/dt (nb/GeV^{2} )");
   tex->SetTextFont(132);
   tex->SetTextSize(0.06);
   tex->SetTextAngle(90);
   tex->SetLineWidth(2);
   tex->Draw();
   
   TLegend *leg = new TLegend(0.7,0.15,0.95,0.3,NULL,"brNDC");
   leg->SetBorderSize(0);
   leg->SetTextFont(132);
   leg->SetTextSize(0.05);
   leg->SetLineColor(1);
   leg->SetLineStyle(1);
   leg->SetLineWidth(1);
   leg->SetFillColor(0);
   leg->SetFillStyle(1001);
   TLegendEntry *entry=leg->AddEntry("Graph","GlueX-I","lep");

   ci = TColor::GetColor("#0033ff");
   entry->SetLineColor(ci);
   entry->SetLineStyle(1);
   entry->SetLineWidth(2);

   ci = TColor::GetColor("#0033ff");
   entry->SetMarkerColor(ci);
   entry->SetMarkerStyle(20);
   entry->SetMarkerSize(0.8);
   entry->SetTextFont(132);
   leg->Draw();
   canvas->Modified();
   canvas->cd();
   canvas->SetSelected(canvas);
}
