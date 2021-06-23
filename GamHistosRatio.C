// Purpose: produce data/MC ratios for JEC global fit
//          copy data and MC into the combination file
//          leave controls still in separate data and MC files
#include "TSystem.h"
#include "TFile.h"
#include "TDirectory.h"
#include "TList.h"
#include "TObject.h"
#include "TKey.h"
#include "TProfile.h"
#include "TH1D.h"
#include "TGraphErrors.h"

#include <iostream>
using namespace std;

// Remove zeroes
void cleanGraph(TGraphErrors *g) {
  for (int i = g->GetN()-1; i != -1; --i) {
    if (g->GetY()[i]==0 && g->GetEY()[i]==0) g->RemovePoint(i);
  }
} // cleanGraph

// Replace pT in graph
void replacePt(TGraphErrors *g, TH1 *h) {
  for (int i = 0; i != g->GetN(); ++i) {
    double x = g->GetX()[i];
    int j = h->FindBin(x);
    double xnew = h->GetBinContent(j);
    double exnew = h->GetBinError(j);
    g->SetPoint(i, xnew, g->GetY()[i]);
    g->SetPointError(i, exnew, g->GetEY()[i]);
  }

  return;
}


void GamHistosRatio() {

  // Merge files, if not already done (delete combination file to redo)
  gSystem->Exec("hadd files/GamHistosFill_data_2018ABCD_v3.root "
		"files/GamHistosFill_data_2018A_v3.root "
		"files/GamHistosFill_data_2018B_v3.root "
		"files/GamHistosFill_data_2018C_v3.root "
		"files/GamHistosFill_data_2018D_v3.root");

  TFile *fd = new TFile("files/GamHistosFill_data_2018ABCD_v3.root","READ");
  assert(fd && !fd->IsZombie());

  TFile *fm = new TFile("files/GamHistosFill_mc_2018P8_v3.root","READ");
  assert(fm && !fm->IsZombie());
  
  TFile *fr = new TFile("files/GamHistosRatio_2018ABCD_P8_v3.root","RECREATE");
  assert(fr && !fr->IsZombie());
  fr->mkdir("orig");
  
  // Automatically go through the list of keys (profiles, histograms)
  TList *keys = fm->GetListOfKeys();
  TListIter itkey(keys);
  TObject *obj;
  TKey *key;
  
  bool debug = false;
  while ( (key = dynamic_cast<TKey*>(itkey.Next())) ) {
    if (debug) cout << key->GetName() << endl << flush;
    obj = key->ReadObj(); assert(obj);

    if (obj->InheritsFrom("TProfile")) {

      TProfile *pm = (TProfile*)obj;
      TString mcname = obj->GetName();
      TString dataname = mcname;
      dataname.ReplaceAll("MC","DATA");
      TProfile *pd = (TProfile*)fd->Get(dataname.Data());
      assert(pd);

      fr->cd("orig");
      TH1D *hm = pm->ProjectionX(Form("h_%s",pm->GetName()));
      TH1D *hd = pd->ProjectionX(Form("h_%s",pd->GetName()));
      TString rationame = mcname;
      rationame.ReplaceAll("_MC","");
      TH1D *hr = (TH1D*)hd->Clone(Form("h_%s",rationame.Data()));
      hr->Divide(hm);

      // Retrieve pT mapping
      char c[256];
      int a, y1, y2;
      if (debug) cout << "mcname: " << mcname << endl << flush;
      sscanf(mcname.Data(),"resp_%[PtGamBalMPFpfR1nuchs]_MC_a%d_eta%d_%d",
	     c,&a,&y1,&y2);
      if (debug) cout << "c: " << c << endl << flush;
      TString mcptname = mcname;
      mcptname.ReplaceAll(c,"PtGam");
      if (debug) cout << "PtGam(MC):" << mcptname << endl << flush;
      TString dataptname = dataname;
      dataptname.ReplaceAll(c,"PtGam");
      if (debug) cout << "PtGam(data):" << dataptname << endl << flush;
      TProfile *pmpt = (TProfile*)fm->Get(mcptname.Data());   assert(pmpt);
      TProfile *pdpt = (TProfile*)fd->Get(dataptname.Data()); assert(pdpt);

      // Detailed calculation for data/MC average
      TH1D *hrpt = pmpt->ProjectionX("tmp");
      for (int i = 1; i != hrpt->GetNbinsX()+1; ++i) {
	hrpt->SetBinContent(i, 0.5*(pmpt->GetBinContent(i)+
				    pdpt->GetBinContent(i)));
	hrpt->SetBinError(i, sqrt(pow(0.5*(pmpt->GetBinContent(i)-
					   pdpt->GetBinContent(i)),2) +
				  pow(pmpt->GetBinError(i),2) +
				  pow(pdpt->GetBinError(i),2)));
      } // for i in hrpt

      fr->cd();
      TGraphErrors *gd = new TGraphErrors(hd);
      gd->SetNameTitle(dataname.Data(), dataname.Data());
      TGraphErrors *gm = new TGraphErrors(hm);
      gm->SetNameTitle(mcname.Data(), mcname.Data());
      TGraphErrors *gr = new TGraphErrors(hr);
      gr->SetNameTitle(rationame.Data(), rationame.Data());
      
      cleanGraph(gm);
      cleanGraph(gd);
      cleanGraph(gr);
      
      // Replace pT with more accurate estimate
      replacePt(gm,pmpt);
      replacePt(gd,pdpt);
      replacePt(gr,hrpt);
      delete hrpt;
      
      gm->SetLineColor(kRed);
      gd->SetLineColor(kBlue);
      gr->SetLineColor(kBlack);
      
      gm->Write(); 
      gd->Write();
      gr->Write();
    } // TProfile
  } // while key in itkey

  fr->Write();
  fr->Close();
} // GamHistosRatio

// J. Mnich, DG's talk on June 22, 2021
// Primary target of LHC: di-Higgs, ATLAS HH->gamma gamma b bar (4b?)
