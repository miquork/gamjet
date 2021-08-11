// Purpose: draw stability of photon+jet results vs pT over time
//          compare data and MC for response, HOE, R9 etc.
#include "TFile.h"
#include "TH1D.h"
#include "TProfile.h"
#include "TLine.h"

#include "tdrstyle_mod15.C"

bool addMPFu2n = true;
bool addG1toMPF = true;
bool addG12toMPF = false;

// Forward declaration of call
void drawPhotonJetVsPtVsIOVs(string so, string var, string name,
			     double y1, double y2, double z1, double z2);

//void drawPhotonJetVsPtVsIOV(string so = "resp_MPFchs_%s_a100_eta00_13") {
//void drawPhotonJetVsPtVsIOVs(string so = "control/phoevspt",
//			     string var = "H/E",
//			     double y1 = 0, double y2 = 0.01,
//			     double z1 = 0.7, double z2 = 1.4) {
			    
// Multiple calls to draw function
void drawPhotonJetVsPtVsIOV() {
  //drawPhotonJetVsPtVsIOVs("control/pphoj","Leakage","Leakage",0.,0.125,-0.005,0.03);//0.5,2.5);
  //drawPhotonJetVsPtVsIOVs("control/phoevspt","H/E","HoE",0.,0.01,0.7,1.6);
  //drawPhotonJetVsPtVsIOVs("control/pr9vspt","R9","R9",0.83,1.03,0.9,1.14);
  //drawPhotonJetVsPtVsIOVs("control/pgain1vspt","Gain1","Gain1",0.,1.2,0.5,1.5);
  //drawPhotonJetVsPtVsIOVs("control/pgain12vspt","Gain12","Gain12",0.,1.2,0.5,1.5);
  //drawPhotonJetVsPtVsIOVs("resp_MPFchs_%s_a100_eta00_13","MPF","MPF",
  //			  0.94,1.08,0.95,1.03);
  //drawPhotonJetVsPtVsIOVs("resp_MPFR1chs_%s_a100_eta00_13","MPF1","MPF1",
  //			  0.8,1.10,0.95,1.05);
  //drawPhotonJetVsPtVsIOVs("resp_MPFRnchs_%s_a100_eta00_13","MPFn","MPFn",
  //			  -0.1,0.2,-0.03,0.03);
  drawPhotonJetVsPtVsIOVs("resp_MpfRuchs_%s_a100_eta00_13","MPFu","MPFu",
  			  -0.05,0.15,-0.03,0.02);
} // drawPhotonJetVsPtVsIOV

void drawPhotonJetVsPtVsIOVs(string so, string var, string name,
			     double y1, double y2, double z1, double z2) {
  setTDRStyle();
  TDirectory *curdir = gDirectory;

  string iovs[] = {"2016BCDEF","2016FGH","2017BCDEF","2018ABCD","Run2"};
  string mcs[] = {"2016P8APV","2016P8","2017P8","2018P8","Run2P8"};
  const int niov = sizeof(iovs)/sizeof(iovs[0]);
  const int nmc = sizeof(mcs)/sizeof(mcs[0]);
  assert(niov==nmc);
  
  map<string,int> mcolor;
  mcolor["2016BCDEF"] = kBlue;
  mcolor["2016FGH"] = kCyan+2;
  mcolor["2017BCDEF"] = kGreen+2;
  mcolor["2018ABCD"] = kRed;
  mcolor["Run2"] = kBlack;

  const char *cvar = var.c_str();
  const char *cname = name.c_str();

  TH1D *h = tdrHist("h",cvar,y1,y2);
  TH1D *h2 = tdrHist("h2","Data/MC",z1,z2);
  lumi_13TeV = "Run2";
  TCanvas *c1 = tdrDiCanvas(Form("c1_%s",cname),h,h2,4,11);


  c1->cd(1);
  TLine *l = new TLine();
  l->SetLineStyle(kDashed);
  l->DrawLine(h->GetXaxis()->GetXmin(),1,h->GetXaxis()->GetXmax(),1);
  l->DrawLine(h->GetXaxis()->GetXmin(),0,h->GetXaxis()->GetXmax(),0);
  gPad->SetLogx();

  TLegend *leg = tdrLeg(0.60,0.90-niov*0.06,0.80,0.90);

  c1->cd(2);
  gPad->SetLogx();
  l->DrawLine(h->GetXaxis()->GetXmin(),1,h->GetXaxis()->GetXmax(),1);
  l->DrawLine(h->GetXaxis()->GetXmin(),0,h->GetXaxis()->GetXmax(),0);

  for (int i = 0; i != niov; ++i) {
  
    const char *ciov = iovs[i].c_str();
    const char *cmc = mcs[i].c_str();

    TFile *fd(0), *fm(0);
    fd = new TFile(Form("files/GamHistosFill_data_%s_v11.root",ciov));
    assert(fd && !fd->IsZombie());
    fm = new TFile(Form("files/GamHistosFill_mc_%s_v11.root",cmc));
    assert(fm && !fm->IsZombie());

    curdir->cd();
    
    TObject *od = fd->Get(Form(so.c_str(),"DATA")); assert(od);
    TObject *om = fm->Get(Form(so.c_str(),"MC")); assert(om);
    
    TH1D *hd(0), *hm(0);
    if (od->InheritsFrom("TProfile")) {
      hd = ((TProfile*)od)->ProjectionX(Form("hd_%s_%s",cvar,ciov));
      hm = ((TProfile*)om)->ProjectionX(Form("hm_%s_%s",cvar,ciov));

      if (name=="MPFn" && addMPFu2n) {
	const char *co = "resp_MpfRuchs_%s_a100_eta00_13";
	TProfile *pd = (TProfile*)fd->Get(Form(co,"DATA")); assert(pd);
	TProfile *pm = (TProfile*)fm->Get(Form(co,"MC")); assert(pm);
	hd->Add(pd,1.2);
	hm->Add(pm,1.2);
      }
      if ((name=="MPF" || name=="MPF1") && addG1toMPF) {
	const char *co = "control/pgain1vspt";
	TProfile *pd = (TProfile*)fd->Get(Form(co,"DATA")); assert(pd);
	TProfile *pm = (TProfile*)fm->Get(Form(co,"MC")); assert(pm);
	hd->Add(pd,0.02);
      }
      if ((name=="MPF" || name=="MPF1") && addG12toMPF) {
	const char *co = "control/pgain12vspt";
	TProfile *pd = (TProfile*)fd->Get(Form(co,"DATA")); assert(pd);
	TProfile *pm = (TProfile*)fm->Get(Form(co,"MC")); assert(pm);
	hd->Add(pd,0.0);
      }
    }
    else {
      hd = (TH1D*)od;
      hm = (TH1D*)om;
    }
    assert(hd);
    assert(hm);
    
    TH1D *hr = (TH1D*)hd->Clone(Form("hr_%s_%s",cvar,ciov));
    if (name=="MPFn" || name=="MPFu" || name=="Leakage") {
      hr->Add(hm,-1);
      h2->SetYTitle("Data-MC");
    }
    else
      hr->Divide(hm);
    
    c1->cd(1);
    gPad->SetLogx();
    tdrDraw(hm,"H",kNone,mcolor[ciov],kSolid,-1,kNone);
    tdrDraw(hd,"Pz",kFullCircle,mcolor[ciov]);
    
    leg->AddEntry(hd,ciov,"PLE");

    c1->cd(2);
    gPad->SetLogx();
    tdrDraw(hr,"Pz",kFullCircle,mcolor[ciov]);
  } // for iov

  c1->SaveAs(Form("pdf/drawPhotonJetVsPtVsIOVs_%s.pdf",name.c_str()));
} // void drawPhotonJetVsPtVsIOVs
