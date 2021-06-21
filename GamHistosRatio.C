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

void GamHistosRatio() {

  // Merge files, if not already done (delete combination file to redo)
  gSystem->Exec("hadd files/GamHistosFill_data_2018ABCD_v2.root "
		"files/GamHistosFill_data_2018A_v2.root "
		"files/GamHistosFill_data_2018B_v2.root "
		"files/GamHistosFill_data_2018C_v2.root "
		"files/GamHistosFill_data_2018D_v2.root");

  TFile *fd = new TFile("files/GamHistosFill_data_2018ABCD_v2.root","READ");
  assert(fd && !fd->IsZombie());

  TFile *fm = new TFile("files/GamHistosFill_mc_2018P8_v2.root","READ");
  assert(fm && !fm->IsZombie());
  
  TFile *fr = new TFile("files/GamHistosRatio_2018BCD_P8_v2.root","RECREATE");
  assert(fr && !fr->IsZombie());
  fr->mkdir("orig");
  
  // Automatically go through the list of keys (profiles, histograms)
  TList *keys = fm->GetListOfKeys();
  TListIter itkey(keys);
  TObject *obj;
  TKey *key;
  
  bool debug = true;
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
