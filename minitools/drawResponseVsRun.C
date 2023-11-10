// Purpose: draw photon+jet response vs run to check time stability
#include "TFile.h"
#include "TProfile.h"
#include "TLatex.h"

#include "../tdrstyle_mod22.C"

// Clean
void clean(TH1 *p, double maxerr = 0.005) {
  
  for (int i = 1; i != p->GetNbinsX()+1; ++i) {
    if (p->GetBinError(i)==0 || p->GetBinError(i)>maxerr) {
      p->SetBinContent(i, 0);
      p->SetBinError(i, 0);
    }
  }
} // clean

void drawResponseVsRun() {

  setTDRStyle();
  TDirectory *curdir = gDirectory;

  // Open input files
  TFile *f = new TFile("rootfiles/GamHistosFill_data_Run3_v26.root","READ");
  assert(f && !f->IsZombie());

  f->cd("control");
  TDirectory *d = gDirectory;

  // Load input profiles
  TProfile *pr30b = (TProfile*)d->Get("pr30b"); clean(pr30b,0.006);
  TProfile *pr30m = (TProfile*)d->Get("pr30m"); clean(pr30m,0.006);
  TProfile *pr110b = (TProfile*)d->Get("pr110b"); clean(pr110b,0.003);
  TProfile *pr110m = (TProfile*)d->Get("pr110m"); clean(pr110m,0.003);

  // Scale BAL and pT30 results
  double kbal = 1.13;
  pr30b->Scale(kbal);
  pr110b->Scale(kbal);
  double kpt30 = 0.98;
  pr30b->Scale(kpt30);
  pr30m->Scale(kpt30);
  double kbal30 = 0.96;
  pr30b->Scale(kbal30);

  // Setup canvas
  //TH1D *h = tdrHist("h","Response",0.8,1.2,"Run",355300,371300);
  TH1D *h = tdrHist("h","Response",0.92,1.08,"Run",355300,371300);
  lumi_136TeV = "Photon+jet, Run 3, v26";
  extraText = "Private";
  TCanvas *c1 = tdrCanvas("c1",h,8,11);
  TLine *l = new TLine();
  TLatex *t = new TLatex();
  t->SetTextSize(0.045);// t->SetNDC();

  // Start drawing
  c1->cd();
  gPad->SetLogx();
  double x1(h->GetXaxis()->GetXmin()), x2(h->GetXaxis()->GetXmax());
  double y1(h->GetMinimum()), y2(h->GetMaximum());
  l->DrawLine(x1,1,x2,1);

  // 2023 Era definition
  // https://twiki.cern.ch/twiki/bin/viewauth/CMS/PdmVRun3Analysis#2023_Era_definition
  double run22c1b(355794), run22c1e(357486);
  l->DrawLine(run22c1b,y1,run22c1b,y2);
  l->DrawLine(run22c1e,y1,run22c1e,y2);
  t->DrawLatex(run22c1b+350,0.925,"22C");
  double run22d1b(357487), run22d1e(357733);
  double run22d2b(357734), run22d2e(358219);
  double run22d3b(358220), run22d3e(359021);
  l->DrawLine(run22d1b,y1,run22d1b,y2);
  l->SetLineStyle(kDashed);
  l->DrawLine(run22d1e,y1+0.015,run22d1e,y2);
  l->DrawLine(run22d2b,y1+0.015,run22d2b,y2);
  l->DrawLine(run22d2e,y1+0.015,run22d2e,y2);
  l->DrawLine(run22d3b,y1+0.015,run22d3b,y2);
  l->SetLineStyle(kSolid);
  l->DrawLine(run22d3e,y1,run22d3e,y2);
  t->DrawLatex(run22d1b+300,0.925,"22D");
  double run22e1b(359022), run22e1e(360331);
  l->DrawLine(run22e1b,y1,run22e1b,y2);
  l->DrawLine(run22e1e,y1,run22e1e,y2);
  t->DrawLatex(run22e1b+150,0.925,"22E");
  double run22f1b(360332), run22f1e(362180);
  l->DrawLine(run22f1b,y1,run22f1b,y2);
  l->DrawLine(run22f1e,y1,run22f1e,y2);  
  t->DrawLatex(run22f1b+450,0.925,"22F");
  double run22g1b(362350), run22g1e(362760);
  l->DrawLine(run22g1b,y1,run22g1b,y2);
  l->DrawLine(run22g1e,y1,run22g1e,y2);  
  t->DrawLatex(run22g1b+50,0.925,"G");
  //
  double run23c1b(367080), run23c1e(367515);
  double run23c2b(367516), run23c2e(367620);
  double run23c3b(367621), run23c3e(367763);
  double run23c4b(367765), run23c4e(369802);
  l->DrawLine(run23c1b,y1,run23c1b,y2);
  l->SetLineStyle(kDashed);
  l->DrawLine(run23c1e,y1+0.015,run23c1e,y2);
  l->DrawLine(run23c2b,y1+0.015,run23c2b,y2);
  l->DrawLine(run23c2e,y1+0.015,run23c2e,y2);
  l->DrawLine(run23c3b,y1+0.015,run23c3b,y2);
  l->SetLineStyle(kSolid);
  l->DrawLine(run23c3e,y1+0.015,run23c3e,y2);
  l->DrawLine(run23c4b,y1+0.015,run23c4b,y2);
  l->DrawLine(run23c4e,y1,run23c4e,y2);
  t->DrawLatex(run23c1b+250,0.925,"23C");
  t->DrawLatex(run23c4b+250,0.935,"v4");
  double run23d1b(369803), run23d1e(370602);
  double run23d2b(370603), run23d2e(372415);
  l->DrawLine(run23d1b,y1,run23d1b,y2);
  l->SetLineStyle(kDashed);
  l->DrawLine(run23d1e,y1+0.015,run23d1e,y2);
  l->DrawLine(run23d2b,y1+0.015,run23d2b,y2);
  l->SetLineStyle(kSolid);
  l->DrawLine(run23d2e,y1,run23d2e,y2);
  t->DrawLatex(run23d1b+250,0.925,"23D");

  l->SetLineWidth(2);
  l->SetLineColor(kMagenta+1);
  double run22ae(354640); // HB "ieta flattening" (2022A)
  //l->DrawLine(run22ae,y1+0.025,run22ae,y2-0.025);
  double run22f(362102); // HB realignment (2022F)
  l->DrawLine(run22f,y1+0.025,run22f,y2-0.025);
  l->SetLineColor(kOrange+1);
  double run22ab(352319); // (start of era A) no HB corrections (respCorr = 1.0)
  //l->DrawLine(run22ab,y1+0.025,run22ab,y2-0.025);
  double run22fb(360329); // 360329 (start of era F) big HB corrections (~ +17%)
  l->DrawLine(run22fb,y1+0.025,run22fb,y2-0.025);

  l->SetLineColor(kRed+1);
  double runHBG(367765); // response correction update using 2022G data, deployed to prompt only
  l->DrawLine(runHBG,y1+0.025,runHBG,y2-0.025);
  double runHB(368775); // HB realignment based on TDC timing
  //also: 368775 compensating response correction for the HB time alignment
  l->DrawLine(runHB,y1+0.025,runHB,y2-0.025);
  
  tdrDraw(pr30b,"Pz",kOpenSquare,kBlue,kSolid); pr30b->SetMarkerSize(0.7);
  tdrDraw(pr30m,"Pz",kFullSquare,kBlue,kSolid); pr30m->SetMarkerSize(0.6);
  tdrDraw(pr110b,"Pz",kOpenCircle,kRed,kSolid); pr110b->SetMarkerSize(0.7);
  tdrDraw(pr110m,"Pz",kFullCircle,kRed,kSolid); pr110m->SetMarkerSize(0.6);


  // Add legend
  c1->cd(1);
  TLegend *leg = tdrLeg(0.54,0.68,0.64,0.88);
  leg->AddEntry(pr110m,"MPF 110EB","PLE");
  leg->AddEntry(pr110b,"BAL 110EB","PLE");
  leg->AddEntry(pr30m,"MPF 30EB","PLE");
  leg->AddEntry(pr30b,"BAL 30EB","PLE");

  c1->SaveAs("pdf/drawResponseVsRun.pdf");
  
} // drawResponseVsRun
