// Purpose: Draw estimate for photon+jet purity to combine with dijet scale
//          Approach 1: scale QCD cross section by photon fakerate
//          Approach 2: use gluon-jet fraction (with QGL SF) to infer purity
// NB: Another script to be written for dijet MPF,DB (HDM) wrt to photon MPP,DB
//     Pure photon+jet MC should be scaled by purity x response difference
#include "TFile.h"
#include "TProfile.h"
#include "TF1.h"

#include "tdrstyle_mod15.C"
#include "../jecsys2020/Flavor.h"

// Replace fakerate histogram by fit of the same over multiple years
bool replaceFakeByFit = true;
bool doRebin = false;
bool doPerGeV = true;

void drawEMJetScale(string era, string version);
void drawGluonFraction(string era, string version);
void drawGluonResponse(string era, string version);

// v27->v29
void drawPurityEstimates(string era="2022CD", string version="v29") {
  
  setTDRStyle();
  TDirectory *curdir = gDirectory;
  const char *cera = era.c_str();
  const char *cv = version.c_str();

  string eramc = "2022";//(era=="2022FG" ? "2022EE" : "2022");
  const char *ceramc = eramc.c_str();
  // Open files
  /*
  TFile *fg = new TFile("files/GamHistosFill_mc_2018P8_v19.root","READ");
  assert(fg && !fg->IsZombie());
  TFile *fq = new TFile("files/GamHistosFill_mc_2018QCD_v19d.root","READ");
  assert(fq && !fq->IsZombie());
  TFile *fd = new TFile("files/GamHistosFill_data_2018ABCD_v19.root","READ");
  assert(fd && !fd->IsZombie());
  */
  /*
  TFile *fg = new TFile("files/GamHistosFill_mc_2017P8_v19.root","READ");
  assert(fg && !fg->IsZombie());
  TFile *fq = new TFile("files/GamHistosFill_mc_2017QCD_v19d.root","READ");
  assert(fq && !fq->IsZombie());
  TFile *fd = new TFile("files/GamHistosFill_data_2017BCDEF_v19.root","READ");
  assert(fd && !fd->IsZombie());
  */
  TFile *fg = new TFile(Form("rootfiles/GamHistosFill_mc_%sP8_%s.root",
			     ceramc,cv),"READ");
  assert(fg && !fg->IsZombie());
  TFile *fq = new TFile(Form("rootfiles/GamHistosFill_mc_%sQCD_%s.root",
			     ceramc,cv),"READ");
  assert(fq && !fq->IsZombie());
  TFile *fd = new TFile(Form("rootfiles/GamHistosFill_data_%s_%s.root",
			     cera,cv),"READ");
  assert(fd && !fd->IsZombie());
  
  // Read in pT spectra with full event selection and for photon only
  TH1D *hrefbins = (TH1D*)fg->Get("control/hgam"); assert(hrefbins);
  const Double_t *xbins = hrefbins->GetXaxis()->GetXbins()->GetArray();
  const int nxbins = hrefbins->GetNbinsX();
  //TH1D *hg0 = (TH1D*)fg->Get("control/hgam"); assert(hg0);
  //TH1D *hg0 = (TH1D*)fg->Get("control/hgamtrig"); assert(hg0); // 2017
  TH1D *hg0 = (TH1D*)fg->Get("control/hgamtrg"); assert(hg0); // 2022
  if (doRebin) hg0 = (TH1D*)hg0->Rebin(nxbins,"hg0",xbins);
  if (doPerGeV) hg0->Scale(1,"width");
  //TH1D *hg = (TH1D*)fg->Get("resp_MPFchs_MC_a100_eta00_13_RawNEvents_data_vs_pt"); assert(hg); // 2017
  TH1D *hg = (TH1D*)fg->Get("resp_MPFchs_MC_a100_eta00_13_Xsec_data_vs_pt"); assert(hg); // 2022
  if (doPerGeV) hg->Scale(1,"width");
  //TH1D *hq0 = (TH1D*)fq->Get("control/hgam"); assert(hq0);
  //TH1D *hq0 = (TH1D*)fq->Get("control/hgamtrig"); assert(hq0); // 2017
  TH1D *hq0 = (TH1D*)fq->Get("control/hgamtrg"); assert(hq0); // 2022
  if (doRebin) hq0 = (TH1D*)hq0->Rebin(nxbins,"hq0",xbins);
  if (doPerGeV) hq0->Scale(1,"width");
  //TH1D *hq = (TH1D*)fq->Get("resp_MPFchs_MC_a100_eta00_13_RawNEvents_data_vs_pt"); assert(hq); // 2017
  TH1D *hq = (TH1D*)fq->Get("resp_MPFchs_MC_a100_eta00_13_Xsec_data_vs_pt"); assert(hq); // 2022
  if (doPerGeV) hq->Scale(1,"width");
  //TH1D *hdt0 = (TH1D*)fd->Get("control/hgam"); assert(hdt0);
  //TH1D *hdt0 = (TH1D*)fd->Get("control/hgamtrig"); assert(hdt0); // 2017
  TH1D *hdt0 = (TH1D*)fd->Get("control/hgamtrg"); assert(hdt0); // 2022
  if (doRebin) hdt0 = (TH1D*)hdt0->Rebin(nxbins,"hdt0",xbins);
  if (doPerGeV) hdt0->Scale(1,"width");
  TH1D *hdt = (TH1D*)fd->Get("resp_MPFchs_DATA_a100_eta00_13_RawNEvents_data_vs_pt"); assert(hdt);
  if (doPerGeV) hdt->Scale(1,"width");

  // Read in fake rates
  TProfile *pfq = (TProfile*)fq->Get("control/pfakeqcd2"); assert(pfq);
  TH1D *hfq0 = pfq->ProjectionX("hfq0");
  TH1D *hfq = pfq->ProjectionX("hfq");


  /*
  // Hand-crafted "fit" to 2016+2018 control/pfakeqcd2
  TF1 *f1p = new TF1("f1p","[0]+[1]*exp(-[2]*x)+[3]*x",15,3500);
  f1p->SetParameters(7e-5,0.008,0.1,4e-8);
  */
  // Hand-crafted "fit" to 2022QCD control/pfakeqcd2
  // Some odd spikes at 100 and 150 GeV mess up midrange
  TF1 *f1p0 = new TF1("f1p0","[0]+[1]/x+[2]/(x*x)+[3]/(x*x*x)",15,3500);
  f1p0->SetParameters(2.2e-5,0.8e-3,0.08,5.5);
  //
  if (replaceFakeByFit) {
    for (int i = 1; i != hfq->GetNbinsX()+1; ++i) {
      double pt = hfq->GetBinCenter(i);
      hfq->SetBinContent(i, f1p0->Eval(pt));
      hfq0->SetBinContent(i, f1p0->Eval(pt));
    }
  }
  
  curdir->cd();

  /*
  // 2017,2018
  // Cross sections from McM:
  // https://cms-pdmv.cern.ch/mcm/requests?page=0&shown=2148532224

  // dataset_name = GJet_Pt-15To6000_TuneCP5-Flat_13TeV_pythia8
  // member_of_campaign = RunIISummer19UL18GEN (NanoAODv2 -> GEN)
  // => xsec = 290100 pb, entries 20,000,000
  // => NanoAODv2 entries 19,441,000
  // => 2018P8 with 19,307,200 entries (few files missing?)
  double xsecg = 290100; double ng = 19307200; // 2018
  hg0->Scale(xsecg/ng); // 0.0150
  hg->Scale(xsecg/ng); // 0.0150
  // Photon+jet k-factor
  //hg0->Scale(1.1);
  //hg->Scale(1.1);

  // dataset_name = QCD_Pt-15to7000_TuneCP5_Flat_13TeV_pythia8
  // member_of_campaign = RunIISummer19UL18GEN (NanoAODv2 -> GEN)
  // => xsec = 1352000000 pb (1.35 mb), entries 20,000,000
  // => NanoAODv2 entries 19,942,200
  // => 2018QCD with 19942200 entries
  //double xsecq = 1352000000; double nq = 19942200; // 2018
  // => 2017QCD with 4648699 entries only (just a few files available)
  //double xsecq = 1352000000; double nq = 19883800; // 2017 (original)
  double xsecq = 1352000000; double nq = 4648699; // 2017 (hefaistos)
  hq0->Scale(xsecq/nq); // 67.8
  hq->Scale(xsecq/nq); // 67.8
  // Add x2 for both leading jets (now only one randomly as photon)
  hq0->Scale(2.);
  hq->Scale(2.);
  */
  // 2022: using MadGraph samples that are already normalized with genWeight
  
  // Enter integrated luminosities per pT bin by hand
  TH1D *hlum17 = (TH1D*)hdt->Clone("hlum17"); hlum17->Reset();
  for (int i = 1; i != hlum17->GetNbinsX()+1; ++i) {
    double pt = hlum17->GetBinCenter(i);
    if      (pt>=230) hlum17->SetBinContent(i, 41.477735690); // HLT_Photon200
    else if (pt>175)  hlum17->SetBinContent(i, 28.315865389); // HLT_Photon165..
    else if (pt>130)  hlum17->SetBinContent(i, 7.797325137);  // HLT_Photon120..
    else if (pt>105)  hlum17->SetBinContent(i, 2.640888480);  // HLT_Photon90..
    else if (pt>85)   hlum17->SetBinContent(i, 1.326146145);  // HLT_Photon75..
    else if (pt>60)   hlum17->SetBinContent(i, 0.307208422);  // HLT_Photon50..
    else if (pt>35)   hlum17->SetBinContent(i, 0.002159100);  //*HLT_Photon30..
    else if (pt>20)   hlum17->SetBinContent(i, 0.000447966);  // HLT_Photon20..
    hlum17->SetBinError(i, 0);
  }
  hlum17->Scale(1e3); // fb-1 -> pb-1

  TH1D *hlum22 = (TH1D*)hdt->Clone("hlum22"); hlum22->Reset();
  for (int i = 1; i != hlum22->GetNbinsX()+1; ++i) {
    double pt = hlum22->GetBinCenter(i);
    // https://indico.cern.ch/event/1339886/contributions/5640525/attachments/2741370/4768535/23-10-26_News_PPD.pdf?#page=10 (and 11)
    double lum22cde = 6.3+3.3+6.1; // 22CDE
    double lum22cd = 6.3+3.3; // 22CD
    double lum22e = 6.1; // 22E
    double lum22fg = 18.4+3.2; // 22FG
    // From Sami
    // 2023Cv123 7143 x2 pb-1
    // 2023Cv4 9918 x2 pb-1
    // 2023D1 2023D2  7854 x2 pb-1 1672 x2 pb-1
    double lum23c123 = 7.1; // PPD 23c: 17.0
    double lum23cv4 = 9.9;  // PPD 23c: 17.0
    double lum23d = 7.85+1.67;//9.5;
    double lum(0);
    if (era=="2022CD") lum = lum22cd;
    if (era=="2022E") lum = lum22e;
    if (era=="2022FG") lum = lum22fg;
    if (era=="2023Cv123") lum = lum23c123;
    if (era=="2023Cv4D") lum = lum23cv4+lum23d;
    // Rough numbers for 2022FG
    if      (pt>=230) hlum22->SetBinContent(i, lum); // HLT_Photon200
    else if (pt>110)  hlum22->SetBinContent(i, 0.94*lum); // HLT_Photon110EB*
    else if (pt>90)  hlum22->SetBinContent(i, 0.0278*lum);  // HLT_Photon90*
    else if (pt>75)   hlum22->SetBinContent(i, 0.0136*lum);  // HLT_Photon75*
    else if (pt>50)   hlum22->SetBinContent(i, 0.0031*lum);  // HLT_Photon50*
    else if (pt>30)   hlum22->SetBinContent(i, 0.0057*lum);  // HLT_Photon30EB*
    else if (pt>20)   hlum22->SetBinContent(i, 0.00003*lum);  // HLT_Photon20*
    hlum22->SetBinError(i, 0);
  }
  hlum22->Scale(1e3); // fb-1 -> pb-1

  // Data has 59.9 fb-1
  //double lumi = 59.9e3; // 2018
  //double lumi = 41.477735690e3; // 2017
  //hdt0->Scale(1./lumi);
  //hdt->Scale(1./lumi);

  //hdt0->Divide(hlum17); // 2017
  //hdt->Divide(hlum17); // 2017

  hdt0->Divide(hlum22); // 2022
  hdt->Divide(hlum22); // 2022

  /*
  // 2017,2018
  // Cross sections apparently without genWeight? Account for that as well
  // So basically divide by sum of weights instead of number of events
  double wq = 0.012730690; // files-qcd-15to7000.root
  hq0->Scale(1./wq);
  hq->Scale(1./wq);
  double wg = 0.0045190600; // files-pt-15to6000.root
  hg0->Scale(1./wg);
  hg->Scale(1./wg);
  */
  // Estimate fake photon cross section
  // NB: QCD=4660 times higher xsec, 1654 with x wg/wq
  // (alphas=0.1185/alpha=1/137)^2 = 263.6. Still x6 different? gg vs qg?
  hfq0->Multiply(hq0);
  hfq->Multiply(hq);

  
  TH1D *hr0 = (TH1D*)hq0->Clone("hr0");
  hr0->Divide(hg0);
  hr0->Scale(1e-4);
  TH1D *hr = (TH1D*)hq->Clone("hr");
  hr->Divide(hg);
  hr->Scale(1e-4);

  TH1D *hmc0 = (TH1D*)hg0->Clone("hmc0");
  hmc0->Add(hfq0);
  TH1D *hfr0 = (TH1D*)hfq0->Clone("hfr0");
  hfr0->Divide(hmc0);
  TH1D *hmc = (TH1D*)hg->Clone("hmc");
  hmc->Add(hfq);
  TH1D *hfr = (TH1D*)hfq->Clone("hfr");
  hfr->Divide(hmc);

  TH1 *hgr0 = (TH1D*)hg0->Clone("hg0");
  hgr0->Divide(hdt0);
  TH1 *hgr = (TH1D*)hg->Clone("hgr");
  hgr->Divide(hdt);

  TH1 *hmr0 = (TH1D*)hmc0->Clone("hmr0");
  hmr0->Divide(hdt0);
  TH1 *hmr = (TH1D*)hmc->Clone("hmr");
  hmr->Divide(hdt);

  TH1D *hu = tdrHist("hu","Cross section (pb/bin)",0.9e-5,1.1e11);//2e-7,2e8);
  if (doPerGeV) {
    hu->SetYTitle("Cross section pb/GeV");
    hu->GetYaxis()->SetRangeUser(1.1e-7,0.9e10);
  }
  //TH1D *hd = tdrHist("hd","Ratio (10^{4})",1e-1,5);
  TH1D *hd = tdrHist("hd","Ratio",0,2);
  //lumi_13TeV = "UL2018";
  lumi_13TeV = Form("%s, %s",cera,cv);//"2022CD";//"UL2017";
  TCanvas *c1 = tdrDiCanvas("c1",hu,hd,4,11);

  c1->cd(1);
  gPad->SetLogx();
  gPad->SetLogy();
  tdrDraw(hq0,"HIST][",kNone,kBlue,kSolid,-1,kNone);
  tdrDraw(hg0,"HIST][",kNone,kRed,kSolid,-1,kNone);
  tdrDraw(hq,"Pz",kOpenSquare,kBlue,kSolid,-1,kNone);
  tdrDraw(hg,"Pz",kOpenCircle,kRed,kSolid,-1,kNone);

  tdrDraw(hfq0,"HIST][",kNone,kGreen+2,kSolid,-1,kNone);
  tdrDraw(hfq,"Pz",kOpenSquare,kCyan+2,kSolid,-1,kNone);
  hfq->SetMarkerSize(0.7);

  tdrDraw(hmc0,"HIST][",kNone,kGray+1,kSolid,-1,kNone);
  tdrDraw(hmc,"Pz",kOpenDiamond,kBlack,kSolid,-1,kNone);
  tdrDraw(hdt0,"HIST][",kNone,kBlack,kSolid,-1,kNone);
  tdrDraw(hdt,"Pz",kFullDiamond,kBlack,kSolid,-1,kNone);

  TLegend *legu1 = tdrLeg(0.59,0.90-6*0.05,0.79,0.90);
  legu1->SetHeader("\"#gamma\"");
  legu1->AddEntry(hq0," ","L");
  legu1->AddEntry(hg0," ","L");
  legu1->AddEntry(hfq0," ","L");
  legu1->AddEntry(hmc0," ","L");
  legu1->AddEntry(hdt0," ","L");

  TLegend *legu2 = tdrLeg(0.65,0.90-6*0.05,0.85,0.90);
  legu2->SetHeader("\"#gamma + jet\"");
  legu2->AddEntry(hq,"QCD dijet","PLE");
  legu2->AddEntry(hg,"#gamma + jet","PLE");
  legu2->AddEntry(hfq,"QCD #times #epsilon_{#gamma}","PLE");
  legu2->AddEntry(hmc,"Mixed MC","PLE");
  legu2->AddEntry(hdt,"Data","PLE");


  c1->cd(2);
  gPad->SetLogx();

  TLine *l = new TLine();
  l->SetLineStyle(kSolid);//kDashed);
  l->DrawLine(15,1,3500,1);
  l->SetLineStyle(kDotted);
  l->DrawLine(15,1.2,3500,1.2);
  l->DrawLine(15,0.8,3500,0.8);

  //tdrDraw(hr0,"HIST",kNone,kBlue,kSolid,-1,kNone);
  //tdrDraw(hr,"Pz",kOpenSquare,kBlue,kSolid,-1,kNone);
  tdrDraw(hfr0,"HIST][",kNone,kGreen+2,kSolid,-1,kNone);
  tdrDraw(hfr,"Pz",kOpenDiamond,kCyan+2,kSolid,-1,kNone);
  tdrDraw(hgr0,"HIST][",kNone,kRed,kSolid,-1,kNone);
  tdrDraw(hgr,"Pz",kOpenCircle,kRed,kSolid,-1,kNone);
  tdrDraw(hmr0,"HIST][",kNone,kBlack,kSolid,-1,kNone);
  tdrDraw(hmr,"Pz",kOpenDiamond,kBlack,kSolid,-1,kNone);

  double k = 1.5;
  TLegend *legd = tdrLeg(0.18,0.90-2*0.05*k,0.18+3*0.25,0.90);
  legd->SetTextSize(0.045*k);
  legd->SetNColumns(3);
  legd->AddEntry(hfr,"(QCD#times#epsilon_{#gamma}) / Mix","PLE");
  legd->AddEntry(hgr,"(#gamma+jet) / Data","PLE");
  legd->AddEntry(hmr,"Mix / Data","PLE");
  legd->AddEntry(hfr0,"#gamma sel. only","L");
  legd->AddEntry(hgr0,"#gamma sel. only","L");
  legd->AddEntry(hmr0,"#gamma sel. only","L");

  // Redefine fit function from f1p0
  //TF1 *f1p = new TF1("f1p","min([0]+[1]/x+[2]/(x*x)+[3]/(x*x*x),1.)",15,2000);
  TF1 *f1p = new TF1("f1p","min([0]+[1]/x+[2]/(x*x)+[3]/(x*x*x),1.)",50,2000);
  f1p->SetParameters(2.2e-5,0.8e-3,0.08,5.5);

  //f1p->SetRange(20.,3500.);
  // //f1p->SetParameters(0.2,0.008,0.1,4e-8);
  //f1p->SetParameters(0.1737, 0.9318, 0.01506, 0); // chi2/NDF=3.4/19, 2018
  //f1p->FixParameter(3,0.);
  //hfr->Fit(f1p,"R"); // 2018
  //hfr->Fit(f1p,"RW"); // 2017
  hfr->Fit(f1p,"R"); // 2022
  f1p->SetRange(15,2000);

  f1p->SetLineColor(kGreen+1);
  f1p->Draw("SAME");
  cout << Form("  TF1 *f1p = new TF1(\"f1p\",\"%s\",15,3500);",
	       f1p->GetExpFormula().Data()) << endl;
  cout << "  f1p->SetParameters(";
  for (int i = 0; i != f1p->GetNpar(); ++i) {
    cout << Form("%s%1.4g",i==0 ? "" : ", ",f1p->GetParameter(i));
  }
  cout << Form("); // chi2/NDF=%1.1f/%d",
	       f1p->GetChisquare(), f1p->GetNDF())
       << endl;

  c1->cd(2);
  // For comparison early 2018 fit used in GamHistosMix
  TF1 *_f1p = new TF1("_f1p","[0]+[1]*exp(-[2]*x)+[3]*x",15,3500);
  //_f1p->SetParameters(7e-5,0.008,0.1,4e-8);
  _f1p->SetParameters(0.1737, 0.9318, 0.01506, 0); // chi2/NDF=3.4/19
  _f1p->SetLineColor(kGray);//kBlack);
  _f1p->SetLineStyle(kDotted);
  _f1p->Draw("SAME");

  c1->cd(1);
  TLatex *tex = new TLatex();
  tex->SetNDC();
  tex->SetTextSize(0.025);
  //tex->DrawLatex(0.20,0.07,"NB1: No MC trigger (HoverELoose) at p_{T}<60 GeV");
  //tex->DrawLatex(0.20,0.04,"NB2: #hat{p}_{T}>15 GeV biases MC at p_{T}<50 GeV");
  tex->DrawLatex(0.20,0.04,"NB: H_{T}>40 GeV biases MC at p_{T}<50 GeV");
  l->SetLineStyle(kDotted);

  c1->cd(1);
  if (!doPerGeV) l->DrawLine(60,10,60,1e8);
  c1->cd(2);
  if (!doPerGeV) l->DrawLine(60,0.2,60,1.4);

  //c1->SaveAs("pdf/drawPurityEstimates.pdf");
  c1->SaveAs(Form("pdf/drawPurityEstimates_%s_%s.pdf",cera,cv));

  drawEMJetScale(eramc, version);
  drawGluonFraction(eramc, version);
  drawGluonResponse(eramc, version);
} // drawPurityEstimates

// Draw EM jet scale estimate from MC
void drawEMJetScale(string era, string version) {

  setTDRStyle();
  TDirectory *curdir = gDirectory;
  const char *cera = era.c_str();
  const char *cv = version.c_str();

  // Open files
  //TFile *fq = new TFile("files/GamHistosFill_mc_2018QCD_v19d.root","READ");
  TFile *fq = new TFile(Form("rootfiles/GamHistosFill_mc_%sQCD_%s.root",
			     cera,cv),"READ");
  assert(fq && !fq->IsZombie());

  curdir->cd();
  
  TProfile *pr1 = (TProfile*)fq->Get("control/prgamqcd");  assert(pr1);
  TProfile *pr2 = (TProfile*)fq->Get("control/prgamqcd2"); assert(pr2);

  TH1D *h = tdrHist("hem","EM jet scale",0.80,1.05);
  TCanvas *c1 = tdrCanvas("c1em",h,4,11,kSquare);
  gPad->SetLogx();

  tdrDraw(pr1,"Pz",kOpenSquare);
  tdrDraw(pr2,"Pz",kFullSquare);

  TF1 *f1 = new TF1("f1","[0]+[1]/x",30,1500);//20,3500);
  pr1->Fit(f1,"RN");
  f1->SetLineColor(kBlack);
  f1->SetLineStyle(kDashed);
  f1->DrawClone("SAME");
  //f1->FixParameter(1,0);
  //pr1->Fit(f1,"RN");
  //f1->Draw("SAME");

  TF1 *f2 = new TF1("f2","[0]+[1]/x",30,1500);//20,3500);
  pr2->Fit(f2,"RN");
  f2->SetLineColor(kBlack);
  f2->SetLineStyle(kSolid);
  f2->DrawClone("SAME");
  //f2->FixParameter(1,0);
  //pr2->Fit(f2,"RN");
  //f2->Draw("SAME");
  
  //c1->SaveAs("pdf/drawPurityEstimates_drawEMJetScale.pdf");
  c1->SaveAs(Form("pdf/drawPurityEstimates_drawEMJetScale_%s.pdf",cera));
} // drawEMJetScale


void drawGluonFraction(string era, string version) {

  setTDRStyle();
  TDirectory *curdir = gDirectory;
  const char *cera = era.c_str();
  const char *cv = version.c_str();
  
  // Open files
  //TFile *fg = new TFile("files/GamHistosFill_mc_2018P8_v19.root","READ");
  //TFile *fg = new TFile("files/GamHistosFill_mc_2018P8_v19e.root","READ");
  TFile *fg = new TFile(Form("rootfiles/GamHistosFill_mc_%sP8_%s.root",
			     cera,cv),"READ");
  //TFile *fq = new TFile("files/GamHistosFill_mc_2018QCD_v19d.root","READ");
  //TFile *fq = new TFile("files/GamHistosFill_mc_2018QCD_v19e.root","READ");
  TFile *fq = new TFile(Form("rootfiles/GamHistosFill_mc_%sQCD_%s.root",
			     cera,cv),"READ");
  assert(fq && !fq->IsZombie());

  curdir->cd();

  TH1D *h = tdrHist("hgf","Flavor fraction",0.0,1.1,"p_{T,EM} (GeV)",40,2000);
  TCanvas *c1 = tdrCanvas("c1gf",h,4,11,kSquare);
  gPad->SetLogx();
  
  //string aflv[] = {"i","b","c","q","g","n"};
  //string aflv[] = {"q","c","b","g","n"};
  //string aflv[] = {"q","c","b","g"};
  string aflv[] = {"ud","s","c","b","g"};
  const int nflv = sizeof(aflv)/sizeof(aflv[0]);

  map<string, int> fmarker;
  fmarker["q"] = kFullSquare;
  fmarker["ud"] = kFullSquare;
  fmarker["s"] = kFullStar;
  fmarker["c"] = kFullDiamond;
  fmarker["b"] = kFullDiamond;
  fmarker["g"] = kFullCircle;
  fmarker["n"] = kFullDotMedium;
  map<string, int> omarker;
  omarker["q"] = kOpenSquare;
  omarker["ud"] = kOpenSquare;
  omarker["s"] = kOpenStar;
  omarker["c"] = kOpenDiamond;
  omarker["b"] = kOpenDiamond;
  omarker["g"] = kOpenCircle;
  omarker["n"] = kFullDotSmall;
  map<string, int> color;
  color["q"] = kBlue;
  color["ud"] = kBlue;
  color["s"] = kCyan+2;
  color["c"] = kGreen+2;
  color["b"] = kRed;
  color["g"] = kOrange+2;
  color["n"] = kBlack;

  TH1D *hiq = (TH1D*)fq->Get("flavor/counts_gii");  assert(hiq);
  for (int iflv = 0; iflv != nflv; ++iflv) {  
    // gbi=b-tags (inclusive), gib=(inclusive) b-jets, gbb=b-tagged b-jets
    const char *ct = "i";
    //const char *cf = "g";
    string sf = aflv[iflv];
    const char *cf = sf.c_str();

    TH1D *hfq = (TH1D*)fq->Get(Form("flavor/counts_g%s%s",ct,cf));
    assert(hfq);
    hfq = (TH1D*)hfq->Clone(Form("hfq_%s",cf));
    hfq->Divide(hiq);

    hfq->GetXaxis()->SetRangeUser(40,2000);
    tdrDraw(hfq,"Pz",fmarker[cf],color[cf]);

    TF1 *f1q = new TF1(Form("f1q_%s_%s",ct,cf),"[0]+[1]*pow(x,[2])+[3]/x",
		       40,2000);//1650.);
    f1q->SetLineColor(color[cf]);
    // Initial parameters from jecsys/minitools/drawMultijetFlavor.C
    if (sf=="g")  f1q->SetParameters(0.84,-0.028,0.3333,0); //gluons
    if (sf=="ud") f1q->SetParameters(0.04,+0.021,0.3333,0); //ud
    if (sf=="q") f1q->SetParameters(0.04,+0.021,0.3333,0); //ud
    if (sf=="c") f1q->SetParameters(0.04,+0.007,0.3333,0); //c
    if (sf=="s") f1q->SetParameters(0.06,-0.003,0.3333,0); //s
    if (sf=="b") f1q->SetParameters(0.02,+0.003,0.3333,0); //b
    hfq->Fit(f1q,"QRN");
    f1q->Draw("SAME");

    if (iflv==0) {
      cout <<"    // Parameters from gamjet/drawPurityEstimates.C (EM+jet)\n";
      cout << Form("    double p[nf][%d] = {",f1q->GetNpar()) << endl;
      cout << Form("      // %s",f1q->GetExpFormula().Data()) << endl;
    }
    cout << Form("      {%1.5g, %1.5g, %1.5g, %1.5g}%s // (%s %1.1f/%d)",
		 f1q->GetParameter(0), f1q->GetParameter(1),
		 f1q->GetParameter(2), f1q->GetParameter(3),
		 (iflv==nflv-1 ? "};" : ","),
		 cf, f1q->GetChisquare(), f1q->GetNDF()) << endl;
  } // for iflv in nflv

  TH1D *hig = (TH1D*)fg->Get("flavor/counts_gii");  assert(hig);
  for (int iflv = 0; iflv != nflv; ++iflv) {  
    // gbi=b-tags (inclusive), gib=(inclusive) b-jets, gbb=b-tagged b-jets
    const char *ct = "i";
    //const char *cf = "g";
    string sf = aflv[iflv];
    const char *cf = sf.c_str();

    TH1D *hfg = (TH1D*)fg->Get(Form("flavor/counts_g%s%s",ct,cf));
    assert(hfg);
    hfg = (TH1D*)hfg->Clone(Form("hfg_%s",cf));
    hfg->Divide(hig);

    hfg->GetXaxis()->SetRangeUser(40,2000);
    tdrDraw(hfg,"Pz",omarker[cf],color[cf]);

    TF1 *f1g = new TF1(Form("f1g_%s_%s",ct,cf),"[0]+[1]*pow(x,[2])"
		       "+[3]*pow(x,[4])",
		      40,2000);
    f1g->SetLineColor(color[cf]);
    f1g->SetLineStyle(kDashed);
    // Initial parameters from 3p fit
    /*
    if (sf=="g")  f1g->SetParameters(0.23506, -0.56376, -0.31037, 0, -1); //g
    if (sf=="ud") f1g->SetParameters(-1.5097, 1.9343, 0.018966, -1,-0.5); //ud
    if (sf=="q") f1g->SetParameters(-1.5097, 1.9343, 0.018966, -1, -0.5); //ud
    if (sf=="c") f1g->SetParameters(0.054869, 1.9682, -0.55643, -1, -1); //c
    if (sf=="s") f1g->SetParameters(-0.046629, 0.45336, -0.2897, 0, -1); //s
    if (sf=="b") f1g->SetParameters(0.02483, 0.5275, -0.91262, 0, -1); //b
    */
    if (sf=="q") f1g->SetParameters(0.756,-0.0277,-0.413,-1.4,-0.423);
    if (sf=="ud") f1g->SetParameters(0.756,-0.0277,-0.413,-1.4,-0.423);
    if (sf=="s") f1g->SetParameters(0.252, 6.13e-06, 1.18, -0.0877, 0.157);
    if (sf=="c") f1g->SetParameters(2.88, -0.00831, 0.00273, -2.53, 0.0137);
    if (sf=="b") f1g->SetParameters(0.774, 0.00124, 0.490, -0.689, 0.0189);
    if (sf=="g") f1g->SetParameters(0.778, -7.34e-05, 0.900, -0.952, -0.0740);
    f1g->SetParameters(0.5,0.01,-0.3,-1);
    hfg->Fit(f1g,"QRN");
    f1g->Draw("SAME");

    if (iflv==0) {
      cout <<"    // Parameters from gamjet/drawPurityEstimates.C (gamma+jet)"
	   << endl;
      cout << Form("    double p[nf][%d] = {",f1g->GetNpar()) << endl;
      cout << Form("      // %s",f1g->GetExpFormula().Data()) << endl;
    }
    cout << Form("      {%1.5g, %1.5g, %1.5g, %1.5g, %1.5g}%s // (%s %1.1f/%d)",
		 f1g->GetParameter(0), f1g->GetParameter(1),
		 f1g->GetParameter(2), f1g->GetParameter(3),
		 f1g->GetParameter(4), 
		 (iflv==nflv-1 ? "};" : ","),
		 cf, f1g->GetChisquare(), f1g->GetNDF()) << endl;
  } // for iflv in nflv

  //c1->SaveAs("pdf/drawPurityEstimates_drawGluonFraction.pdf");
  c1->SaveAs(Form("pdf/drawPurityEstimates_drawGluonFraction_%s.pdf",cera));
} // void drawGluonFraction


// Estimate impact of gJES in data on EM+jet background wrt Photon+jet
void drawGluonResponse(string era, string version) {

  setTDRStyle();
  TDirectory *curdir = gDirectory;
  const char *cera = era.c_str();
  const char *cv = version.c_str();
  
  // Open files
  //TFile *fq = new TFile("files/GamHistosFill_mc_2018QCD_v19d.root","READ");
  TFile *fq = new TFile(Form("rootfiles/GamHistosFill_mc_%sQCD_%s.root",
			     cera,cv),"READ");
  assert(fq && !fq->IsZombie());

  curdir->cd();
  
  // Use pr to get pT binning
  TProfile *pr = (TProfile*)fq->Get("control/prgamqcd");  assert(pr);
  TH1D *hq0 = pr->ProjectionX("hqr0"); hq0->Reset();
  TH1D *hg0 = pr->ProjectionX("hgr0"); hg0->Reset();
  TH1D *hq = pr->ProjectionX("hq"); hq->Reset();
  TH1D *hg = pr->ProjectionX("hg"); hg->Reset();
  Flavor flv;
  for (int i = 1; i != hq->GetNbinsX()+1; ++i) {

    double pt = hq->GetBinCenter(i);

    double rq0 = flv.getResp(pt,0.,1.3,"EMJet13",0); // MC P8
    hq0->SetBinContent(i, rq0);
    double rg0 = flv.getResp(pt,0.,1.3,"PhotonJet13",0); // MC P8
    hg0->SetBinContent(i, rg0);

    double rq = flv.getResp(pt,0.,1.3,"EMJet13",-1); // DATA
    hq->SetBinContent(i, rq);
    double rg = flv.getResp(pt,0.,1.3,"PhotonJet13",-1); // DATA
    hg->SetBinContent(i, rg);
  } // for i

  TH1D *h = tdrHist("h1gr","Flavor JES bias",0.97,1.05);
  TCanvas *c1 = tdrCanvas("c1gr",h,4,11,kSquare);
  gPad->SetLogx();

  TLine *l = new TLine();
  l->SetLineStyle(kSolid);//kDashed);
  l->DrawLine(15,1,3500,1);
  
  tdrDraw(hg0,"HIST",kNone,kRed,kSolid,-1,kNone);
  tdrDraw(hq0,"HIST",kNone,kBlue,kSolid,-1,kNone);

  tdrDraw(hg,"Pz",kOpenCircle,kRed);
  tdrDraw(hq,"Pz",kFullCircle,kBlue);
  
  TF1 *f1 = new TF1("f1gr","[0]+[1]*pow(x,[2])",15,1650);
  f1->SetParameters(1,-0.02,-0.3);
  f1->SetLineColor(kBlue);
  hq->Fit(f1,"QRNW");
  f1->Draw("SAME");

  cout << Form("  TF1 *f1qcd = new TF1(\"f1qcd\",\"%s\",15,1650);",
	       f1->GetExpFormula().Data()) << endl;
  cout << Form("  f1qcd->SetParameters(%1.5g, %1.5g, %1.5g);",
	       f1->GetParameter(0), f1->GetParameter(1),
	       f1->GetParameter(2)) << endl;

  c1->SaveAs(Form("pdf/drawPurityEstimates_drawGluonResponse_%s.pdf",cera));
} // drawGluonResponse
