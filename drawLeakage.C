// Purpose: draw photon leakage distribution to estimate appropriate cuts
#include "TFile.h"
#include "TH2D.h"

#include "tdrstyle_mod15.C"

void drawLeakage() {
  
  setTDRStyle();
  TDirectory *curdir = gDirectory;

  TFile *fd = new TFile("files/GamHistosFill_data_Run2_v11.root","READ");
  //TFile *fd = new TFile("files/GamHistosFill_data_Run2_v16.root","READ");
  assert(fd && !fd->IsZombie());
  TFile *fm = new TFile("files/GamHistosFill_mc_Run2P8_v11.root","READ");
  //TFile *fm = new TFile("files/GamHistosFill_mc_Run2P8_v16.root","READ");
  assert(fm && !fm->IsZombie());

  curdir->cd();

  TH2D *h2d = (TH2D*)fd->Get("control/h2phoj"); assert(h2d);
  TH2D *h2m = (TH2D*)fm->Get("control/h2phoj"); assert(h2m);
  //TH2D *h2d = (TH2D*)fd->Get("control/h2phoj0"); assert(h2d);
  //TH2D *h2m = (TH2D*)fm->Get("control/h2phoj0"); assert(h2m);

  for (int i = 1; i != h2d->GetNbinsX()+1; ++i) {

    double nd = h2d->Integral(i,i,-1,-1);
    double nm = h2m->Integral(i,i,-1,-1);

    for (int j = 1; j != h2d->GetNbinsY()+1; ++j) {
      if (nd>0) h2d->SetBinContent(i, j, h2d->GetBinContent(i, j) / nd);
      if (nm>0) h2m->SetBinContent(i, j, h2m->GetBinContent(i, j) / nm);
    } // for j
  } // for i

  TCanvas *c1d = new TCanvas("c1d","c1d",600,600);
  gPad->SetLogx();
  gPad->SetLogz();
  gPad->SetRightMargin(0.10);
  h2d->RebinY(4);
  h2d->Draw("COLZ");

  TCanvas *c1m = new TCanvas("c1m","c1m",600,600);
  gPad->SetLogx();
  gPad->SetLogz();
  gPad->SetRightMargin(0.10);
  h2m->RebinY(4);
  h2m->Draw("COLZ");

  TCanvas *c1r = new TCanvas("c1r","c1r",600,600);
  gPad->SetLogx();
  //gPad->SetLogz();
  gPad->SetRightMargin(0.10);
  TH2D *h2r = (TH2D*)h2d->Clone("h2r");
  TH2D *h2m2 = (TH2D*)h2m->Clone("h2m");
  //h2r->RebinY(4);
  //h2m2->RebinY(4);
  h2r->Divide(h2m2);
  h2r->GetZaxis()->SetRangeUser(0.0,2.0);
  h2r->Draw("COLZ");

}
