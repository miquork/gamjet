// Purpose: draw L2Res plots for gamma+jet to compare (combine) with dijet
#include "TFile.h"
#include "TProfile2D.h"

#include "../tdrstyle_mod22.C"

// v29->v31: L2L3Res_V1
TCanvas *_cb(0); // batch run canvas
TLegend *_leg(0), *_legz(0); // batch run legend
void drawL2Res(string era="2022E", string version="v31", bool closure=false,
	       string eraz="", string versionz="v64") {

  setTDRStyle();
  TDirectory *curdir = gDirectory;

  const char *ce = era.c_str();
  const char *cv = version.c_str();
  const char *cc = (closure ? "closure" : "L2Res");

  // Recursive launch
  if (era=="batchRun") {
    
    TH1D *h = tdrHist("h","L2Res closure (R^{MC}/R^{data})",0.89+1e-4,1.18-1e-4,
		      "#eta_{jet}",0,5.2);
    lumi_136TeV = Form("Run3, %s",version.c_str());
    extraText = "Private";
    _cb = tdrCanvas("_cb",h,8,11,kSquare);
    _leg = tdrLeg(0.37,0.855,0.57,0.90);
    _leg->SetHeader(" #gamma + jet");
    _legz = tdrLeg(0.33,0.855,0.53,0.90);
    _legz->SetHeader("Z");
    
    drawL2Res("2022CD","v31",true, "2022CD","v64");
    drawL2Res("2022E","v31",true,  "2022E","v64");
    drawL2Res("2022FG","v31",true, "2022FG","v64");
    
    drawL2Res("2023Cv123","v31",true, "2023C123","v64");
    drawL2Res("2023Cv4","v31",true,   "2023C4","v64");
    drawL2Res("2023D","v31",true,     "2023D","v64");

    _cb->SaveAs(Form("pdf/drawL2Res/drawL2Res_%s_%s_%s.pdf",ce,cv,cc));
    return;
  }

  // Same for Z+jet
  const char *cez = eraz.c_str();
  const char *cvz = versionz.c_str();

  // Load gamma+jet files
  TFile *fd(0), *fm(0), *fz(0);
  fd = new TFile(Form("rootfiles/GamHistosFill_data_%s_%s.root",ce,cv),"READ");
  assert(fd && !fd->IsZombie());
  fm = new TFile(Form("rootfiles/GamHistosFill_mc_2022EEP8_%s.root",cv),"READ");
  assert(fm && !fm->IsZombie());
  
  if (eraz!="") {
    fz = new TFile(Form("../jecsys3/rootfiles/jme_bplusZ_%s_Zmm_sync_%s.root",cez,cvz),"READ");
    assert(fz && !fz->IsZombie());
  }

  curdir->cd();

  TProfile2D *p2d(0), *p2m(0), *p2r(0);
  p2d = (TProfile2D*)fd->Get("Gamjet2/p2m0"); assert(p2d); // MPF
  p2r = (TProfile2D*)fd->Get("Gamjet2/p2res"); assert(p2r); // 1/L2L3Res
  p2m = (TProfile2D*)fm->Get("Gamjet2/p2m0"); assert(p2m); // MPF
  
  TProfile2D *p2dz(0), *p2mz(0), *p2rz(0);
  if (fz) {
    p2dz = (TProfile2D*)fz->Get("data/l2res/p2m0"); assert(p2dz);
    p2rz = (TProfile2D*)fz->Get("data/l2res/p2res"); assert(p2rz);
    p2mz = (TProfile2D*)fz->Get("mc/l2res/p2m0"); assert(p2mz);
  }

  // Step 0: prepare canvases for drawing later
  lumi_136TeV = Form("Run3, %s, %s",ce,cv);
  extraText = "Private";
  TH1D *h1 = tdrHist(Form("h1_%s_%s",ce,cv),
		    "R^{data}/R^{MC}",0.94+1e-4,1.05-1e-4,
		    "p_{T,#gamma} (GeV)",10,4000);
  TCanvas *c1 = tdrCanvas(Form("c1_%s_%s",ce,cv),h1,8,11,kSquare);
  
  TLatex *tex = new TLatex();
  tex->SetNDC(); tex->SetTextSize(0.045);
  
  TLine *l = new TLine();
  l->SetLineStyle(kDashed);
  l->SetLineColor(kGray);
  l->DrawLine(10,1,4000,1);

  TH1D *h2 = tdrHist(Form("h2_%s_%s",ce,cv),
		    "R^{data}/R^{MC}",0.85+1e-4,1.25-1e-4,
		     "#eta_{jet}",0,5.2);
  TCanvas *c2 = tdrCanvas(Form("c2_%s_%s",ce,cv),h2,8,11,kSquare);
  c2->cd();
  l->DrawLine(10,1,4000,1);
  
  // Step 1: get barrel RDATA/RMC so we can divide it out for L2Res
  int i1 = p2d->GetXaxis()->FindBin(0.);
  int i2 = p2d->GetXaxis()->FindBin(+1.305-0.05);
  TProfile *pd13 = p2d->ProfileY(Form("pd13_%s_%s",ce,cv),i1,i2);
  TProfile *pr13 = p2r->ProfileY(Form("pr13_%s_%s",ce,cv),i1,i2);
  TProfile *pm13 = p2m->ProfileY(Form("pm13_%s_%s",ce,cv),i1,i2);

  TH1D *h13 = pd13->ProjectionX(Form("h13_%s_%s",ce,cv)); // Get data RMC (MPF)
  if (!closure) h13->Multiply(pr13); // Undo L2L3Res for data (unless closure)
  h13->Divide(pm13); // Divide by RMC

  // Repeat for Z+jet
  TH1D *h13z(0);
  if (fz) {
    int i1 = p2dz->GetXaxis()->FindBin(0.);
    int i2 = p2dz->GetXaxis()->FindBin(+1.305-0.05);
    TProfile *pd13 = p2dz->ProfileY(Form("pd13z_%s_%s",ce,cv),i1,i2);
    TProfile *pr13 = p2rz->ProfileY(Form("pr13z_%s_%s",ce,cv),i1,i2);
    TProfile *pm13 = p2mz->ProfileY(Form("pm13z_%s_%s",ce,cv),i1,i2);

    h13z = pd13->ProjectionX(Form("h13z_%s_%s",ce,cv));
    if (!closure) h13z->Multiply(pr13);
    h13z->Divide(pm13);
  }

  
  // Divide into >=21 boxes in ~4:3 ratio => 6*4
  TCanvas *ca = new TCanvas(Form("ca_%s_%s",ce,cv),"ca",1800,1200);
  ca->Divide(6,4,0,0);
  double npad = 6*4;

  TH1D *ha = tdrHist(Form("ha_%s_%s",ce,cv),
		     "R^{MC}/R^{data}",0.75,1.35,
		     "p_{T,#gamma} (GeV)",10,4000);

  TH1D *heta = p2d->ProjectionX(Form("heta_%s_%s",ce,cv)); heta->Clear();
  TH1D *hetaz(0);
  if (p2dz) {
    hetaz = p2dz->ProjectionX(Form("hetaz_%s_%s",ce,cv)); hetaz->Clear();
  }
  
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

    // Repeat for Z+jet
    TH1D *hiz(0);
    if (fz) {
      int ietaz = p2dz->GetXaxis()->FindBin(eta1); // in case different bins
      int j1 = ieta; int j2 = ieta;
      TProfile *pdi = p2dz->ProfileY(Form("pdiz_%s_%s_%d",cez,cvz,ieta),j1,j2);
      TProfile *pri = p2rz->ProfileY(Form("priz_%s_%s_%d",cez,cvz,ieta),j1,j2);
      TProfile *pmi = p2mz->ProfileY(Form("pmiz_%s_%s_%d",cez,cvz,ieta),j1,j2);
    
      hiz = pdi->ProjectionX(Form("hiz_%s_%s_%d",ce,cv,ieta));
      if (!closure) hiz->Multiply(pri);
      hiz->Divide(pmi);
    }
    
    // Step3: divide ieta by barrel to get L2Res (and L2JES=1/L2RES=RDATA/RMC)
    TH1D *hl2jes = (TH1D*)hi->Clone(Form("hl2jes_%s_%s_%d",cv,ce,ieta));
    hl2jes->Divide(h13);
    TH1D *hl2res = (TH1D*)h13->Clone(Form("hl2res_%s_%s_%d",cv,ce,ieta));
    hl2res->Divide(hi);

    TF1 *f0 = new TF1(Form("f0_%s_%s_%d",ce,cv,ieta),"[0]",15,2000);
    TF1 *f1 = new TF1(Form("f1_%s_%s_%d",ce,cv,ieta),"[0]+[1]*log(x)",15,2000);
    f1->SetParameters(1,0);
    hl2res->Fit(f0,"QRN");
    hl2res->Fit(f1,"QRN");
    heta->SetBinContent(ieta,f1->Eval(100.));
    heta->SetBinError(ieta,f0->GetParError(0));
    
    // Repeat for Z+jet
    TH1D *hl2jesz(0), *hl2resz(0);
    TF1 *f1z(0);
    if (fz) {
      hl2jesz = (TH1D*)hiz->Clone(Form("hl2jesz_%s_%s_%d",cvz,cez,ieta));
      hl2jesz->Divide(h13z);
      hl2resz = (TH1D*)h13z->Clone(Form("hl2resz_%s_%s_%d",cvz,cez,ieta));
      hl2resz->Divide(hiz);

      TF1 *f0z = new TF1(Form("f0z_%s_%s_%d",ce,cv,ieta),"[0]",15,400);
      f1z = new TF1(Form("f1z_%s_%s_%d",ce,cv,ieta),"[0]+[1]*log(x)",15,400);
      f1z->SetParameters(1,0);
      hl2resz->Fit(f0z,"QRN");
      hl2resz->Fit(f1z,"QRN");

      int ietaz = p2dz->GetXaxis()->FindBin(eta1); // in case different bins
      hetaz->SetBinContent(ietaz,f1z->Eval(100.));
      hetaz->SetBinError(ietaz,f0z->GetParError(0));
    }
    
    // Step4: plotting time!
    if (ieta <= npad) {
      ca->cd(ieta);
      gPad->SetLogx();
      ha->Draw();
      l->DrawLine(10,1,4000,1);

      if (hl2resz) tdrDraw(hl2resz,"Pz",kFullSquare,kRed);
      tdrDraw(hl2res,"Pz",kFullCircle,kBlue);

      if (f1z) {
	f1z->SetLineColor(kRed);
	f1z->Draw("SAME");
      }
      f1->SetLineColor(kBlue);
      f1->Draw("SAME");
      
      tex->SetTextSize(0.045*2.);
      tex->DrawLatex(0.35,0.85,Form("%1.3f<|#eta|<%1.3f",eta1,eta2));
      tex->SetTextSize(0.045);
    }
    
    // Draw barrel and one example bin
    if (eta>0 && eta<0.261) {

      c1->cd();
      gPad->SetLogx();

      if (h13z) tdrDraw(h13z,"Pz",kFullSquare,kRed);
      tdrDraw(h13,"Pz",kFullCircle,kBlue);

      if (hiz) tdrDraw(hiz,"Pz",kOpenSquare,kRed-9);
      tdrDraw(hi,"Pz",kOpenCircle,kBlue-9);

      if (hl2resz) tdrDraw(hl2resz,"Pz",kOpenSquare,kRed);
      tdrDraw(hl2res,"Pz",kOpenCircle,kBlue);
    }

  } // for i(eta)

  c2->cd();
  if (hetaz) tdrDraw(hetaz,"Pz",kFullSquare,kRed);
  tdrDraw(heta,"Pz",kFullCircle,kBlue);

  c1->SaveAs(Form("pdf/drawL2Res/drawL3Res_%s_%s_%s.pdf",ce,cv,cc));
  c2->SaveAs(Form("pdf/drawL2Res/drawL2Res_%s_%s_%s.pdf",ce,cv,cc));
  ca->SaveAs(Form("pdf/drawL2Res/drawL2ResBins_%s_%s_%s.pdf",ce,cv,cc));

  if (_cb) {

    map<string,int> mmarker;
    mmarker["2022CD"] = kFullSquare;
    mmarker["2022E"] = kOpenSquare;
    mmarker["2022FG"] = kFullTriangleUp;
    mmarker["2023Cv123"] = kFullDiamond;
    mmarker["2023Cv4"] = kFullTriangleDown;
    mmarker["2023D"] = kOpenTriangleDown;
    mmarker["Run3"] = kFullSquare;
  
    map<string,int> mcolor;
    mcolor["2022CD"] = kGreen+2;
    mcolor["2022E"] = kCyan+2;
    mcolor["2022FG"] = kRed;
    mcolor["2023Cv123"] = kOrange+1;
    mcolor["2023Cv4"] = kBlue;
    mcolor["2023D"] = kMagenta;
    mcolor["Run3"] = kBlack;
    
    _cb->cd();
    tdrDraw(heta,"Pz",mmarker[era],mcolor[era]);
    _leg->SetY1NDC(_leg->GetY1NDC()-0.045);
    _leg->AddEntry(heta,ce,"PLE");

    if (hetaz) {
      tdrDraw(hetaz,"PLz",mmarker[era],mcolor[era]-9);
      hetaz->SetMarkerSize(0.5);
      _legz->SetY1NDC(_legz->GetY1NDC()-0.045);
      _legz->AddEntry(hetaz," ","PLE");

    }

  }
  
} // drawL2Res
