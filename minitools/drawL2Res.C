// Purpose: draw L2Res plots for gamma+jet to compare (combine) with dijet
#include "TFile.h"
#include "TProfile2D.h"

#include "../tdrstyle_mod22.C"

// v29->v31: L2L3Res_V1
void drawL2Res(string era = "2022E", string version = "v31", bool closure = false) {
  
  // Recursive launch
  if (era=="batchRun") {
    drawL2Res("2022CD","v31",true);
    drawL2Res("2022E","v31",true);
    drawL2Res("2022FG","v31",true);
    
    drawL2Res("2023Cv123","v31",true);
    drawL2Res("2023Cv4","v31",true);
    drawL2Res("2023D","v31",true);
    return;
  }
  
  const char *ce = era.c_str();
  const char *cv = version.c_str();
  const char *cc = (closure ? "closure" : "L2Res");
  
  setTDRStyle();
  TDirectory *curdir = gDirectory;

  //TFile *fd = new TFile("files/GamHistosFill_data_2022F_v23.root","READ");
  //TFile *fd = new TFile("files/GamHistosFill_data_2022E_v23.root","READ");
  TFile *fd = new TFile(Form("rootfiles/GamHistosFill_data_%s_%s.root",ce,cv),"READ");
  assert(fd && !fd->IsZombie());
  //TFile *fm = new TFile("files/GamHistosFill_mc_2022EEP8_v23.root","READ");
  TFile *fm = new TFile(Form("rootfiles/GamHistosFill_mc_2022EEP8_%s.root",cv),"READ");
  assert(fm && !fm->IsZombie());
  curdir->cd();

  TProfile2D *p2d(0), *p2m(0), *p2r(0);
  p2d = (TProfile2D*)fd->Get("Gamjet2/p2m0"); assert(p2d); // MPF
  p2r = (TProfile2D*)fd->Get("Gamjet2/p2res"); assert(p2r); // 1/L2L3Res
  p2m = (TProfile2D*)fm->Get("Gamjet2/p2m0"); assert(p2m); // MPF

  // Step 0: prepare canvases for drawing later
  TCanvas *c1 = new TCanvas("c1","c1",600,400);
  TH1D *h1 = new TH1D(Form("h1_%s_%s",ce,cv),
		      ";p_{T,#gamma} (GeV);R^{data}/R^{MC}",100,10,4000);
  h1->SetMinimum(0.95+1e-4); h1->SetMaximum(1.05-1e-4);
  h1->Draw();
  gPad->SetLogx();
  
  TCanvas *c2 = new TCanvas(Form("c2_%s_%s",ce,cv),"c2",600,400);
  TH1D *h = new TH1D(Form("h_%s_%s",ce,cv),
		     ";p_{T,#gamma} (GeV);R^{MC}/R^{data}",100,10,4000);
  h->SetMinimum(0.95+1e-4); h->SetMaximum(1.05-1e-4);
  h->Draw();
  gPad->SetLogx();

  TLatex *tex = new TLatex();
  tex->SetNDC(); tex->SetTextSize(0.045);
  
  TLine *l = new TLine();
  l->SetLineStyle(kDashed);
  l->SetLineColor(kGray);
  l->DrawLine(10,1,4000,1);
  
  // Step 1: get barrel RDATA/RMC so we can divide it out for L2Res
  //int i1 = p2d->GetXaxis()->FindBin(-1.305+0.05);
  //int i1 = p2d->GetXaxis()->FindBin(0+0.05);
  //int i1 = p2d->GetXaxis()->FindBin(0+0.05)+1; // exclude eta=0
  int i1 = p2d->GetXaxis()->FindBin(0.); // include eta=0
  int i2 = p2d->GetXaxis()->FindBin(+1.305-0.05);
  TProfile *pd13 = p2d->ProfileY(Form("pd13_%s_%s",ce,cv),i1,i2);
  TProfile *pr13 = p2r->ProfileY(Form("pr13_%s_%s",ce,cv),i1,i2);
  TProfile *pm13 = p2m->ProfileY(Form("pm13_%s_%s",ce,cv),i1,i2);

  TH1D *h13 = pd13->ProjectionX(Form("h13_%s_%s",ce,cv)); // Get data RMC (MPF)
  if (!closure) h13->Multiply(pr13); // Undo L2L3Res for data (unless closure)
  h13->Divide(pm13); // Divide by RMC

  // Divide into >=21 boxes in ~4:3 ratio => 6*4
  TCanvas *ca = new TCanvas(Form("ca_%s_%s",ce,cv),"ca",1800,1200);
  ca->Divide(6,4,0,0);
  double npad = 6*4;

  TH1D *ha = new TH1D(Form("ha_%s_%s",ce,cv),
		      ";p_{T,#gamma} (GeV);R^{MC}/R^{data}",100,10,4000);
  ha->SetMinimum(0.85+1e-4); ha->SetMaximum(1.45-1e-4);
  
  // Step2: get RDATA/RMC for ieta of interest (loop over this later)
  // Need to project plus and minus sides separately, not easy to add directly
  // Duh, only have |eta| bins
  for (int ieta = 1; ieta != p2d->GetNbinsX()+1; ++ieta) {
    
    //int j1 = p2d->GetXaxis()->FindBin(0.261+0.05);
    //int j2 = p2d->GetXaxis()->FindBin(0.522-0.05);
    int j1 = ieta; int j2 = ieta;
    double eta1 = p2d->GetXaxis()->GetBinLowEdge(j1);
    double eta2 = p2d->GetXaxis()->GetBinLowEdge(j2+1);
    double eta = 0.5*(eta1+eta2);
    TProfile *pdi = p2d->ProfileY(Form("pdi_%s_%s_%d",ce,cv,ieta),j1,j2);
    TProfile *pri = p2r->ProfileY(Form("pri_%s_%s_%d",ce,cv,ieta),j1,j2);
    TProfile *pmi = p2m->ProfileY(Form("pmi_%s_%s_%d",ce,cv,ieta),j1,j2);
    
    TH1D *hi = pdi->ProjectionX(Form("hi_%s_%s_%d",ce,cv,ieta)); // Data RMC
    if (!closure) hi->Multiply(pri); // Undo L2L3Res for data (unless closure)
    hi->Divide(pmi); // Divide by RMC
    
    
    // Step3: divide ieta by barrel to get L2Res (and L2JES=1/L2RES=RDATA/RMC)
    TH1D *hl2jes = (TH1D*)hi->Clone(Form("hl2jes_%s_%s_%d",cv,ce,ieta));
    hl2jes->Divide(h13);
    TH1D *hl2res = (TH1D*)h13->Clone(Form("hl2res_%s_%s_%d",cv,ce,ieta));
    hl2res->Divide(hi);

    // Step4: plotting time!
    if (ieta <= npad) {
      ca->cd(ieta);
      gPad->SetLogx();
      ha->Draw();
      l->DrawLine(10,1,4000,1);

      tdrDraw(hl2res,"Pz",kFullCircle,kBlack);

      tex->SetTextSize(0.045*2.);
      tex->DrawLatex(0.40,0.85,Form("%1.2f<|#eta|<%1.3f",eta1,eta2));
      tex->SetTextSize(0.045);
    }
    

    //if (eta>0.261 && eta<0.522) {
    if (eta>0 && eta<0.261) {

      c1->cd();
    
      h13->Draw("SAME"); h13->SetMarkerStyle(kFullCircle);
      hi->Draw("SAME"); hi->SetMarkerStyle(kOpenCircle);
      hl2jes->Draw("SAME"); hl2jes->SetMarkerStyle(kFullDiamond);
      hl2res->Draw("SAME"); hl2res->SetMarkerStyle(kOpenDiamond);
      hl2res->SetMarkerColor(kRed); hl2res->SetLineColor(kRed);
      gPad->SetLogx();
    }

    //if (eta>0.261 && eta<0.522) {
    if (eta<1.305) {
      
      c2->cd();

      //hl2res->SetMarkerStyle(kFullCircle);
      hl2res->SetMarkerStyle(kOpenCircle);
      hl2res->SetMarkerColor(kGray+2);
      hl2res->SetLineColor(kGray+2);
      hl2res->SetMarkerSize(1.0*max(1.-0.1*(ieta-1),0.1));
      hl2res->Draw("SAMEPz");
      //hl2res->GetYaxis()->SetRangeUser(0.95+1e-4,1.05-1e-4);
      //hl2res->GetXaxis()->SetRangeUser(10.,4000.);

      if (ieta==1) {
	hl2res->SetMarkerColor(kRed);
	hl2res->SetLineColor(kRed);
      }
      if (ieta==2) {
	hl2res->SetMarkerColor(kGreen+2);
	hl2res->SetLineColor(kGreen+2);
      }
      
      hl2jes->SetMarkerStyle(kOpenDiamond);
      hl2jes->SetMarkerColor(kGray+2);
      hl2jes->SetLineColor(kGray+2);
      hl2jes->SetMarkerSize(1.0*max(1.-0.1*(ieta-1),0.1));
      //hl2jes->Draw("SAMEPz");
    
      /*
	hl2resp->SetMarkerStyle(kOpenDiamond);
	hl2resp->SetMarkerColor(kRed);
	hl2resp->SetLineColor(kRed);
	hl2resp->Draw("SAMEPz");
	
	hl2resm->SetMarkerStyle(kOpenDiamond);
	hl2resm->SetMarkerColor(kBlue);
	hl2resm->SetLineColor(kBlue);
	hl2resm->Draw("SAMEPz");
      */
    } // barrel
  } // for i(eta)

  ca->SaveAs(Form("pdf/drawL2Res/drawL2Res_%s_%s_%s.pdf",ce,cv,cc));
} // drawL2Res
