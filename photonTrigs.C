// Purpose: draw photon triggers and check their efficiency
#include "TFile.h"
#include "TH1D.h"

#include "tdrstyle_mod15.C"

// Draw active trigger range and extended trigger range
void trigDraw(TH1D *h, int color, int style, double xmin, double xmax,
	      double xmin2, double xmax2) {

  TH1D *h1 = (TH1D*)h->Clone(Form("%s_1",h->GetName()));
  TH1D *h2 = (TH1D*)h->Clone(Form("%s_2",h->GetName()));
  for (int i = 1; i != h1->GetNbinsX()+1; ++i) {
    double x = h1->GetBinCenter(i);
    if (x<xmin || x>xmax) {
      h1->SetBinContent(i,0);
      h1->SetBinError(i,0);
    }
    if (x<xmin2 || x>xmax2) {
      h2->SetBinContent(i,0);
      h2->SetBinError(i,0);
    }
  } // for i

  tdrDraw(h2,"HIST][",kNone,kBlack,kSolid,color,kNone,0);
  h2->SetFillColorAlpha(color,0.7);
  tdrDraw(h1,"HIST",kNone,kBlack,kSolid,color,style,0);
  h1->SetFillColorAlpha(color,0.7);
} // trigDraw


void photonTrigs(string year = "2018") {

  setTDRStyle();
  TDirectory *curdir = gDirectory;

  TFile *f0(0), *f1(0);

  if (year=="2023Cv4") {
    f1 = new TFile("files/GamHistosFill_data_2023Cv4_v21.root","READ");
    f0 = f1;
    lumi_13TeV = "2023Cv4";
  }
  if (year=="2023Cv123") {
    f1 = new TFile("files/GamHistosFill_data_2023Cv123_v21.root","READ");
    f0 = f1;
    lumi_13TeV = "2023Cv123";
  }
  if (year=="2022G") {
    f1 = new TFile("files/GamHistosFill_data_2022G_v21.root","READ"); f0 = f1;
    lumi_13TeV = "2022G";
  }
  if (year=="2022F") {
    f1 = new TFile("files/GamHistosFill_data_2022F_v21.root","READ"); f0 = f1;
    lumi_13TeV = "2022F";
  }
  if (year=="2022E") {
    f1 = new TFile("files/GamHistosFill_data_2022E_v21.root","READ"); f0 = f1;
    lumi_13TeV = "2022E";
  }
  if (year=="2022D") {
    f1 = new TFile("files/GamHistosFill_data_2022D_v21.root","READ"); f0 = f1;
    lumi_13TeV = "2022D";
  }
  if (year=="2022C") {
    f1 = new TFile("files/GamHistosFill_data_2022C_v21.root","READ"); f0 = f1;
    lumi_13TeV = "2022C";
  }
  if (year=="2018") { // 2018
    f0 = new TFile("files/GamHistosFill_data_v1.root","READ");
    f1 = new TFile("files/GamHistosFill_data_2018ABCD_v16.root","READ");//9
    lumi_13TeV = "2018ABCD";
  }
  if (year=="2017") { // 2017
    f0 = new TFile("files/GamHistosFill_data_2017BCDEF_v16.root","READ");//10
    f1 = f0;
    lumi_13TeV = "2017BCDEF";
  }
  if (year=="2016GH") { // 2016GH
    f0 = new TFile("files/GamHistosFill_data_2016FGH_v16.root","READ");//10
    f1 = f0;
    lumi_13TeV = "2016GH";
  }
  if (year=="2016BCDEF") { // 2016
    f0 = new TFile("files/GamHistosFill_data_2016BCDEF_v16.root","READ");//10
    f1 = f0;
    lumi_13TeV = "2016BCDEF";
  }
  assert(f0 && !f0->IsZombie());
  assert(f1 && !f1->IsZombie());

  f1->cd("control"); 
  TDirectory *d1 = gDirectory;

  curdir->cd();

  TH1D *h = tdrHist("h","N_{events}",1e0,2e9,"p_{T,#gamma} (GeV)",15,1000);
  TCanvas *c1 = tdrCanvas("c1",h,4,11,kSquare);
  gPad->SetLogx();
  gPad->SetLogy();

  TH1D *had = (TH1D*)f0->Get("hgam0_data"); assert(had);
  TH1D *htd = (TH1D*)d1->Get("hgamtrig_data"); assert(htd);

  TH1D *h200 = (TH1D*)d1->Get("hgam200"); assert(h200);
  TH1D *h175 = (TH1D*)d1->Get("hgam175"); assert(h175);

  TH1D *h100h = (TH1D*)d1->Get("hgam100h"); assert(h100h); // run3
  
  TH1D *h120t = (TH1D*)d1->Get("hgam120t"); assert(h120t);
  TH1D *h110t = (TH1D*)d1->Get("hgam110t"); assert(h110t);
  TH1D *h100t = (TH1D*)d1->Get("hgam100t"); assert(h100t);
  TH1D *h30t = (TH1D*)d1->Get("hgam30t"); assert(h30t); // run3

  TH1D *h165m = (TH1D*)d1->Get("hgam165m"); assert(h165m);
  TH1D *h120m = (TH1D*)d1->Get("hgam120m"); assert(h120m);
  TH1D *h90m = (TH1D*)d1->Get("hgam90m"); assert(h90m);
  TH1D *h75m = (TH1D*)d1->Get("hgam75m"); assert(h75m);
  TH1D *h50m = (TH1D*)d1->Get("hgam50m"); assert(h50m);

  TH1D *h36m = (TH1D*)d1->Get("hgam36m"); assert(h36m);
  TH1D *h30m = (TH1D*)d1->Get("hgam30m"); assert(h30m);
  TH1D *h22m = (TH1D*)d1->Get("hgam22m"); assert(h22m);

  TH1D *h30l = (TH1D*)d1->Get("hgam30l"); assert(h30l);
  TH1D *h20l = (TH1D*)d1->Get("hgam20l"); assert(h20l);

  TH1D *h33 = (TH1D*)d1->Get("hgam33"); assert(h33);
  

  tdrDraw(had,"HIST",kNone,kBlack,kSolid,kGray+2,1001,kGray);
  tdrDraw(htd,"HIST",kNone,kBlack,kSolid,kRed+2,1001,kRed-9);

  if (year=="2016BCDEF" || year=="2016GH" || year=="2017" || year=="2018") {
    
    //tdrDraw(h200,"HIST",kNone,kBlack,kSolid,kBlack,kNone,0);
    trigDraw(h33,kGray+2,3004,40,60,25,130);
    trigDraw(h20l,kOrange+2,3005,20,60,15,60);
    trigDraw(h30l,kOrange+3,3005,30,60,25,60);
    if (lumi_13TeV=="2016BCDEF" || lumi_13TeV=="2016GH") {
      trigDraw(h22m,kCyan+1,3005,20,35,15,60);
      trigDraw(h30m,kCyan+2,3005,35,40,15,60);
    trigDraw(h36m,kCyan+3,3005,40,60,15,60);
    }
    trigDraw(h50m,kGray+3,3005,60,85,45,85);
    trigDraw(h75m,kGray+3,3005,85,105,70,105);
    trigDraw(h90m,kGray+2,3004,105,130,85,130);
    trigDraw(h120m,kGray+2,3004,130,175,115,175);
    trigDraw(h165m,kGray+2,3004,175,230,160,230);
    trigDraw(h100t,kMagenta+2,3005,105,130,95,130);
    trigDraw(h110t,kMagenta+2,3005,130,230,105,230);
    trigDraw(h120t,kMagenta+2,3005,175,230,115,230);
    if (lumi_13TeV=="2016BCDEF")
      trigDraw(h175,kRed+2,3005,230,400,165,500);
    else
      trigDraw(h200,kRed+2,3005,230,400,180,500);
  }

  if (year=="2022C" || year=="2022D" ||
      year=="2022E" || year=="2022F" || year=="2022G" ||
      year=="2023Cv123" || year=="2023Cv4") {
    // 22-23 binning
    //double vx[] = {15, 20, 25, 30, 35, 40, 50, 60, 75, 90, 110, 130, 175, 230,
    // 		     300, 400, 500, 600, 700, 850, 1000, 1200, 1450, 1750};
  
    trigDraw(h20l,kOrange+2,3002,20,30,15,35);
    trigDraw(h30t,kMagenta+3,3005,30,50,25,55);
    trigDraw(h50m,kGray+3,3004,50,75,45,130);//85);
    trigDraw(h75m,kGray+2,3004,75,90,70,95);
    trigDraw(h90m,kGray+1,3004,90,110,85,115);
    //trigDraw(h120m,kGray+2,3004,130,175,115,175);
    //trigDraw(h165m,kGray+2,3004,175,230,160,230);
    //trigDraw(h100t,kMagenta+2,3005,105,130,95,130);
    //trigDraw(h100h,kMagenta+1,3005,110,130,95,130);
    trigDraw(h110t,kMagenta+2,3005,110,230,105,235);
    //trigDraw(h175,kRed+2,3005,230,400,165,500);
    trigDraw(h200,kRed+2,3002,230,400,195,500);

  }
    
  h200->SetLineColor(kRed);
  h200->SetFillColor(kRed);
  h200->SetFillStyle(3002);//3005);

  h175->SetLineColor(kRed);
  h175->SetFillColor(kRed);
  h175->SetFillStyle(3005);

  h110t->SetLineColor(kMagenta+2);
  h110t->SetFillColor(kMagenta+2);
  h110t->SetFillStyle(3005);

  h30t->SetLineColor(kMagenta+3);
  h30t->SetFillColor(kMagenta+3);
  h30t->SetFillStyle(3005);

  h90m->SetLineColor(kGray+1);
  h90m->SetFillColor(kGray+1);
  h90m->SetFillStyle(3004);

  h75m->SetLineColor(kGray+2);
  h75m->SetFillColor(kGray+2);
  h75m->SetFillStyle(3004);//3005);

  h50m->SetLineColor(kGray+3);
  h50m->SetFillColor(kGray+3);
  h50m->SetFillStyle(3004);//3005);

  h22m->SetLineColor(kCyan+1);
  h22m->SetFillColor(kCyan+1);
  h22m->SetFillStyle(3005);

  h20l->SetLineColor(kOrange+2);
  h20l->SetFillColor(kOrange+2);
  h20l->SetFillStyle(3002);//3005);

  h33->SetLineColor(kGray+2);
  h33->SetFillColor(kGray+2);
  h33->SetFillStyle(3004);


  TLegend *leg = tdrLeg(0.48,0.90-5*0.04,0.68,0.90);
  TLegend *leg2 = tdrLeg(0.62,0.70-2*0.04,0.82,0.70);
  TLegend *leg3 = tdrLeg(0.67,0.62-1*0.04,0.87,0.62);

  leg->AddEntry(had,"EGamma","F");
  leg->AddEntry(htd,"JEC #gamma + jet","F");
  if (year=="2016BCDEF" || year=="2016GH" || year=="2017" || year=="2018") {
    
    if (lumi_13TeV=="2016BCDEF" || lumi_13TeV=="2016GH")
      leg->AddEntry(h175,"HLT_Photon175","F");
    else
      leg->AddEntry(h200,"HLT_Photon200","F");
    leg->AddEntry(h110t,"*[100,110]_tight","F");
    leg->AddEntry(h90m,"*[90,120,165]_medium","F");
    
    if (lumi_13TeV=="2016BCDEF" || lumi_13TeV=="2016GH") {
      leg2->AddEntry(h50m,"*[50,75]_med.","F");
      leg2->AddEntry(h22m,"*[22,30,36]_m.","F");
    }
    else {
      leg2->AddEntry(h50m,"*[50,75]_med.","F");
      leg2->AddEntry(h20l,"*[20,30]_loose","F");
    }
    
    if (lumi_13TeV!="2016BCDEF" && lumi_13TeV!="2016GH")
      leg3->AddEntry(h33,"Photon33","F");
  }
  if (year=="2022C" || year=="2022D" ||
      year=="2022E" || year=="2022F" || year=="2022G" ||
      year=="2023Cv123" || year=="2023Cv4") {
    
    leg->AddEntry(h200,"Photon200","F");
    leg->AddEntry(h110t,"110_tight","F");
    leg->AddEntry(h90m,"90_medium","F");    
    //leg->AddEntry(h90m,"[90,120,165]_medium","F");    
    leg2->AddEntry(h50m,"[50,75]_med.","F");
    leg2->AddEntry(h30t,"30_tight","F");
    leg3->AddEntry(h20l,"20_loose","F");
  }
    
  gPad->RedrawAxis();

  //c1->SaveAs("pdf/photonTrigs.pdf");
  c1->SaveAs(Form("pdf/photonTrigs_%s.pdf",lumi_13TeV.Data()));
} // photonTrigs
