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

void GamHistosRatios(string ver = "v9", string iov = "2018ABCD");
void GamHistosRatio() {
  GamHistosRatios("v10","2016BCDEF");
  GamHistosRatios("v10","2016FGH");
  GamHistosRatios("v10","2017BCDEF");
  GamHistosRatios("v9","2018ABCD");
}

void GamHistosRatios(string ver, string iov) {

  const char *cv = ver.c_str();
  const char *ci = iov.c_str();

  TFile *fd(0), *fm(0), *fr(0);
  if (ver=="v10" && iov=="2016BCDEF") {
    // Merge files, if not already done (delete combination file to redo)
    gSystem->Exec(Form("hadd files/GamHistosFill_data_2016BCDEF_%s.root "
		       "files/GamHistosFill_data_2016BCD_%s.root "
		       "files/GamHistosFill_data_2016EF_%s.root ",
		       cv,cv,cv));

    fd = new TFile(Form("files/GamHistosFill_data_%s_%s.root",ci,cv),"READ");
    fm = new TFile(Form("files/GamHistosFill_mc_2016P8APV_%s.root",cv),"READ");
    fr = new TFile(Form("files/GamHistosRatio_%s_P8APV_%s.root",ci,cv),"RECREATE");
  }
  if (ver=="v10" && iov=="2016FGH") {

    fd = new TFile(Form("files/GamHistosFill_data_%s_%s.root",ci,cv),"READ");
    fm = new TFile(Form("files/GamHistosFill_mc_2016P8_%s.root",cv),"READ");
    fr = new TFile(Form("files/GamHistosRatio_%s_P8_%s.root",ci,cv),"RECREATE");
  }
  if (ver=="v10" && iov=="2017BCDEF") {
    // Merge files, if not already done (delete combination file to redo)
    gSystem->Exec(Form("hadd files/GamHistosFill_data_2017BCDEF_%s.root "
		       "files/GamHistosFill_data_2017B_%s.root "
		       "files/GamHistosFill_data_2017C_%s.root "
		       "files/GamHistosFill_data_2017D_%s.root "
		       "files/GamHistosFill_data_2017E_%s.root "
		       "files/GamHistosFill_data_2017F_%s.root ",
		       cv,cv,cv,cv,cv,cv));

    fd = new TFile(Form("files/GamHistosFill_data_%s_%s.root",ci,cv),"READ");
    fm = new TFile(Form("files/GamHistosFill_mc_2017P8_%s.root",cv),"READ");
    fr = new TFile(Form("files/GamHistosRatio_%s_P8_%s.root",ci,cv),"RECREATE");
  }
  if (ver=="v9" && iov=="2018ABCD") {

    // Merge files, if not already done (delete combination file to redo)
    gSystem->Exec(Form("hadd files/GamHistosFill_data_2018ABCD_%s.root "
		       "files/GamHistosFill_data_2018A_%s.root "
		       "files/GamHistosFill_data_2018B_%s.root "
		       "files/GamHistosFill_data_2018C_%s.root "
		       //"files/GamHistosFill_data_2018D1_%s.root "
		       //"files/GamHistosFill_data_2018D2_%s.root",
		       "files/GamHistosFill_data_2018D_%s.root",
		       //cv,cv,cv,cv,cv,cv));
		       cv,cv,cv,cv,cv));
    
    fd = new TFile(Form("files/GamHistosFill_data_%s_%s.root",ci,cv),"READ");
    fm = new TFile(Form("files/GamHistosFill_mc_2018P8_%s.root",cv),"READ");
    fr = new TFile(Form("files/GamHistosRatio_%s_P8_%s.root",ci,cv),"RECREATE");
  }

  assert(fd && !fd->IsZombie());
  assert(fm && !fm->IsZombie());
  assert(fr && !fr->IsZombie());
  fr->mkdir("orig");
  
  cout << "Merging files "<<fd->GetName()<<" and "<<fm->GetName() << endl;  
  cout << "Output file " << fr->GetName() << endl << flush;
  
  // Automatically go through the list of keys (profiles, histograms)
  TList *keys = fm->GetListOfKeys();
  TListIter itkey(keys);
  TObject *obj;
  TKey *key;
  
  bool debug = false;
  while ( (key = dynamic_cast<TKey*>(itkey.Next())) ) {
    if (debug) cout << key->GetName() << endl << flush;
    obj = key->ReadObj(); assert(obj);

    fm->cd();
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
      fm->cd();

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
    else if (obj->InheritsFrom("TH1D")) {

      TH1D *hm = (TH1D*)obj;
      TString mcname = obj->GetName();
      if (mcname.Contains("RawNEvents")) hm->Scale(750e3);

      TString dataname = mcname;
      dataname.ReplaceAll("MC","DATA");
      TH1D *hd = (TProfile*)fd->Get(dataname.Data());
      assert(hd);

      TString rationame = mcname;
      rationame.ReplaceAll("_MC","");
      TH1D *hr = (TH1D*)hd->Clone(rationame.Data());
      hr->Divide(hm);

      fr->cd();
      //hm->SetDirectory(gDirectory);
      //hd->SetDirectory(gDirectory);
      //hr->SetDirectory(gDirectory);
      hm->Write();
      hd->Write();
      hr->Write();
    } // TH1D
  } // while key in itkey

  cout << "Writing file " << fr->GetName() << endl << flush;
  fr->Write();
  fr->Close();
  cout << "File closed." << endl << flush;
} // GamHistosRatio

// J. Mnich, DG's talk on June 22, 2021
// Primary target of LHC: di-Higgs, ATLAS HH->gamma gamma b bar (4b?)
