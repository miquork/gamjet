#define GamHistosFill_cxx
#include "GamHistosFill.h"
#include <TH2.h>
#include <TStyle.h>
#include <TCanvas.h>

#include "TH1D.h"
#include "TProfile.h"
#include "TProfile2D.h"
#include "TLorentzVector.h"
#include "TStopwatch.h"

#include <iostream>
using namespace std;
//using namespace GamHistosFill;

#include "parsePileUpJSON.C"

bool _gh_debug = false;

// Classes to structure sets of histograms and profiles
struct BasicHistos {
  TH1D *hn;
  TH1D *hxsec;
  TProfile *prpt;
  TProfile *prbal;
  TProfile *prdb;
  TProfile *prmpf;
  TProfile *prmpf1;
  TProfile *prmpfn;
  TProfile *prmpfu;
  TProfile *prho;
};

// Helper function to retrieve FactorizedJetCorrector 
FactorizedJetCorrector *getFJC(string l1="", string l2="", string res="",
			       string path="") {

  // Set default jet algo                                                       
  if (l1!="" && !(TString(l1.c_str()).Contains("_AK")))
    l1 += "_AK4PFchs";
  if (l2!="" && !(TString(l2.c_str()).Contains("_AK")))
    l2 += "_AK4PFchs";
  if (res!="" && !(TString(res.c_str()).Contains("_AK")))
    res += "_AK4PFchs";

  // Set default path
  if (path=="") path = "CondFormats/JetMETObjects/data";
  const char *cd = path.c_str();
  const char *cl1 = l1.c_str();
  const char *cl2 = l2.c_str();
  const char *cres = res.c_str();
  string s("");

  vector<JetCorrectorParameters> v;
  if (l1!=""){
    s = Form("%s/%s.txt",cd,cl1);
    cout << s << endl << flush;
    JetCorrectorParameters *pl1 = new JetCorrectorParameters(s);
    v.push_back(*pl1);
  }
  if (l2!="") {
    s = Form("%s/%s.txt",cd,cl2);
    cout << s << endl << flush;
    JetCorrectorParameters *pl2 = new JetCorrectorParameters(s);
    v.push_back(*pl2);
  }
  if (res!="") {
    s = Form("%s/%s.txt",cd,cres);
    cout << s << endl << flush;
    JetCorrectorParameters *pres = new JetCorrectorParameters(s);
    v.push_back(*pres);
  }
  FactorizedJetCorrector *jec = new FactorizedJetCorrector(v);

  return jec;
} // getJFC      

void GamHistosFill::Loop()
{
//   In a ROOT session, you can do:
//      root> .L GamHistosFill.C
//      root> GamHistosFill t
//      root> t.GetEntry(12); // Fill t data members with entry number 12
//      root> t.Show();       // Show values of entry 12
//      root> t.Show(16);     // Read and show values of entry 16
//      root> t.Loop();       // Loop on all entries
//

//     This is the loop skeleton where:
//    jentry is the global entry number in the chain
//    ientry is the entry number in the current Tree
//  Note that the argument to GetEntry must be:
//    jentry for TChain::GetEntry
//    ientry for TTree::GetEntry and TBranch::GetEntry
//
//       To read only selected branches, Insert statements like:
// METHOD1:
//    fChain->SetBranchStatus("*",0);  // disable all branches
//    fChain->SetBranchStatus("branchname",1);  // activate branchname
// METHOD2: replace line
//    fChain->GetEntry(jentry);       //read all branches
//by  b_branchname->GetEntry(ientry); //read only this branch
  if (fChain == 0) return;

  TStopwatch fulltime, laptime;
  fulltime.Start();
  TDatime bgn;
  int nlap(0);

  int _ntot(0), _nevents(0), _nbadevents_json(0), _nbadevents_trigger(0);
  int _nbadevents_veto(0);
  
  if (true) { // ProcessFast
    fChain->SetBranchStatus("*",0);  // disable all branches
    
    // Baseline triggers with very high prescale except Photon200
    if (is18) {
      fChain->SetBranchStatus("HLT_Photon20",1);
    }
    if (is16) {
      if (!isMC) fChain->SetBranchStatus("HLT_Photon22",1);
      if (!isMC) fChain->SetBranchStatus("HLT_Photon30",1);
      if (!isMC) fChain->SetBranchStatus("HLT_Photon36",1);
    }
    if (is17 || is18) {
      fChain->SetBranchStatus("HLT_Photon33",1);
      fChain->SetBranchStatus("HLT_Photon200",1);
    //fChain->SetBranchStatus("HLT_Photon500",1);
    //fChain->SetBranchStatus("HLT_Photon600",1);
    }
    fChain->SetBranchStatus("HLT_Photon50",1);
    fChain->SetBranchStatus("HLT_Photon75",1);
    fChain->SetBranchStatus("HLT_Photon90",1);
    fChain->SetBranchStatus("HLT_Photon120",1);
    fChain->SetBranchStatus("HLT_Photon175",1);

    // Presumably backup triggers for unprescaled Photon175 or Photon200
    if (is16 && !isMC) fChain->SetBranchStatus("HLT_Photon165_HE10",1);
    if (is16 && !isMC) fChain->SetBranchStatus("HLT_Photon250_NoHE",1);
    fChain->SetBranchStatus("HLT_Photon300_NoHE",1);
    
    // Triggers to recover pT=105-230 GeV range. Efficient, low prescale
    // Not active in some of 2018 data (~10%)?
    if (is18) {
      fChain->SetBranchStatus("HLT_Photon100EB_TightID_TightIso",1); // ok
      fChain->SetBranchStatus("HLT_Photon110EB_TightID_TightIso",1); // best
      fChain->SetBranchStatus("HLT_Photon120EB_TightID_TightIso",1); // backup
    }

    // Triggers to recover 60-105 GeV range. However, inefficient up to high pT
    // Possibly medium HLT cuts not fully consistent with tight ID?
    if (is16 && !isMC) {
      fChain->SetBranchStatus("HLT_Photon22_R9Id90_HE10_IsoM",1);
      fChain->SetBranchStatus("HLT_Photon30_R9Id90_HE10_IsoM",1);
      fChain->SetBranchStatus("HLT_Photon36_R9Id90_HE10_IsoM",1);
    }
    fChain->SetBranchStatus("HLT_Photon50_R9Id90_HE10_IsoM",1);
    fChain->SetBranchStatus("HLT_Photon75_R9Id90_HE10_IsoM",1);
    fChain->SetBranchStatus("HLT_Photon90_R9Id90_HE10_IsoM",1);
    fChain->SetBranchStatus("HLT_Photon120_R9Id90_HE10_IsoM",1);
    fChain->SetBranchStatus("HLT_Photon165_R9Id90_HE10_IsoM",1);
    
    // Triggers to recover 30-60 GeV range. Efficient above 30 GeV
    if (is17 || is18) {
      fChain->SetBranchStatus("HLT_Photon20_HoverELoose",1);
      fChain->SetBranchStatus("HLT_Photon30_HoverELoose",1);
    }
    if (is17 && !isMC) {
      fChain->SetBranchStatus("HLT_Photon40_HoverELoose",1);
      fChain->SetBranchStatus("HLT_Photon50_HoverELoose",1);
      fChain->SetBranchStatus("HLT_Photon60_HoverELoose",1);
    }
    
    // JSON filtering, PU reweighing
    if (isMC) fChain->SetBranchStatus("Pileup_nTrueInt");
    fChain->SetBranchStatus("run",1);
    if (!isMC) fChain->SetBranchStatus("luminosityBlock",1);
    //if (!isMC) fChain->SetBranchStatus("event",1);
    
    // Event filters listing from Sami:
    // twiki.cern.ch/twiki/bin/viewauth/CMS/MissingETOptionalFiltersRun2
    // #2018_2017_data_and_MC_UL
    fChain->SetBranchStatus("Flag_goodVertices",1);
    fChain->SetBranchStatus("Flag_globalSuperTightHalo2016Filter",1);
    fChain->SetBranchStatus("Flag_HBHENoiseFilter",1);
    fChain->SetBranchStatus("Flag_HBHENoiseIsoFilter",1);
    fChain->SetBranchStatus("Flag_EcalDeadCellTriggerPrimitiveFilter",1);
    fChain->SetBranchStatus("Flag_BadPFMuonFilter",1);
    //fChain->SetBranchStatus("Flag_BadPFMuonDzFilter",1); // not in nAOD?
    //inputList.("Flag_BadPFMuonDzFilter",1); // new in UL, but part of above?
    //inputList.("Flag_BadChargedCandidateFilter",0); // not recommended
    //inputList.("Flag_globalTightHalo2016Filter",0); // obsolete?
    //inputList.("Flag_CSCTightHaloFilter",0); // obsolete?
    if (!is16) {
      fChain->SetBranchStatus("Flag_ecalBadCalibFilter",1);//new (add for Sami)
    }
    //if dataset.isData:
    //if (!isMC) {
    fChain->SetBranchStatus("Flag_eeBadScFilter",1); // MC added 7 July 2021
    //}
    
    // MC weights
    if (isMC)  fChain->SetBranchStatus("genWeight",1);
    if (isMC)  fChain->SetBranchStatus("nPSWeight",1);
    if (isMC)  fChain->SetBranchStatus("PSWeight",1);

    fChain->SetBranchStatus("fixedGridRhoFastjetAll",1);
    fChain->SetBranchStatus("PV_npvs",1);
    fChain->SetBranchStatus("PV_npvsGood",1);

    fChain->SetBranchStatus("ChsMET_pt",1);
    fChain->SetBranchStatus("ChsMET_phi",1);
    
    fChain->SetBranchStatus("nPhoton",1);
    fChain->SetBranchStatus("Photon_pt",1);
    fChain->SetBranchStatus("Photon_eta",1);
    fChain->SetBranchStatus("Photon_phi",1);
    fChain->SetBranchStatus("Photon_mass",1);
    fChain->SetBranchStatus("Photon_hoe",1);
    if (is17 && isMC && isQCD)
      fChain->SetBranchStatus("Photon_cutBasedBitmap",1);
    else
      fChain->SetBranchStatus("Photon_cutBased",1);
    fChain->SetBranchStatus("Photon_jetIdx",1);

    fChain->SetBranchStatus("Photon_seedGain",1);
    //if (b_Photon_eCorr)
    fChain->SetBranchStatus("Photon_eCorr",1); // not in 2016
    fChain->SetBranchStatus("Photon_energyErr",1);
    fChain->SetBranchStatus("Photon_r9",1);
    
    fChain->SetBranchStatus("nJet",1);
    fChain->SetBranchStatus("Jet_pt",1);
    fChain->SetBranchStatus("Jet_eta",1);
    fChain->SetBranchStatus("Jet_phi",1);
    fChain->SetBranchStatus("Jet_mass",1);
    fChain->SetBranchStatus("Jet_rawFactor",1);
    fChain->SetBranchStatus("Jet_area",1);
    fChain->SetBranchStatus("Jet_jetId",1);

    // PF composition
    fChain->SetBranchStatus("Jet_chHEF",1);
    fChain->SetBranchStatus("Jet_neHEF",1);
    fChain->SetBranchStatus("Jet_neEmEF",1);
    fChain->SetBranchStatus("Jet_chEmEF",1);
    fChain->SetBranchStatus("Jet_muEF",1);
    fChain->SetBranchStatus("Jet_chFPV0EF",1);

    if (isMC) fChain->SetBranchStatus("Jet_genJetIdx",1);
    
    fChain->SetBranchStatus("Jet_btagDeepB",1);
    fChain->SetBranchStatus("Jet_btagDeepC",1);
    fChain->SetBranchStatus("Jet_qgl",1);

    /*
      fChain->SetBranchStatus("nLHEPart",1);
      fChain->SetBranchStatus("LHEPart_pt",1);
      fChain->SetBranchStatus("LHEPart_eta",1);
      fChain->SetBranchStatus("LHEPart_phi",1);
      fChain->SetBranchStatus("Jet_mass",1);
      fChain->SetBranchStatus("LHEPart_status",1);
      fChain->SetBranchStatus("LHEPart_pdgId",1);
    */
    
    // Photon pT correlates with Generator_binvar (pThat) in P8 PtFlat,
    // and with LHEPart_pt[2] (parton photon) in MadGraph HT samples, but
    // MadGraph has plenty of uncorrelated soft and hard photons as well
    if (isMC) {
      if (!isQCD) {
	fChain->SetBranchStatus("nGenIsolatedPhoton",1);
	fChain->SetBranchStatus("GenIsolatedPhoton_pt",1);
	fChain->SetBranchStatus("GenIsolatedPhoton_eta",1);
	fChain->SetBranchStatus("GenIsolatedPhoton_phi",1);
	fChain->SetBranchStatus("GenIsolatedPhoton_mass",1);
      }
      else
	nGenIsolatedPhoton = 0;
    }
    if (isMC) {
      fChain->SetBranchStatus("nGenJet",1);
      fChain->SetBranchStatus("GenJet_pt",1);
      fChain->SetBranchStatus("GenJet_eta",1);
      fChain->SetBranchStatus("GenJet_phi",1);
      fChain->SetBranchStatus("GenJet_mass",1);
      fChain->SetBranchStatus("GenJet_partonFlavour",1);
    }
  }
  
  // Select appropriate L1RC for type-I MET L1L2L3-RC calculation
  FactorizedJetCorrector *jecl1rc(0);
  string& ds = dataset;
  if (ds=="2016B")    jecl1rc = getFJC("Summer19UL16APV_RunBCD_V7_DATA_L1RC");
  if (ds=="2016C")    jecl1rc = getFJC("Summer19UL16APV_RunBCD_V7_DATA_L1RC");
  if (ds=="2016D")    jecl1rc = getFJC("Summer19UL16APV_RunBCD_V7_DATA_L1RC");
  if (ds=="2016BCD")  jecl1rc = getFJC("Summer19UL16APV_RunBCD_V7_DATA_L1RC");
  if (ds=="2016E")    jecl1rc = getFJC("Summer19UL16APV_RunEF_V7_DATA_L1RC");
  if (ds=="2016F")    jecl1rc = getFJC("Summer19UL16APV_RunEF_V7_DATA_L1RC");
  if (ds=="2016EF")   jecl1rc = getFJC("Summer19UL16APV_RunEF_V7_DATA_L1RC");
  //if (ds=="2016BCDEF")jecl1rc=getFJC("Summer19UL16APV_RunBCDEF_V7_DATA_L1RC");

  if (ds=="2016FG")   jecl1rc = getFJC("Summer19UL16_RunFGH_V7_DATA_L1RC");
  if (ds=="2016H")    jecl1rc = getFJC("Summer19UL16_RunFGH_V7_DATA_L1RC");
  if (ds=="2016FGH")  jecl1rc = getFJC("Summer19UL16_RunFGH_V7_DATA_L1RC");

  if (ds=="2017B")  jecl1rc = getFJC("Summer19UL17_RunB_V5_DATA_L1RC");
  if (ds=="2017C")  jecl1rc = getFJC("Summer19UL17_RunC_V5_DATA_L1RC");
  if (ds=="2017D")  jecl1rc = getFJC("Summer19UL17_RunD_V5_DATA_L1RC");
  if (ds=="2017E")  jecl1rc = getFJC("Summer19UL17_RunE_V5_DATA_L1RC");
  if (ds=="2017F")  jecl1rc = getFJC("Summer19UL17_RunF_V5_DATA_L1RC");
  //if (ds=="2017BCDEF")jecl1rc=getFJC("Summer19UL17_RunBCDEF_V5_DATA_L1RC");

  if (ds=="2018A")  jecl1rc = getFJC("Summer19UL18_RunA_V5_DATA_L1RC");
  if (ds=="2018B")  jecl1rc = getFJC("Summer19UL18_RunB_V5_DATA_L1RC");
  if (ds=="2018C")  jecl1rc = getFJC("Summer19UL18_RunC_V5_DATA_L1RC");
  if (ds=="2018D")  jecl1rc = getFJC("Summer19UL18_RunD_V5_DATA_L1RC");

  if (ds=="2018A1") jecl1rc = getFJC("Summer19UL18_RunA_V5_DATA_L1RC");
  if (ds=="2018A2") jecl1rc = getFJC("Summer19UL18_RunA_V5_DATA_L1RC");
  if (ds=="2018D1") jecl1rc = getFJC("Summer19UL18_RunD_V5_DATA_L1RC");
  if (ds=="2018D2") jecl1rc = getFJC("Summer19UL18_RunD_V5_DATA_L1RC");
  if (ds=="2018D3") jecl1rc = getFJC("Summer19UL18_RunD_V5_DATA_L1RC");
  if (ds=="2018D4") jecl1rc = getFJC("Summer19UL18_RunD_V5_DATA_L1RC");
  //if (ds=="2018ABCD")jecl1rc=getFJC("Summer19UL18_RunABCD_V5_DATA_L1RC");

  if (ds=="2016P8")    jecl1rc = getFJC("Summer19UL16_V7_MC_L1RC");
  if (ds=="2016APVP8") jecl1rc = getFJC("Summer19UL16APV_V7_MC_L1RC");
  if (ds=="2017P8")    jecl1rc = getFJC("Summer19UL17_V5_MC_L1RC");
  if (ds=="2018P8")    jecl1rc = getFJC("Summer19UL18_V5_MC_L1RC");
  //
  if (ds=="2016QCD")    jecl1rc = getFJC("Summer19UL16_V7_MC_L1RC");
  if (ds=="2016APVQCD") jecl1rc = getFJC("Summer19UL16APV_V7_MC_L1RC");
  if (ds=="2017QCD")    jecl1rc = getFJC("Summer19UL17_V5_MC_L1RC");
  if (ds=="2018QCD")    jecl1rc = getFJC("Summer19UL18_V5_MC_L1RC");
  assert(jecl1rc);
  
  string sera("");
  if (ds=="2016APVP8" || ds=="2016APVQCD") sera = "2016APV";
  if (ds=="2016P8" || ds=="2016QCD") sera = "2016FGH";
  if (ds=="2017P8" || ds=="2017QCD") sera = "2017";
  if (ds=="2018P8" || ds=="2018QCD") sera = "2018";
  //
  if (ds=="2016B"||ds=="2016C"||ds=="2016D"||ds=="2016BCD"||
      ds=="2016E"||ds=="2016F"||ds=="2016EF"||ds=="2016BCDEF") sera = "2016APV";
  if (ds=="2016FG"||ds=="2016H"||ds=="2016FGH") sera = "2016FGH";
  if (ds=="2017B"||ds=="2017C"||ds=="2017D"||ds=="2017E"||ds=="2017F"||
      ds=="2017BCDEF") sera = "2017";
  if (ds=="2018A"||ds=="2018A1"||ds=="2018A2"||ds=="2018B"||ds=="2018C"||
      ds=="2018D"||ds=="2018D1"||ds=="2018D2"||ds=="2018D3"||ds=="2018D4")
    sera = "2018";
  assert(sera!="");

  // Load JSON files
  if (TString(ds.c_str()).Contains("2016"))
    LoadJSON("files/Cert_271036-284044_13TeV_Legacy2016_Collisions16_JSON.txt");
  if (TString(ds.c_str()).Contains("2017"))
    LoadJSON("files/Cert_294927-306462_13TeV_UL2017_Collisions17_GoldenJSON.txt");
  if (TString(ds.c_str()).Contains("2018"))
    LoadJSON("files/Cert_314472-325175_13TeV_Legacy2018_Collisions18_JSON.txt");

  // Load pileup JSON
  parsePileUpJSON("files/pileup_ASCII_UL16-UL18.txt");

  // Load pileup profiles
  LoadPU();
  
  // Load veto maps
  // JECDatabase/jet_veto_maps/Summer19UL16_V0/hotjets-UL16.root
  // JECDatabase/jet_veto_maps/Summer19UL17_V2/hotjets-UL17_v2.root
  // JECDatabase/jet_veto_maps/Summer19UL18_V1/hotjets-UL18.root
  TFile *fjv(0);
  if (TString(ds.c_str()).Contains("2016"))
    fjv = new TFile("files/hotjets-UL16.root","READ");
  if (TString(ds.c_str()).Contains("2017"))
    fjv = new TFile("files/hotjets-UL17_v2.root","READ");
  if (TString(ds.c_str()).Contains("2018"))
    fjv = new TFile("files/hotjets-UL18.root","READ");
  assert(fjv);

  // Veto lists for different years (NB: extra MC map for UL16):
  // h2hot_ul16_plus_hbm2_hbp12_qie11 + h2hot_mc (for UL16)
  // h2hot_ul17_plus_hep17_plus_hbpw89 (UL17)
  // h2hot_ul18_plus_hem1516_and_hbp2m1 (UL18)
  TH2D *h2jv = 0;
  if (TString(ds.c_str()).Contains("2016")) {
    h2jv = (TH2D*)fjv->Get("h2hot_ul16_plus_hbm2_hbp12_qie11");
    assert(h2jv);
    TH2D *h2mc = (TH2D*)fjv->Get("h2hot_mc");
    assert(h2mc);
    h2jv->Add(h2mc);
  }
  if (TString(ds.c_str()).Contains("2017"))
    h2jv = (TH2D*)fjv->Get("h2hot_ul17_plus_hep17_plus_hbpw89");
  if (TString(ds.c_str()).Contains("2018"))
    h2jv = (TH2D*)fjv->Get("h2hot_ul18_plus_hem1516_and_hbp2m1");
  assert(h2jv);

  // Setup B and C tagging thresholds according to Z+jet settings (Sami)
  double bthr(0.7527), cthr(0.3985), frac(0.5);
  if (TString(ds.c_str()).Contains("2016")) {
    //btagDeepB->set("2016",0.2217,0.6321,0.8953);
    //btagDeepC->set("2016",-0.48,-0.1,-0.1+frac*(0.69+0.1));
    //bthr = 0.8953; // tight
    bthr = 0.6321; // medium
    cthr = -0.1+frac*(0.69+0.1);
  }
  if (TString(ds.c_str()).Contains("2017")) {
    //btagDeepB->set("2017",0.1522,0.4941,0.8001);
    //btagDeepC->set("2017",0.05,0.15,0.15+frac*(0.8-0.15));
    //bthr = 0.8001; // tight
    bthr = 0.4941; // medium
    cthr = 0.15+frac*(0.8-0.15);
  }
  if (TString(ds.c_str()).Contains("2018")) {
    //btagDeepB->set("2018",0.1241,0.4184,0.7527);
    //btagDeepC->set("2018",0.04,0.137,0.137+frac*(0.66-0.137));
    //bthr = 0.7527; // tight
    bthr = 0.4184; // medium
    cthr = 0.137+frac*(0.66-0.137);
  }

  // Create histograms. Copy format from existing files from Lyon
  // Keep only histograms actually used by global fit (reprocess.C)
  TDirectory *curdir = gDirectory;
  TFile *fout = new TFile(Form("files/GamHistosFill_%s_%s.root",
			       isMC ? "mc" : "data", dataset.c_str()),
			  "RECREATE");
  assert(fout && !fout->IsZombie());
  
  // Original gamma+jet binning
  //     old bin trigger edges  (20,30,60,85,*95*,105,130,230)
  double vx[] = {15, 20, 25, 30, 35, 40, 50, 60, 70, 85, 105, 130, 175, 230,
  		 300, 400, 500, 600, 700, 850, 1000, 1200, 1450, 1750};
  const int nx = sizeof(vx)/sizeof(vx[0])-1;
  
  // L2L3Res eta binning
  double vy[] = {-5.191, -3.839, -3.489, -3.139, -2.964, -2.853, -2.650,
		 -2.500, -2.322, -2.172, -1.930, -1.653, -1.479, -1.305,
		 -1.044, -0.783, -0.522, -0.261, 0.000, 0.261, 0.522, 0.783,
		 1.044, 1.305, 1.479, 1.653, 1.930, 2.172, 2.322, 2.500,
		 2.650, 2.853, 2.964, 3.139, 3.489, 3.839, 5.191};
  const int ny = sizeof(vy)/sizeof(vy[0])-1;

  string dir = (isMC ? "MC" : "DATA");
  
  vector<pair<double,double> > etas;
  etas.push_back(make_pair<double,double>(0,1.305));
  etas.push_back(make_pair<double,double>(0,2.500));
  
  vector<double> alphas;
  alphas.push_back(1.00); 
  alphas.push_back(0.30);
  alphas.push_back(0.20); 
  alphas.push_back(0.15);
  alphas.push_back(0.10);
  alphas.push_back(0.50); 

  // PF composition plots stored in a separate directory
  fout->mkdir("pf");
  fout->cd("pf");  
  
  TH2D *h2pteta = new TH2D("h2pteta","",nx,vx,ny,vy);
  TProfile *pabseta = new TProfile("pabseta","",nx,vx);

  // 1D composition and response
  TProfile *pdb = new TProfile("pdb","",nx,vx);
  TProfile *pmpf = new TProfile("pmpf","",nx,vx);
  TProfile *pchf = new TProfile("pchf","",nx,vx);
  TProfile *pnhf = new TProfile("pnhf","",nx,vx);
  TProfile *pnef = new TProfile("pnef","",nx,vx);
  TProfile *pcef = new TProfile("pcef","",nx,vx);
  TProfile *pmuf = new TProfile("pmuf","",nx,vx);
  TProfile *ppuf = new TProfile("ppuf","",nx,vx);

  // for (int i = 0; i != 72; ++i) cout << Form("%1.3f, ",-TMath::Pi()+i*TMath::TwoPi()/72.); cout << endl;
  const int nphi = 72;
  const double vphi[nphi+1] = 
    {-3.142, -3.054, -2.967, -2.880, -2.793, -2.705, -2.618, -2.531, -2.443,
     -2.356, -2.269, -2.182, -2.094, -2.007, -1.920, -1.833, -1.745, -1.658,
     -1.571, -1.484, -1.396, -1.309, -1.222, -1.134, -1.047, -0.960, -0.873,
     -0.785, -0.698, -0.611, -0.524, -0.436, -0.349, -0.262, -0.175, -0.087,
     0.000, 0.087, 0.175, 0.262, 0.349, 0.436, 0.524, 0.611, 0.698, 0.785,
     0.873, 0.960, 1.047, 1.134, 1.222, 1.309, 1.396, 1.484, 1.571, 1.658,
     1.745, 1.833, 1.920, 2.007, 2.094, 2.182, 2.269, 2.356, 2.443, 2.531,
     2.618, 2.705, 2.793, 2.880, 2.967, 3.054,3.142};

  // 2D composition and response
  TProfile2D *p2db = new TProfile2D("p2db","",ny,vy,nphi,vphi);
  TProfile2D *p2mpf = new TProfile2D("p2mpf","",ny,vy,nphi,vphi);
  TProfile2D *p2chf = new TProfile2D("p2chf","",ny,vy,nphi,vphi);
  TProfile2D *p2nhf = new TProfile2D("p2nhf","",ny,vy,nphi,vphi);
  TProfile2D *p2nef = new TProfile2D("p2nef","",ny,vy,nphi,vphi);
  TProfile2D *p2cef = new TProfile2D("p2cef","",ny,vy,nphi,vphi);
  TProfile2D *p2muf = new TProfile2D("p2muf","",ny,vy,nphi,vphi);
  TProfile2D *p2puf = new TProfile2D("p2puf","",ny,vy,nphi,vphi);

  // Control plots stored in a separate directory
  fout->mkdir("control");
  fout->cd("control");
  
  // 1D plots for mu per trigger
  //hmusmc,20,30,50,75,90,100,110,200
  TH1D *hmus  = new TH1D("hmus","",100,0,100);
  TH1D *hmusmc = new TH1D("hmusmc","",100,0,100);
  TH1D *hmus20 = new TH1D("hmus20","",100,0,100);
  TH1D *hmus30 = new TH1D("hmus30","",100,0,100);
  TH1D *hmus50 = new TH1D("hmus50","",100,0,100);
  TH1D *hmus75 = new TH1D("hmus75","",100,0,100);
  TH1D *hmus90 = new TH1D("hmus90","",100,0,100);
  TH1D *hmus100 = new TH1D("hmus100","",100,0,100);
  TH1D *hmus110 = new TH1D("hmus110","",100,0,100);
  TH1D *hmus200 = new TH1D("hmus200","",100,0,100);

  // 2D plots for mu vs photon pT
  TH2D *h2mus = new TH2D("h2mus","",nx,vx,100,0,100);

  // Plots of npvgood, npvall vs mu
  TProfile *pmuvsmu = new TProfile("pmuvsmu","",100,0,100);
  TProfile *prhovsmu = new TProfile("prhovsmu","",100,0,100);
  TProfile *pnpvgoodvsmu = new TProfile("pnpvgoodvsmu","",100,0,100);
  TProfile *pnpvallvsmu = new TProfile("pnpvallvsmu","",100,0,100);
  // Plots of photon corr, err, hoe, r9, vs mu 
  TProfile *pgainvsmu = new TProfile("pgainvsmu","",100,0,100);
  TProfile *pcorrvsmu = new TProfile("pcorrvsmu","",100,0,100);
  TProfile *perrvsmu = new TProfile("perrvsmu","",100,0,100);
  TProfile *phoevsmu = new TProfile("phoevsmu","",100,0,100);
  TProfile *pr9vsmu = new TProfile("pr9vsmu","",100,0,100);
  // ...and vs pT
  TProfile *pmuvspt = new TProfile("pmuvspt","",nx,vx);
  TProfile *prhovspt = new TProfile("prhovspt","",nx,vx);
  TProfile *pnpvgoodvspt = new TProfile("pnpvgoodvspt","",nx,vx);
  TProfile *pnpvallvspt = new TProfile("pnpvallvspt","",nx,vx);
  // ..and vs pT
  TProfile *pgain1vspt = new TProfile("pgain1vspt","",nx,vx);
  TProfile *pgain6vspt = new TProfile("pgain6vspt","",nx,vx);
  TProfile *pgain12vspt = new TProfile("pgain12vspt","",nx,vx);
  TProfile *pgainvspt = new TProfile("pgainvspt","",nx,vx);
  TProfile *pcorrvspt = new TProfile("pcorrvspt","",nx,vx);
  TProfile *perrvspt = new TProfile("perrvspt","",nx,vx);
  TH2D *h2hoevspt = new TH2D("h2hoevspt","",nx,vx,125,0,0.025);
  TProfile *phoevspt = new TProfile("phoevspt","",nx,vx);
  TH2D *h2r9vspt = new TH2D("h2r9vspt","",nx,vx,150,0.90,1.05);
  TProfile *pr9vspt = new TProfile("pr9vspt","",nx,vx);

  // 2D plots for jet response
  TH2D *h2bal = new TH2D("h2bal","",nx,vx,200,0,4);
  TH2D *h2mpf = new TH2D("h2mpf","",nx,vx,300,-2,4);
  TH2D *h2balc = new TH2D("h2balc","",nx,vx,200,0,4);
  TH2D *h2mpfc = new TH2D("h2mpfc","",nx,vx,300,-2,4);
  TH2D *h2balc2 = new TH2D("h2balc2","",nx,vx,200,0,4);
  TH2D *h2mpfc2 = new TH2D("h2mpfc2","",nx,vx,300,-2,4);
  
  // 2D and profile for photon-jet pT
  TH2D *h2phoj = new TH2D("h2phoj","",nx,vx,240,-0.1,0.5);
  TProfile *pphoj = new TProfile("pphoj","",nx,vx);

  // Extras without zero suppression and for gain paths
  TH2D *h2phoj0 = new TH2D("h2phoj0","",nx,vx,140,-0.2,0.5);
  TProfile *pphoj0 = new TProfile("pphoj0","",nx,vx);
  TH2D *h2phoj1 = new TH2D("h2phoj1","",nx,vx,140,-0.2,0.5);
  TProfile *pphoj1 = new TProfile("pphoj1","",nx,vx);
  TH2D *h2phoj6 = new TH2D("h2phoj6","",nx,vx,140,-0.2,0.5);
  TProfile *pphoj6 = new TProfile("pphoj6","",nx,vx);
  TH2D *h2phoj12 = new TH2D("h2phoj12","",nx,vx,140,-0.2,0.5);
  TProfile *pphoj12 = new TProfile("pphoj12","",nx,vx);

  // Plots for photon properties (more in MC)
  TH2D *h2gametaphi = new TH2D("h2gametaphi","",30,-1.305,+1.305,
			       72,-TMath::Pi(),TMath::Pi());
  TH2D *h2gametaphi2 = new TH2D("h2gametaphi2","",150,-1.305,+1.305,
				360,-TMath::Pi(),TMath::Pi());
  TH2D *h2gametaphi3 = new TH2D("h2gametaphi3","",150,-1.305,+1.305,
				720,-TMath::Pi(),TMath::Pi());
  TH2D *h2gametaphi4 = new TH2D("h2gametaphi4","",150,-1.305,+1.305,
				1440,-TMath::Pi(),TMath::Pi());
  TH2D *h2ngam = new TH2D("h2ngam","",nx,vx,5,0,5);
  TH1D *hgen = new TH1D("hgen","",nx,vx);
  TH1D *hgam = new TH1D("hgam","",nx,vx);
  TH1D *hgamtrg = new TH1D("hgamtrg","",nx,vx);
  TProfile *peffgr = new TProfile("peffgr","",nx,vx);
  TProfile *peffid = new TProfile("peffid","",nx,vx);
  TProfile *pfake = new TProfile("pfake","",nx,vx);
  TProfile *pfakeqcd = new TProfile("pfakeqcd","",nx,vx); // for QCD bkg
  TProfile *pfakeqcd2 = new TProfile("pfakeqcd2","",nx,vx); // for QCD bkg
  TH2D *h2rgam = new TH2D("h2rgam","",nx,vx,350,0.80,1.15);
  TH2D *h2rgamqcd = new TH2D("h2rgamqcd","",nx,vx,350,0.80,1.15); // for QCD bkg
  TProfile *prgam = new TProfile("prgam","",nx,vx);
  TProfile *prgamqcd = new TProfile("prgamqcd","",nx,vx); // for QCD bkg
  TProfile *prgamqcd2 = new TProfile("prgamqcd2","",nx,vx); // for QCD bkg
  TH2D *h2cgam = new TH2D("h2cgam","",nx,vx,100,0.90,1.10);
  TProfile *pcgam = new TProfile("pcgam","",nx,vx);

  // Plots for jet properties
  TH2D *h2gjet = new TH2D("h2gjet","",nx,vx,100,0.90,1.10);
  TProfile *pgjet = new TProfile("pgjet","",nx,vx);
  TH2D *h2rjet = new TH2D("h2rjet","",nx,vx,100,0.90,1.10);
  TProfile *prjet = new TProfile("prjet","",nx,vx);
  TH2D *h2rgen = new TH2D("h2rgen","",nx,vx,100,0.90,1.10);
  TProfile *prgen = new TProfile("prgen","",nx,vx);

  TH2D *h2gjet2 = new TH2D("h2gjet2","",nx,vx,100,0.90,1.10);
  TProfile *pgjet2 = new TProfile("pgjet2","",nx,vx);
  TH2D *h2rjet2 = new TH2D("h2rjet2","",nx,vx,100,0.90,1.10);
  TProfile *prjet2 = new TProfile("prjet2","",nx,vx);
  TH2D *h2rgen2 = new TH2D("h2rgen2","",nx,vx,100,0.90,1.10);
  TProfile *prgen2 = new TProfile("prgen2","",nx,vx);

  // Plots for jet flavor
  /*
  TProfile *prgenud = new TProfile("prgenud","",nx,vx);
  TProfile *prgens = new TProfile("prgens","",nx,vx);
  TProfile *prgenc = new TProfile("prgenc","",nx,vx);
  TProfile *prgenb = new TProfile("prgenb","",nx,vx);
  TProfile *prgeng = new TProfile("prgeng","",nx,vx);
  TProfile *prgeno = new TProfile("prgeno","",nx,vx);
  TProfile *pgjetud = new TProfile("pgjetud","",nx,vx);
  TProfile *pgjets = new TProfile("pgjets","",nx,vx);
  TProfile *pgjetc = new TProfile("pgjetc","",nx,vx);
  TProfile *pgjetb = new TProfile("pgjetb","",nx,vx);
  TProfile *pgjetg = new TProfile("pgjetg","",nx,vx);
  TProfile *pgjeto = new TProfile("pgjeto","",nx,vx);
  TProfile *pfud = new TProfile("pfud","",nx,vx);
  TProfile *pfs = new TProfile("pfs","",nx,vx);
  TProfile *pfc = new TProfile("pfc","",nx,vx);
  TProfile *pfb = new TProfile("pfb","",nx,vx);
  TProfile *pfg = new TProfile("pfg","",nx,vx);
  TProfile *pfo = new TProfile("pfo","",nx,vx);
  */

  // Plots for photon response in data
  TProfile *prbal = new TProfile("prbal","",nx,vx);
  TProfile *prmpf = new TProfile("prmpf","",nx,vx);
  TProfile *prbal0 = new TProfile("prbal0","",980,20,1000);
  TProfile *prmpf0 = new TProfile("prmpf0","",980,20,1000);
  TProfile *prbal1 = new TProfile("prbal1","",nx,vx);
  TProfile *prmpf1 = new TProfile("prmpf1","",nx,vx);
  TProfile *prbal6 = new TProfile("prbal6","",nx,vx);
  TProfile *prmpf6 = new TProfile("prmpf6","",nx,vx);
  TProfile *prbal12 = new TProfile("prbal12","",nx,vx);
  TProfile *prmpf12 = new TProfile("prmpf12","",nx,vx);
  
  // Plots for photon trigger efficiencies
  // TBD: need to create these more systematically with a loop
  TH1D *hgam0_data = new TH1D("hgam0_data","",197,15,1000);
  TH1D *hgam0_mc = new TH1D("hgam0_mc","",197,15,1000);
  TH1D *hgam0 = new TH1D("hgam0","",197,15,1000);
  TH1D *hgam20 = new TH1D("hgam20","",197,15,1000);
  TH1D *hgam22 = new TH1D("hgam22","",197,15,1000);
  TH1D *hgam30 = new TH1D("hgam30","",197,15,1000);
  TH1D *hgam33 = new TH1D("hgam33","",197,15,1000);
  TH1D *hgam36 = new TH1D("hgam36","",197,15,1000);
  TH1D *hgam50 = new TH1D("hgam50","",197,15,1000);
  TH1D *hgam75 = new TH1D("hgam75","",197,15,1000);
  TH1D *hgam90 = new TH1D("hgam90","",197,15,1000);
  TH1D *hgam120 = new TH1D("hgam120","",197,15,1000);
  TH1D *hgam150 = new TH1D("hgam150","",197,15,1000);
  TH1D *hgam175 = new TH1D("hgam175","",197,15,1000);
  TH1D *hgam200 = new TH1D("hgam200","",197,15,1000);
  TH1D *hgam300 = new TH1D("hgam300","",197,15,1000);
  TH1D *hgam100t = new TH1D("hgam100t","",197,15,1000);
  TH1D *hgam110t = new TH1D("hgam110t","",197,15,1000);
  TH1D *hgam120t = new TH1D("hgam120t","",197,15,1000);
  TH1D *hgam22m = new TH1D("hgam22m","",197,15,1000);
  TH1D *hgam30m = new TH1D("hgam30m","",197,15,1000);
  TH1D *hgam36m = new TH1D("hgam36m","",197,15,1000);
  TH1D *hgam50m = new TH1D("hgam50m","",197,15,1000);
  TH1D *hgam75m = new TH1D("hgam75m","",197,15,1000);
  TH1D *hgam90m = new TH1D("hgam90m","",197,15,1000);
  TH1D *hgam120m = new TH1D("hgam120m","",197,15,1000);
  TH1D *hgam165m = new TH1D("hgam165m","",197,15,1000);
  TH1D *hgam165h = new TH1D("hgam165h","",197,15,1000);
  TH1D *hgam20l = new TH1D("hgam20l","",197,15,1000);
  TH1D *hgam30l = new TH1D("hgam30l","",197,15,1000);
  TH1D *hgam40l = new TH1D("hgam40l","",197,15,1000);
  TH1D *hgam50l = new TH1D("hgam50l","",197,15,1000);
  TH1D *hgam60l = new TH1D("hgam60l","",197,15,1000);
  TH1D *hgamtrig = new TH1D("hgamtrig","",197,15,1000);
  TH1D *hgamtrig_data = new TH1D("hgamtrig_data","",197,15,1000);
  TH1D *hgamtrig_mc = new TH1D("hgamtrig_mc","",197,15,1000);

  // Flavor plots stored in a separate directory
  fout->mkdir("flavor");
  fout->cd("flavor");

  map<string, double> mvar;
  map<string, map<string, map<string, TH1*> > > mp;
  string avar[] = {"counts","mpfchs1","ptchs","mpf1","mpfn","mpfu","rho",
		   "rjet","gjet","rgen"};
  string atag[] = {"i","b","c","q","g","n"};
  //string aflv[] = {"i","b","c","q","g","n"};
  string aflv[] = {"i","b","c","q","s","ud","g","n"};
  const int nvar = sizeof(avar)/sizeof(avar[0]);
  const int ntag = sizeof(atag)/sizeof(atag[0]);
  const int nflv = sizeof(aflv)/sizeof(aflv[0]);
  
  cout << "Creating flavor histograms/profiles in folder flavor/..." << endl;
  for (int ivar = 0; ivar != nvar; ++ivar) {
    for (int itag = 0; itag != ntag; ++itag) {
      for (int iflv = 0; iflv != nflv; ++iflv) {
	string& var = avar[ivar]; const char *cv = var.c_str();
	string& tag = atag[itag]; const char *ct = tag.c_str();
	string& flv = aflv[iflv]; const char *cf = flv.c_str();
	if (var=="counts")
	  mp[var][tag][flv] = new TH1D(Form("%s_g%s%s",cv,ct,cf),"",nx,vx);
	else
	  mp[var][tag][flv] = new TProfile(Form("%s_g%s%s",cv,ct,cf),"",nx,vx);
      } // for iflv
    } // for itag
  } // for ivar

  fout->cd();
  
  // Loop to create histograms and profiles
  // Match ordering to Lyon files (alpha->eta->data/MC) when creating
  // Although otherwise ordering is data/MC->eta->alpha
  // Add PS weight variations
  unsigned int nps = (isMC ? nPSWeightMax+1 : 1);
  map<int, map<int, map<int, BasicHistos*> > > mBasicHistos;
  for (unsigned int ialpha = 0; ialpha != alphas.size(); ++ialpha) {
    for (unsigned int ieta = 0; ieta != etas.size(); ++ieta) { 
    for (unsigned int ips = 0; ips != nps; ++ips) {

      // Select data/MC, alpha and eta bin
      const char *cd = dir.c_str();
      int ia = int(100*alphas[ialpha]);
      int iy1 = int(10*etas[ieta].first);
      int iy2 = int(10*etas[ieta].second);
      int iy = 100*int(iy1) + int(iy2);
      const char *cps = (ips==0 ? "" : Form("_ps%d",ips-1));
      
      // Counts
      TH1D *hn = new TH1D(Form("resp_MPFchs_%s_a%d_eta%02d_%02d_RawNEvents"
			       "_data_vs_pt%s",cd,ia,iy1,iy2,cps),"",nx,vx);
      TH1D *hxsec = new TH1D(Form("resp_MPFchs_%s_a%d_eta%02d_%02d_Xsec"
				  "_data_vs_pt%s",cd,ia,iy1,iy2,cps),"",nx,vx);
      
      // Response profiles
      string name = Form("resp_%%s_%s_a%d_eta%02d_%02d%s",cd,ia,iy1,iy2,cps);
      const char *cname = name.c_str();
      TProfile *prpt  =  new TProfile(Form(cname,"PtGam"),"",nx,vx);
      TProfile *prbal =  new TProfile(Form(cname,"PtBalchs"),"",nx,vx);
      TProfile *prdb =  new TProfile(Form(cname,"DBchs"),"",nx,vx);
      TProfile *prmpf =  new TProfile(Form(cname,"MPFchs"),"",nx,vx);
      TProfile *prmpf1 = new TProfile(Form(cname,"MPFR1chs"),"",nx,vx);
      TProfile *prmpfn = new TProfile(Form(cname,"MPFRnchs"),"",nx,vx);
      TProfile *prmpfu = new TProfile(Form(cname,"MpfRuchs"),"",nx,vx);
      TProfile *prho = new TProfile(Form(cname,"Rho_CHS"),"",nx,vx);

      // Store links to histograms and profiles into maps
      BasicHistos *pmh = new BasicHistos();
      BasicHistos& mh = (*pmh);
      mh.hn = hn;
      mh.hxsec = hxsec;
      mh.prpt = prpt;
      mh.prbal = prbal;
      mh.prdb = prdb;
      mh.prmpf = prmpf;
      mh.prmpf1 = prmpf1;
      mh.prmpfn = prmpfn;
      mh.prmpfu = prmpfu;
      mh.prho = prho;
      mBasicHistos[iy][ia][ips] = pmh;
    } // for ips in PSWeight
    } // for ieta in etas
  } // for ialpha in alphas
  
  curdir->cd();
  
  TLorentzVector gam, gami, lhe, gen, phoj, phoj0, phoj0off, jet, jet2, jetn;
  TLorentzVector gamorig; // for QCD bkg
  TLorentzVector met, met1, metn, metu, rawmet, corrmet, rawgam;
  TLorentzVector jeti, corrjets, rawjet, rawjets, rcjet, rcjets, rcoffsets;
  TLorentzVector geni, genjet, genjet2;
  TLorentzVector fox; // for isQCD
  
  //Long64_t nentries = fChain->GetEntriesFast();
  Long64_t nentries = fChain->GetEntries();
  cout << "\nStarting loop over " << dataset << " with "
       << nentries << " entries" << endl;

  //int skip = 21700000; // 2018A first events without 110EB
  //int skip = 55342793; // 2018A first events with 92 photon
  //int skip = 265126992; // 2018A first events with 191 photons, 23 jets
  //int skip = 14648973; // 2017C bad HDM

  Long64_t nbytes = 0, nb = 0;
  for (Long64_t jentry=0; jentry<nentries;jentry++) {

    // Skip events, typically for debugging purposes
    //if (jentry<skip) continue;
    //if (_gh_debug && jentry%10000==0) cout << "," << endl << flush;
    
    Long64_t ientry = LoadTree(jentry);
    if (ientry < 0) break;
    
    ++_ntot;
    if (jentry==100000 || jentry==1000000 || jentry==1000000 ||
	(jentry%1000000==0 && jentry<10000000) ||
	(jentry%10000000==0 && jentry!=0) ||
	jentry==nentries-1) {
      if (jentry==0) { laptime.Start(); }
      if (nentries!=0) {
	cout << Form("\nProcessed %lld events (%1.1f%%) in %1.0f sec. "
		     "(%1.0f sec. for last %d)",
		     jentry, 100.*jentry/nentries, fulltime.RealTime(),
		     laptime.RealTime(), nlap);
      }
      if (jentry!=0 && nlap!=0) {
	cout << Form("\nEstimated runtime:  %1.0f sec. "
		     " (%1.0f sec. for last %d)",
		     1.*nentries/jentry*fulltime.RealTime(),
		     1.*nentries/nlap*laptime.RealTime(),nlap) << flush;
	laptime.Reset();
	nlap = 0;
      }
      if (jentry==0) fulltime.Reset(); // Leave out initialization time
      fulltime.Continue();
      laptime.Continue();
    }
    if (jentry%10000==0) cout << "." << flush;
    ++nlap;

    // Safety resets for tight triggers that are partly missing in 2018A, 2016
    HLT_Photon100EB_TightID_TightIso =  HLT_Photon110EB_TightID_TightIso =
      HLT_Photon120EB_TightID_TightIso = kFALSE;
    // Safety resets for triggers that are partly missing in 2016, or 2017-18
    HLT_Photon200 = HLT_Photon175 = HLT_Photon150 = HLT_Photon90 =
      HLT_Photon75 = HLT_Photon50 = HLT_Photon36 = HLT_Photon33 =
      HLT_Photon30 = HLT_Photon22 = HLT_Photon20 = kFALSE;
    // Safety resets for medium triggers
    HLT_Photon165_R9Id90_HE10_IsoM = HLT_Photon120_R9Id90_HE10_IsoM = 
      HLT_Photon90_R9Id90_HE10_IsoM = HLT_Photon75_R9Id90_HE10_IsoM = 
      HLT_Photon50_R9Id90_HE10_IsoM = HLT_Photon36_R9Id90_HE10_IsoM = 
      HLT_Photon30_R9Id90_HE10_IsoM = HLT_Photon22_R9Id90_HE10_IsoM = kFALSE;
    // Safety resets for loose triggers (only 2017-18)
    HLT_Photon60_HoverELoose = HLT_Photon50_HoverELoose =
      HLT_Photon40_HoverELoose =  HLT_Photon30_HoverELoose =
      HLT_Photon20_HoverELoose = kFALSE;
    HLT_Photon165_HE10 = kFALSE;

    if (!isMC) { // Fast trigger filtering (useful for data)

      if (b_HLT_Photon200 && !is16) // not in 2016
	b_HLT_Photon200->GetEntry(ientry);
      if (b_HLT_Photon175)
	b_HLT_Photon175->GetEntry(ientry);
      if (b_HLT_Photon150 && !is16) // not in 2016
	b_HLT_Photon150->GetEntry(ientry);
      if (b_HLT_Photon120)
	b_HLT_Photon120->GetEntry(ientry);
      if (b_HLT_Photon90)
	b_HLT_Photon90->GetEntry(ientry);
      if (b_HLT_Photon75)
	b_HLT_Photon75->GetEntry(ientry);
      if (b_HLT_Photon50)
	b_HLT_Photon50->GetEntry(ientry);
      if (b_HLT_Photon36 && is16) // 2016
	b_HLT_Photon36->GetEntry(ientry);
      if (b_HLT_Photon30 && is16) // 2016
	b_HLT_Photon30->GetEntry(ientry);
      if (b_HLT_Photon33 && !is16) // 2016
	b_HLT_Photon33->GetEntry(ientry);
      if (b_HLT_Photon22 && is16) // 2016
	b_HLT_Photon22->GetEntry(ientry);
      if (b_HLT_Photon20 && is18) // 2018
	b_HLT_Photon20->GetEntry(ientry);

      // Only in (most of) 2018
      if (b_HLT_Photon120EB_TightID_TightIso && is18) // not in 2016-17, 2018A
	b_HLT_Photon120EB_TightID_TightIso->GetEntry(ientry);
      if (b_HLT_Photon110EB_TightID_TightIso && is18) // not in 2016-17, 2018A
	b_HLT_Photon110EB_TightID_TightIso->GetEntry(ientry);
      if (b_HLT_Photon100EB_TightID_TightIso && is18) // not in 2016-17, 2018A
	b_HLT_Photon100EB_TightID_TightIso->GetEntry(ientry);

      // Only 2016
      if (b_HLT_Photon22_R9Id90_HE10_IsoM && is16) // only in 2016
	b_HLT_Photon22_R9Id90_HE10_IsoM->GetEntry(ientry);
      if (b_HLT_Photon30_R9Id90_HE10_IsoM && is16) // only in 2016
	b_HLT_Photon30_R9Id90_HE10_IsoM->GetEntry(ientry);
      if (b_HLT_Photon36_R9Id90_HE10_IsoM && is16) // only in 2016
	b_HLT_Photon36_R9Id90_HE10_IsoM->GetEntry(ientry);
      if (b_HLT_Photon165_HE10 && is16)
	b_HLT_Photon165_HE10->GetEntry(ientry);

      // In all years
      if (b_HLT_Photon165_R9Id90_HE10_IsoM)
	b_HLT_Photon165_R9Id90_HE10_IsoM->GetEntry(ientry);
      if (b_HLT_Photon120_R9Id90_HE10_IsoM)
	b_HLT_Photon120_R9Id90_HE10_IsoM->GetEntry(ientry);
      if (b_HLT_Photon90_R9Id90_HE10_IsoM)
	b_HLT_Photon90_R9Id90_HE10_IsoM->GetEntry(ientry);
      if (b_HLT_Photon75_R9Id90_HE10_IsoM)
	b_HLT_Photon75_R9Id90_HE10_IsoM->GetEntry(ientry);
      if (b_HLT_Photon50_R9Id90_HE10_IsoM)
	b_HLT_Photon50_R9Id90_HE10_IsoM->GetEntry(ientry);
							
      // Only in 2017
      if (b_HLT_Photon60_HoverELoose && is17) // only in 2017
	b_HLT_Photon60_HoverELoose->GetEntry(ientry);
      if (b_HLT_Photon50_HoverELoose && is17) // only in 2017
	b_HLT_Photon50_HoverELoose->GetEntry(ientry);
      if (b_HLT_Photon40_HoverELoose && is17) // only in 2017
	b_HLT_Photon40_HoverELoose->GetEntry(ientry);

      // 2017 and 2018
      if (b_HLT_Photon30_HoverELoose && !is16) // not in 2016
	b_HLT_Photon30_HoverELoose->GetEntry(ientry);
      if (b_HLT_Photon20_HoverELoose && !is16) // not in 2016
	b_HLT_Photon20_HoverELoose->GetEntry(ientry);
      
      if ((is18 &&
	   !(HLT_Photon200 ||
	     HLT_Photon175 || 
	     HLT_Photon150 || 
	     HLT_Photon120 || 
	     HLT_Photon90 || 
	     HLT_Photon75 || 
	     HLT_Photon50 || 
	     HLT_Photon33 || 
	     HLT_Photon20 ||
	     HLT_Photon120EB_TightID_TightIso ||
	     HLT_Photon110EB_TightID_TightIso ||
	     HLT_Photon100EB_TightID_TightIso ||
	     HLT_Photon90_R9Id90_HE10_IsoM ||
	     HLT_Photon75_R9Id90_HE10_IsoM ||
	     HLT_Photon50_R9Id90_HE10_IsoM ||
	     HLT_Photon30_HoverELoose ||
	     HLT_Photon20_HoverELoose)) ||
	   (is17 &&
	   !(HLT_Photon200 ||
	     HLT_Photon175 || 
	     HLT_Photon150 || 
	     HLT_Photon120 || 
	     HLT_Photon90 || 
	     HLT_Photon75 || 
	     HLT_Photon50 || 
	     HLT_Photon33 || 
	     HLT_Photon165_R9Id90_HE10_IsoM ||
	     HLT_Photon120_R9Id90_HE10_IsoM ||
	     HLT_Photon90_R9Id90_HE10_IsoM ||
	     HLT_Photon75_R9Id90_HE10_IsoM ||
	     HLT_Photon50_R9Id90_HE10_IsoM ||
	     HLT_Photon60_HoverELoose ||
	     HLT_Photon50_HoverELoose ||
	     HLT_Photon40_HoverELoose ||
	     HLT_Photon30_HoverELoose ||
	     HLT_Photon20_HoverELoose)) ||
	  (is16 &&
	   !(HLT_Photon175 || 
	     HLT_Photon120 || 
	     HLT_Photon90 || 
	     HLT_Photon75 || 
	     HLT_Photon50 || 
	     HLT_Photon36 || 
	     HLT_Photon30 || 
	     HLT_Photon22 || 
	     //HLT_Photon165_HE10 ||
	     HLT_Photon165_R9Id90_HE10_IsoM ||
	     HLT_Photon120_R9Id90_HE10_IsoM ||
	     HLT_Photon90_R9Id90_HE10_IsoM ||
	     HLT_Photon75_R9Id90_HE10_IsoM ||
	     HLT_Photon50_R9Id90_HE10_IsoM ||
	     HLT_Photon36_R9Id90_HE10_IsoM ||
	     HLT_Photon30_R9Id90_HE10_IsoM ||
	     HLT_Photon22_R9Id90_HE10_IsoM))) {
	++_nbadevents_trigger;
	continue;
      }
    } // !isMC

    nb = fChain->GetEntry(jentry);   nbytes += nb;
    // if (Cut(ientry) < 0) continue;

    // Sanity check PS weights
    if (!isMC) { nPSWeight = 0; }
    assert(nPSWeight<=nPSWeightMax);

    // Does the run/LS pass the latest JSON selection?
    if (!isMC && _json[run][luminosityBlock]==0) {
      //_badjson.insert(pair<int, int>(run, lbn));
      ++_nbadevents_json;
      continue;
    }
    else 
      ++_nevents;


    // Safety checks for array sizes (segfault in 2018A)
    if (nJet > nJetMax || nPhoton > nPhotonMax) {
      cout << "Error: nJet="<<nJet<<" or nPhoton="<<nPhoton
	   << " exceeding maximum. Skip event " << jentry << endl << flush;
      exit(0);
    }
    
    // Re-JEC + re-MET, if needed
    // Skipped for now      
    
    // Select leading photon. Use tight cut-based ID and PF relative isolation
    // Temporary: select photon based on LHE photon match
    int iGamGen(-1), iGam(-1), nGam(0);
    int iGamOrig(-1); // for QCD bkg
    gen.SetPtEtaPhiM(0,0,0,0);
    gam.SetPtEtaPhiM(0,0,0,0);
    phoj.SetPtEtaPhiM(0,0,0,0);

    // Gen-photon
    if (isMC && nGenIsolatedPhoton>0) {
      gen.SetPtEtaPhiM(GenIsolatedPhoton_pt[0],GenIsolatedPhoton_eta[0],
		       GenIsolatedPhoton_phi[0],GenIsolatedPhoton_mass[0]);
    }

    // Select tight photons and photon matching gen photon
    for (unsigned int i = 0; i != nPhoton; ++i) {
      
      gami.SetPtEtaPhiM(Photon_pt[i],  Photon_eta[i],
			Photon_phi[i], Photon_mass[i]);
      
      // Photon matching gen photon
      if (gen.Pt()>0 && gen.DeltaR(gami)<0.2 && iGamGen==-1) {
	iGamGen = i;
      } 
      
      // Leading tight photon(s)
      // R9>0.94 to avoid bias wrt R9Id90 triggers and from photon conversions
      if (Photon_pt[i]>15 && Photon_cutBased[i]==3 && Photon_hoe[i]<0.02148 &&
	  Photon_r9[i]>0.94) {
	++nGam;
	if (iGam==-1) {
	  iGam = i;
	  gam = gami;
	}
      } // tight photon
    } // for i in nPhoton

    // Correct photon for gain1 and MPF for "footprint" (photon vs PFgamma)
    rawgam = gam;
    if (iGam!=-1 && Photon_seedGain[iGam]==1 && !isMC) gam *= 1./1.01;
    if (iGam!=-1) {
      // [0]+log(x)*([1]+log(x)*[2]) in range [15,1750] to MC pphoj0
      //1  p0           4.57516e-02   3.91871e-04   1.09043e-07   4.17033e-05
      //2  p1          -1.27462e-02   1.50968e-04   2.08432e-08   3.92715e-03
      //3  p2           1.07760e-03   1.45293e-05   3.93020e-09   1.65460e-01
      double x = max(60.,rawgam.Pt());
      double f = 4.57516e-02 + log(x) * ( -1.27462e-02 + log(x) * 1.07760e-03);
      rawgam *= (1+f);
    }
   
    // Photon-jet: uncorrected jet minus (uncorr.) photon minus L1RC
    if (iGam!=-1 && Photon_jetIdx[iGam]!=-1) {
      unsigned int idx = Photon_jetIdx[iGam]; assert(idx<nJet);
      phoj.SetPtEtaPhiM(Jet_pt[idx], Jet_eta[idx], Jet_phi[idx], Jet_mass[idx]);
      phoj *= (1-Jet_rawFactor[idx]);
      phoj -= rawgam;
      phoj0 = phoj;

      // Calculate L1RC correction
      jecl1rc->setJetPt(phoj.Pt());
      jecl1rc->setJetEta(phoj.Eta());
      jecl1rc->setJetA(Jet_area[idx]);
      jecl1rc->setRho(fixedGridRhoFastjetAll);
      double corrl1rc = jecl1rc->getCorrection();
      phoj *= corrl1rc;

      // Calculate L1RC correction without "zero suppression"
      double refpt = 30; // phoj.Pt~0 leads to negative offset cutoff
      jecl1rc->setJetPt(refpt);
      jecl1rc->setJetEta(phoj0.Eta());
      jecl1rc->setJetA(Jet_area[idx]);
      jecl1rc->setRho(fixedGridRhoFastjetAll);
      double corrl1rc0 = jecl1rc->getCorrection();
      double off0 = (corrl1rc0 - 1) * refpt; // corr*ptref = (ptref-off)
      phoj0off.SetPtEtaPhiM(off0,phoj0.Eta(),phoj0.Phi(),0.);
      phoj0 -= phoj0off;
    }

    // For QCD background, emulate a photon+jet event by replacing
    // one of the leading jets with genjet taking the place of the photon
    int iFox(-1);
    fox.SetPtEtaPhiM(0,0,0,0);
    //if (isQCD && iGam==-1 && nJet>=2) {
    if (isQCD && nJet>=2) {
      // Save original good photon, if one was found
      iGamOrig = iGam;
      gamorig = gam;

      iFox = (jentry%2); // "random" selection from two leading jets
      // Jet_genJetIdx would be great, but only there for UL18 nAOD? Maybe there
      int k = Jet_genJetIdx[iFox];
      if (k>=0 && k<nGenJet) {
	gam.SetPtEtaPhiM(GenJet_pt[k], GenJet_eta[k], GenJet_phi[k],
			 GenJet_mass[k]);
	// NB: should remove UE clustered into gam. In Minsuk's rho_ZB_new.pdf
	// QCD_CP5 has about 3.5 GeV/A of UE offset at generator level
	double area = Jet_area[iFox];
	gam *= (gam.Pt()>0 ? 1 - 3.5*area/gam.Pt() : 1.);
	rawgam = gam;

	fox.SetPtEtaPhiM(Jet_pt[iFox], Jet_eta[iFox], Jet_phi[iFox],
			 Jet_mass[iFox]);
	fox *= (1-Jet_rawFactor[iFox]);
	// Calculate L1RC correction
	jecl1rc->setJetPt(fox.Pt());
	jecl1rc->setJetEta(fox.Eta());
	jecl1rc->setJetA(Jet_area[iFox]);
	jecl1rc->setRho(fixedGridRhoFastjetAll);
	double corrl1rc = jecl1rc->getCorrection();
	fox *= corrl1rc;
	// NB2: should also remove UE clustered into fox. In Minsuk's plot
	// QCD_CP5 has about 2.5 GeV/A of UE offset at FullSim level
	fox *= (fox.Pt()>0 ? 1 - 2.5*area/fox.Pt() : 1.);

	// NB3: For consistency with gamma+jet, phoj should still have reco UE
	phoj.SetPtEtaPhiM(2.5*area,0,0,0);
	phoj0 = phoj;
	nGam = 1;
      }
    } // isQCD
  
    // Event weights (1 for MadGraph)
    //bool isMC = (run==1);
    assert((isMC && run==1) || (!isMC && run!=1));
    double w = (isMC ? genWeight : 1);
    
    // Pileup
    double TruePUrms(0);
    if (!isMC) Pileup_nTrueInt = getTruePU(run,luminosityBlock,&TruePUrms);
    double ptgam = gam.Pt();

    // Trigger selection. Take care to match pT bin edges
    // {15, 20, 25, 30, 35, 40, 50, 60, 70, 85, 105, 130, 175, 230,
    //  300, 400, 500, 600, 700, 850, 1000, 1200, 1450, 1750};
    // NB: Photon90 threshold could be 95, Photon175 coud be 185, if bins split?
    double pt = ptgam; // shorthand to make trigger selection more readable
    int itrg(0); // choose trigger for PU reweighing as side effect (hack...)
    bool pass_trig = 
      ((is16 && 
	((HLT_Photon175                  && pt>=230            && (itrg=175)) ||
	 //(HLT_Photon165_HE10             && pt>=175 && pt<230) || // not in MC
	 (HLT_Photon165_R9Id90_HE10_IsoM && pt>=175 && pt<230  && (itrg=165)) ||
	 (HLT_Photon120_R9Id90_HE10_IsoM && pt>=130 && pt<175  && (itrg=120)) ||
	 (HLT_Photon90_R9Id90_HE10_IsoM  && pt>=105 && pt<130  && (itrg=90)) ||
	 (HLT_Photon75_R9Id90_HE10_IsoM  && pt>=85  && pt<105  && (itrg=75)) ||
	 (HLT_Photon50_R9Id90_HE10_IsoM  && pt>=60  && pt<85   && (itrg=50)) ||
	 (HLT_Photon36_R9Id90_HE10_IsoM  && pt>=40  && pt<60   && (itrg=36)) ||
	 (HLT_Photon30_R9Id90_HE10_IsoM  && pt>=35  && pt<40   && (itrg=30)) ||
	 (HLT_Photon22_R9Id90_HE10_IsoM  && pt>=20  && pt<35   && (itrg=22)) ||
	 (isMC                           && pt>=40  && pt<60   && (itrg=36)) ||
	 (isMC                           && pt>=35  && pt<40   && (itrg=30)) ||
	 (isMC                           && pt>=20  && pt<35   && (itrg=22))
	 )) ||
       (is17 &&
	((HLT_Photon200                  && pt>=230            && (itrg=200)) ||
	 (HLT_Photon165_R9Id90_HE10_IsoM && pt>=175 && pt<230  && (itrg=165)) ||
	 (HLT_Photon120_R9Id90_HE10_IsoM && pt>=130 && pt<175  && (itrg=120)) ||
	 (HLT_Photon90_R9Id90_HE10_IsoM  && pt>=105 && pt<130  && (itrg=90)) ||
	 (HLT_Photon75_R9Id90_HE10_IsoM  && pt>=85  && pt<105  && (itrg=75)) ||
	 (HLT_Photon50_R9Id90_HE10_IsoM  && pt>=60  && pt<85   && (itrg=50)) ||
	 //(HLT_Photon33                   && pt>=35  && pt<60 ) ||
	 (HLT_Photon30_HoverELoose       && pt>=35  && pt<60   && (itrg=30)) ||
	 (HLT_Photon20_HoverELoose       && pt>=20  && pt<35   && (itrg=20)) ||
	 //(HLT_Photon20                     && pt>=20  && pt<60 )
	 (isMC                           && pt>=35  && pt<60   && (itrg=30)) ||
	 (isMC                           && pt>=20  && pt<35   && (itrg=20))
	 )) ||
       (is18 &&
	((HLT_Photon200                    && pt>=230           && (itrg=200))||
	 (HLT_Photon110EB_TightID_TightIso && pt>=130 && pt<230 && (itrg=110))||
	 (HLT_Photon100EB_TightID_TightIso && pt>=105 && pt<130 && (itrg=100))||
	 (HLT_Photon90_R9Id90_HE10_IsoM    && pt>=95  && pt<105 && (itrg=90)) ||
	 (HLT_Photon75_R9Id90_HE10_IsoM    && pt>=85  && pt<95  && (itrg=75)) ||
	 (HLT_Photon50_R9Id90_HE10_IsoM    && pt>=60  && pt<85  && (itrg=50)) ||
	 //(HLT_Photon33                     && pt>=35  && pt<60 ) ||
	 (HLT_Photon30_HoverELoose         && pt>=35  && pt<60  && (itrg=30)) ||
	 (HLT_Photon20_HoverELoose         && pt>=20  && pt<35  && (itrg=20)) ||
	 //(HLT_Photon20                     && pt>=20  && pt<35 )
	 (isMC                             && pt>=35  && pt<60  && (itrg=30)) ||
	 (isMC                             && pt>=20  && pt<35  && (itrg=20))
	 ))
       );
  
    // Select trigger pT bins by hand for QCD. Error prone...
    if (isQCD && !pass_trig) {
      pass_trig = 
	((is16 && 
	  ((pt>=230            && (itrg=175)) ||
	   (pt>=175 && pt<230  && (itrg=165)) ||
	   (pt>=130 && pt<175  && (itrg=120)) ||
	   (pt>=105 && pt<130  && (itrg=90)) ||
	   (pt>=85  && pt<105  && (itrg=75)) ||
	   (pt>=60  && pt<85   && (itrg=50)) ||
	   (pt>=40  && pt<60   && (itrg=36)) ||
	   (pt>=35  && pt<40   && (itrg=30)) ||
	   (pt>=20  && pt<35   && (itrg=22))
	   )) ||
	 (is17 &&
	  ((pt>=230            && (itrg=200)) ||
	   (pt>=175 && pt<230  && (itrg=165)) ||
	   (pt>=130 && pt<175  && (itrg=120)) ||
	   (pt>=105 && pt<130  && (itrg=90)) ||
	   (pt>=85  && pt<105  && (itrg=75)) ||
	   (pt>=60  && pt<85   && (itrg=50)) ||
	   (pt>=35  && pt<60   && (itrg=30)) ||
	   (pt>=20  && pt<35   && (itrg=20))
	   )) ||
	 (is18 &&
	  ((pt>=230           && (itrg=200))||
	   (pt>=130 && pt<230 && (itrg=110))||
	   (pt>=105 && pt<130 && (itrg=100))||
	   (pt>=95  && pt<105 && (itrg=90)) ||
	   (pt>=85  && pt<95  && (itrg=75)) ||
	   (pt>=60  && pt<85  && (itrg=50)) ||
	   (pt>=35  && pt<60  && (itrg=30)) ||
	   (pt>=20  && pt<35  && (itrg=20))
	   ))
	 );
    } // isQCD

    assert(itrg>0 || !pass_trig);

    // Reweight MC pileup
    if (isMC && pass_trig) {
      TH1D *hm = _pu[dataset][1]; assert(hm);
      TH1D *hd = _pu[sera][itrg]; assert(hd);
      assert(hm->GetNbinsX()==hd->GetNbinsX());
      int k = hm->FindBin(Pileup_nTrueInt);
      assert(hm->GetBinLowEdge(k)==hd->GetBinLowEdge(k));
      double nm  = hm->GetBinContent(k);
      assert(nm>0); // should never get here if hm made from fullMC
      double nd  = hd->GetBinContent(k);
      double wt = (nm>0 ? nd / nm : 0);
      w *= wt;
    }
    // Normalize data luminosity
    if (!isMC && pass_trig) {
      double lumi = _lumi[sera][itrg]; assert(lumi>0);
      w *= 1./lumi;
    }

    // Select leading jets. Just exclude photon, don't apply JetID yet
    int iJet(-1), iJet2(-1), nJets(0);
    jet.SetPtEtaPhiM(0,0,0,0);
    jet2.SetPtEtaPhiM(0,0,0,0);
    jetn.SetPtEtaPhiM(0,0,0,0);
    // Also calculate corrected type-I chsMET and HDM inputs
    corrjets.SetPtEtaPhiM(0,0,0,0);
    rawjets.SetPtEtaPhiM(0,0,0,0);
    rcjets.SetPtEtaPhiM(0,0,0,0);
    rcoffsets.SetPtEtaPhiM(0,0,0,0);
    for (unsigned int i = 0; i != nJet; ++i) {
      
      // Check that jet is not photon and pTcorr>15 GeV
      if (Jet_pt[i]>15 && (iGam==-1 || (int)i != Photon_jetIdx[iGam]) &&
	  (!isQCD || i!=iFox)) {
	
	++nJets;
	jeti.SetPtEtaPhiM(Jet_pt[i], Jet_eta[i], Jet_phi[i], Jet_mass[i]);
	
	if (iJet==-1) { // Leading jet for balance
	  iJet = i;
	  jet = jeti;
	}
	else { // Subleading jets 
	  jetn += jeti;
	  if (iJet2==-1) { // First subleading jet for alpha
	    iJet2 = i;
	    jet2 = jeti;
	  }
	}
	
	// Calculate L1RC correction
	rawjet = (1-Jet_rawFactor[i]) * jeti;
	jecl1rc->setJetPt(rawjet.Pt());
	jecl1rc->setJetEta(rawjet.Eta());
	jecl1rc->setJetA(Jet_area[i]);
	jecl1rc->setRho(fixedGridRhoFastjetAll);
	double corrl1rc = jecl1rc->getCorrection();
	rcjet = corrl1rc * rawjet;
	
	// Corrected type-I chsMET calculation
	corrjets += jeti;
	rawjets += rawjet;
	rcjets += rcjet;
	rcoffsets += (rawjet - rcjet);
      } // non-photon jet
    } // for i in nJet
    
    // Select genjet matching leading and subleading reco jet
    int iGenJet(-1), iGenJet2(-1);
    genjet.SetPtEtaPhiM(0,0,0,0);
    genjet2.SetPtEtaPhiM(0,0,0,0);
    if (isMC) {
      for (int i = 0; i != nGenJet; ++i) {
	geni.SetPtEtaPhiM(GenJet_pt[i],GenJet_eta[i],GenJet_phi[i],
			  GenJet_mass[i]);
	if (iJet!=-1 && geni.DeltaR(jet)<0.4 && iGenJet==-1) {
	  iGenJet = i;
	  genjet = geni;
	}
	else if (iJet2!=-1 && geni.DeltaR(jet2)<0.4 && iGenJet2==-1) {
	  iGenJet2 = i;
	  genjet2 = geni;
	}
      } // for i in nGenJet
    } // isMC

    // Set MET vectors
    rawmet.SetPtEtaPhiM(ChsMET_pt, 0, ChsMET_phi, 0);
    if (isQCD && iFox!=-1) rawmet += fox - gam; // fox=rawjet-PU, gam=genjet
    else rawmet += rawgam - gam; // replace PF photon with Reco photon
    met1 = -jet -gam;
    metn = -jetn;
    //corrmet = rawmet +rawjets -corrjets -rcoffsets;
    corrmet = rawmet +rcjets -corrjets; // same as above
    // Unclustered MET from rawMET by taking out all the hard stuff
    // metu = rawmet +gam +rawjets -rcoffsets;
    // metu = rawmet +gam +rcjets;
    // Or equally well, from corrMET (modulo rounding errors)
    metu = corrmet +gam +corrjets;
    met = corrmet;
    
    // Make MET transverse
    corrmet.SetPz(0);
    met.SetPz(0);
    metn.SetPz(0);
    met1.SetPz(0);
    metu.SetPz(0);
    
    // Calculate basic variables
    double ptjet = jet.Pt();
    double abseta = fabs(jet.Eta());
    double pt2 = jet2.Pt();
    double pt2min = 30;
    double bal(0), mpf(0), mpf1(0), mpfn(0), mpfu(0);
    if (ptgam>0) {
      bal = ptjet / ptgam;
      mpf = 1 + met.Vect().Dot(gam.Vect()) / (ptgam*ptgam);
      mpf1 = 1 + met1.Vect().Dot(gam.Vect()) / (ptgam*ptgam);
      mpfn = metn.Vect().Dot(gam.Vect()) / (ptgam*ptgam);
      mpfu = metu.Vect().Dot(gam.Vect()) / (ptgam*ptgam);
    }
    
    // Sanity checks for HDM inputs
    if (!(fabs(mpf1+mpfn+mpfu-mpf)<1e-4)) {
      cout << "\nHDM input error: mpf=" << mpf << " mpf1=" << mpf1
	   << " mpfn=" << mpfn << " mpfu=" << mpfu << endl;
      cout << "Difference = " << mpf1+mpfn+mpfu-mpf << endl << flush;
      //assert(false);
      cout << "Skip entry " << jentry
	   << " ("<<run<<","<<luminosityBlock<<","<<event<<")"
	   << " in file " << _filename << endl << flush;
      continue;
    }
    
    // Photon control plots
    h2ngam->Fill(ptgam, nGam, w);
    if (gen.Pt()>0 && fabs(gen.Eta()) < 1.3) {
      hgen->Fill(gen.Pt(), w);
      peffgr->Fill(gen.Pt(), iGamGen!=-1 ? 1 : 0, w);
      peffid->Fill(gen.Pt(), iGam==iGamGen ? 1 : 0, w);
    }
    if (ptgam>0 && fabs(gam.Eta()) < 1.3) {
      hgam->Fill(ptgam, w);
      if (isMC) pfake->Fill(ptgam, iGam!=iGamGen ? 1 : 0, w);
      if (isQCD) {
	bool hasorig = (iGamOrig!=-1 && gam.DeltaR(gamorig)<0.2);
	bool inwindow = (fabs(gamorig.Pt() / ptgam - 0.9) < 0.2); // [0.8,1.1]
	pfakeqcd->Fill(ptgam, hasorig ? 1 : 0, w);
	pfakeqcd2->Fill(ptgam, hasorig && inwindow ? 1 : 0, w);
	if (hasorig) {
	  h2rgamqcd->Fill(ptgam, gamorig.Pt() / ptgam, w);
	  prgamqcd->Fill(ptgam, gamorig.Pt() / ptgam, w);
	  if (inwindow) prgamqcd2->Fill(ptgam, gamorig.Pt() / ptgam, w);
	} // hasorig
      }
      if (iGam==iGamGen && gen.Pt()>0) {
	h2rgam->Fill(gen.Pt(), ptgam / gen.Pt(), w);
	prgam->Fill(gen.Pt(), ptgam / gen.Pt(), w);
	h2cgam->Fill(ptgam, gen.Pt() / ptgam, w);
	pcgam->Fill(ptgam, gen.Pt() / ptgam, w);
      }

      // Plots for photon trigger efficiencies
      if (isMC)  hgam0_mc->Fill(ptgam, w);
      if (!isMC) hgam0_data->Fill(ptgam, w);
      
      hgam0 ->Fill(ptgam, w);
      // Backup high pT
      if (HLT_Photon300_NoHE) hgam300->Fill(ptgam, w);
      if (HLT_Photon165_HE10) hgam165h->Fill(ptgam, w);
      // Main unprescaled trigger in 2018
      if (HLT_Photon200) hgam200->Fill(ptgam, w);
      // Run 1 style prescaled triggers
      if (HLT_Photon175) hgam175->Fill(ptgam, w);
      if (HLT_Photon150) hgam150->Fill(ptgam, w);
      if (HLT_Photon120) hgam120->Fill(ptgam, w);
      if (HLT_Photon90)  hgam90 ->Fill(ptgam, w);
      if (HLT_Photon75)  hgam75 ->Fill(ptgam, w);
      if (HLT_Photon50)  hgam50 ->Fill(ptgam, w);
      if (HLT_Photon36)  hgam36 ->Fill(ptgam, w);
      if (HLT_Photon33)  hgam33 ->Fill(ptgam, w);
      if (HLT_Photon30)  hgam30 ->Fill(ptgam, w);
      if (HLT_Photon22)  hgam22 ->Fill(ptgam, w);
      if (HLT_Photon20)  hgam20 ->Fill(ptgam, w);
      // 105-230 GeV intermediate range with tight triggers
      if (HLT_Photon120EB_TightID_TightIso) hgam120t->Fill(ptgam, w);
      if (HLT_Photon110EB_TightID_TightIso) hgam110t->Fill(ptgam, w);
      if (HLT_Photon100EB_TightID_TightIso) hgam100t->Fill(ptgam, w);
      // 60-105 GeV with medium triggers. NB: conflicting ID?
      if (HLT_Photon165_R9Id90_HE10_IsoM) hgam165m->Fill(ptgam, w);
      if (HLT_Photon120_R9Id90_HE10_IsoM) hgam120m->Fill(ptgam, w);
      if (HLT_Photon90_R9Id90_HE10_IsoM)  hgam90m ->Fill(ptgam, w);
      if (HLT_Photon75_R9Id90_HE10_IsoM)  hgam75m ->Fill(ptgam, w);
      if (HLT_Photon50_R9Id90_HE10_IsoM)  hgam50m ->Fill(ptgam, w);
      if (HLT_Photon36_R9Id90_HE10_IsoM)  hgam36m ->Fill(ptgam, w);
      if (HLT_Photon30_R9Id90_HE10_IsoM)  hgam30m ->Fill(ptgam, w);
      if (HLT_Photon22_R9Id90_HE10_IsoM)  hgam22m ->Fill(ptgam, w);
      // 20-60 GeV with loose triggers
      if (HLT_Photon60_HoverELoose) hgam60l->Fill(ptgam, w);
      if (HLT_Photon50_HoverELoose) hgam50l->Fill(ptgam, w);
      if (HLT_Photon40_HoverELoose) hgam40l->Fill(ptgam, w);
      if (HLT_Photon30_HoverELoose) hgam30l->Fill(ptgam, w);
      if (HLT_Photon20_HoverELoose) hgam20l->Fill(ptgam, w);
    } // barrel photon

      // Summary of combined trigger efficiencies
      if (ptgam>0 && fabs(gam.Eta())<1.3 && pass_trig) {
	if (isMC)  hgamtrig_mc->Fill(ptgam, w);
	if (!isMC) hgamtrig_data->Fill(ptgam, w);
	hgamtrig->Fill(ptgam, w); // 5 GeV bins to match hgam[trgX]
	hgamtrg->Fill(ptgam, w); // wider binning to higher pT (=hgam)
      }
      if (ptgam>=105 && fabs(gam.Eta())<1.3 && pass_trig) {
	  //HLT_Photon110EB_TightID_TightIso) {
	h2gametaphi->Fill(gam.Eta(), gam.Phi(), w);
	h2gametaphi2->Fill(gam.Eta(), gam.Phi(), w);
	h2gametaphi3->Fill(gam.Eta(), gam.Phi(), w);
	h2gametaphi4->Fill(gam.Eta(), gam.Phi(), w);
      }
      
      // Event filters for 2016 and 2017+2018 data and MC
      // UL lists are separate, but all filter recommendations looked the same
      bool pass_filt = 
	(Flag_goodVertices &&
	 Flag_globalSuperTightHalo2016Filter &&
	 Flag_HBHENoiseFilter &&
	 Flag_HBHENoiseIsoFilter &&
	 Flag_EcalDeadCellTriggerPrimitiveFilter &&
	 Flag_BadPFMuonFilter &&
	 //Flag_BadPFMuonDzFilter && // new in UL, but not in nAOD?
	 //Flag_BadChargedCandidateFilter && // not recommended
	 //Flag_globalTightHalo2016Filter && // obsolete?
	 //Flag_CSCTightHaloFilter // obsolete?
	 (is16 || Flag_ecalBadCalibFilter) && //new in UL, not for UL16
	 //(isMC || Flag_eeBadScFilter) // data only
	 Flag_eeBadScFilter // MC added 7 July 2021
	 ); // pass_filt
      
      bool pass_ngam = (nGam>=1);
      bool pass_njet = (nJets>=1);
      bool pass_gameta = (fabs(gam.Eta()) < 1.3);
      bool pass_dphi = (gam.DeltaPhi(jet) > 2.7); // pi-0.44 as in KIT Z+jet
      bool pass_jetid = (iJet!=-1 && Jet_jetId[iJet]>=4); // tightLepVeto
      bool pass_veto = true;
      if (true) { // jet veto
        int i1 = h2jv->GetXaxis()->FindBin(jet.Eta());
        int j1 = h2jv->GetYaxis()->FindBin(jet.Phi());
        if (h2jv->GetBinContent(i1,j1)>0) {
          ++_nbadevents_veto;
	  pass_veto = false;
	}
      } // jet veto
      bool pass_leak = (phoj.Pt()<0.06*ptgam);
      bool pass_basic = (pass_trig && pass_filt && pass_ngam && pass_njet &&
			 pass_gameta && pass_dphi && pass_jetid && pass_veto &&
			 pass_leak); // add pass_gameta v19 / 202111122 !
      
      // Control plots for jet response
      bool pass_bal = (fabs(1-bal)<0.7);
      bool pass_mpf = (fabs(1-mpf)<0.7);
      bool pass_jeteta = (abseta < 1.3);
      bool pass_alpha100 = (pt2 < ptgam || pt2 < pt2min);      
      bool pass_basic_ext = (pass_basic && pass_bal && pass_mpf);
      bool pass_gen = (iGenJet!=-1);
      if (pass_basic && pass_jeteta && pass_alpha100) {
	h2bal->Fill(ptgam, bal, w);
	h2mpf->Fill(ptgam, mpf, w);
	if (pass_mpf) h2balc->Fill(ptgam, bal, w);
	if (pass_bal) h2mpfc->Fill(ptgam, mpf, w);
	if (pass_mpf && pass_bal) h2balc2->Fill(ptgam, bal, w);
	if (pass_mpf && pass_bal) h2mpfc2->Fill(ptgam, mpf, w);
	if (pass_basic_ext) {

	  if (pass_gen || !isMC) {
	    h2rjet->Fill(ptgam, jet.Pt() / ptgam, w);
	    prjet->Fill(ptgam, jet.Pt() / ptgam, w);
	  }
	  if (pass_gen) {
	    h2gjet->Fill(ptgam, genjet.Pt() / ptgam, w);
	    pgjet->Fill(ptgam, genjet.Pt() / ptgam, w);
	    h2rgen->Fill(genjet.Pt(), jet.Pt() / genjet.Pt(), w);
	    prgen->Fill(genjet.Pt(), jet.Pt() / genjet.Pt(), w);
	  }

	  int flv = (isMC ? GenJet_partonFlavour[iGenJet] : 99);
	  mvar["counts"] = 1;
	  mvar["mpfchs1"] = mpf;
	  mvar["ptchs"] = bal;
	  mvar["mpf1"] = mpf1;
	  mvar["mpfn"] = mpfn;
	  mvar["mpfu"] = mpfu;
	  mvar["rho"] = fixedGridRhoFastjetAll;
	  mvar["rjet"] = (ptgam!=0 ? jet.Pt() / ptgam : 0);
	  mvar["gjet"] = (ptgam!=0 ? genjet.Pt() / ptgam : 0);
	  mvar["rgen"] = (genjet.Pt()!=0 ? jet.Pt() / genjet.Pt() : 0);

	  bool isb = (Jet_btagDeepB[iJet] > bthr);
	  bool isc = (Jet_btagDeepC[iJet] > cthr && !isb);
	  bool isq = (Jet_qgl[iJet]>=0.5 && Jet_qgl[iJet] && !isb && !isc);
	  bool isg = (Jet_qgl[iJet]>=0 && Jet_qgl[iJet]<0.5 && !isb && !isc);
	  bool isn = (!isb && !isc && !isq && !isg);
	  
	  for (int ivar = 0; ivar != nvar; ++ivar) {
	    for (int itag = 0; itag != ntag; ++itag) {
	      for (int iflv = 0; iflv != nflv; ++iflv) {

		string& svr = avar[ivar]; const char *cv = svr.c_str();
		string& stg = atag[itag]; const char *ct = stg.c_str();
		string& sfl = aflv[iflv]; const char *cf = sfl.c_str();

		if (((sfl=="i") ||
		     (sfl=="b" && abs(flv)==5) ||
		     (sfl=="c" && abs(flv)==4) ||
		     (sfl=="q" && abs(flv)<=3 && flv!=0) ||
		     (sfl=="s" && abs(flv)==3) ||
		     (sfl=="ud" && abs(flv)<=2 && flv!=0) ||
		     (sfl=="g" && flv==21) ||
		     (sfl=="n" && flv==0)) &&
		    ((stg=="i") ||
		     (stg=="b" && isb) ||
		     (stg=="c" && isc) ||
		     (stg=="q" && isq) ||
		     (stg=="g" && isg) ||
		     (stg=="n" && isn))) {

		  double x = ptgam;
		  if (svr=="rgen") x = genjet.Pt();
		  if ((svr=="rjet" || svr=="gjet") && iGenJet==-1) x = 0;
		  double var = mvar[svr];
		  TH1* h = mp[svr][stg][sfl];
		  if (!h) {
		    cout << "Missing "<<svr<<stg<<sfl<<endl<<flush;
		    assert(h);
		  }

		  if (svr=="counts")
		    ((TH1D*)h)->Fill(x, w);
		  else
		    ((TProfile*)h)->Fill(x, var, w);
		}
	      } // for iflv
	    } // for itag
	  } // for ivar

	  /*
	  double genpt = genjet.Pt();
	  double rgen = jet.Pt() / genjet.Pt();
	  double ggam = genjet.Pt() / gam.Pt();
	  if (abs(flv)==1) prgenud->Fill(genpt, rgen, w);
	  if (abs(flv)==2) prgenud->Fill(genpt, rgen, w);
	  if (abs(flv)==3) prgens->Fill(genpt, rgen, w);
	  if (abs(flv)==4) prgenc->Fill(genpt, rgen, w);
	  if (abs(flv)==5) prgenb->Fill(genpt, rgen, w);
	  if (flv==21)     prgeng->Fill(genpt, rgen, w);
	  if (flv==0)      prgeno->Fill(genpt, rgen, w);
	  if (abs(flv)==1) pgjetud->Fill(ptgam, ggam, w);
	  if (abs(flv)==2) pgjetud->Fill(ptgam, ggam, w);
	  if (abs(flv)==3) pgjets->Fill(ptgam, ggam, w);
	  if (abs(flv)==4) pgjetc->Fill(ptgam, ggam, w);
	  if (abs(flv)==5) pgjetb->Fill(ptgam, ggam, w);
	  if (flv==21)     pgjetg->Fill(ptgam, ggam, w);
	  if (flv==0)      pgjeto->Fill(ptgam, ggam, w);
	  pfud->Fill(ptgam, abs(flv)==1||abs(flv)==2 ? 1 : 0, w);
	  pfs->Fill(ptgam, abs(flv)==3 ? 1 : 0, w);
	  pfc->Fill(ptgam, abs(flv)==4 ? 1 : 0, w);
	  pfb->Fill(ptgam, abs(flv)==5 ? 1 : 0, w);
	  pfg->Fill(ptgam, flv==21 ? 1 : 0, w);
	  pfo->Fill(ptgam, flv==0 ? 1 : 0, w);
          */
	}
	if (pass_basic_ext && jet2.Pt()>0) {
	  if (iGenJet2!=-1 || !isMC) {
	    h2rjet2->Fill(ptgam, jet2.Pt() / ptgam, w);
	    prjet2->Fill(ptgam, jet2.Pt() / ptgam, w);
	  }
	  if (iGenJet2!=-1) {
	    h2gjet2->Fill(ptgam, genjet2.Pt() / ptgam, w);
	    pgjet2->Fill(ptgam, genjet2.Pt() / ptgam, w);
	    h2rgen2->Fill(genjet2.Pt(), jet2.Pt() / genjet2.Pt(), w);
	    prgen2->Fill(genjet2.Pt(), jet2.Pt() / genjet2.Pt(), w);
	  }
	}
      }

      // Control plots for trigger 
      if (ptgam>0 && pass_basic_ext && pass_jeteta && pass_alpha100) {
	//     optimal trigger edges: (20,30,(35),55,80,95,105,115,210)
	//     old bin trigger edges  (20,30,60,85,*95*,105,130,230)
	double pt = ptgam;
	double mu = Pileup_nTrueInt;
	if (isMC                             && pt>210)  hmusmc->Fill(mu, w);
	int nmax = (isMC ? 1 : 100);
	for (int i=0; i!=nmax; ++i) {
	  mu = gRandom->Gaus(Pileup_nTrueInt,TruePUrms);
	  double w1 = 0.01*w;
	  if (HLT_Photon20_HoverELoose         && pt>=20)  hmus20->Fill(mu,w1);
	  if (HLT_Photon30_HoverELoose         && pt>=30)  hmus30->Fill(mu,w1);
	  if (HLT_Photon50_R9Id90_HE10_IsoM    && pt>=55)  hmus50->Fill(mu,w1);
	  if (HLT_Photon75_R9Id90_HE10_IsoM    && pt>=80)  hmus75->Fill(mu,w1);
	  if (HLT_Photon90_R9Id90_HE10_IsoM    && pt>=95)  hmus90->Fill(mu,w1);
	  if (HLT_Photon100EB_TightID_TightIso && pt>=105) hmus100->Fill(mu,w1);
	  if (HLT_Photon110EB_TightID_TightIso && pt>=115) hmus110->Fill(mu,w1);
	  if (HLT_Photon200                    && pt>=210) hmus200->Fill(mu,w1);
	} // for i in nmax
      } // control plots for triggers

      // Control plots for photon-jet 
      if (ptgam>0 && pass_basic_ext) {

	h2phoj->Fill(ptgam, phoj.Pt()/ptgam, w);
	pphoj->Fill(ptgam, phoj.Pt()/ptgam, w);

	double footprint = (gam.DeltaPhi(phoj0)<0.4 ? 1 : -1)*phoj0.Pt()/ptgam;
	h2phoj0->Fill(ptgam, footprint, w);
	pphoj0->Fill(ptgam, footprint, w);

	if (iGam!=-1 && Photon_seedGain[iGam]==1) {
	  h2phoj1->Fill(ptgam, footprint, w);
	  pphoj1->Fill(ptgam, footprint, w);
	  if (pass_jeteta && pass_alpha100) {
	    prbal1->Fill(ptgam, bal, w);
	    prmpf1->Fill(ptgam, mpf, w);
	  }
	}
	if (iGam!=-1 && Photon_seedGain[iGam]==6) {
	  h2phoj6->Fill(ptgam, footprint, w);
	  pphoj6->Fill(ptgam, footprint, w);
	  if (pass_jeteta && pass_alpha100) {
	    prbal6->Fill(ptgam, bal, w);
	    prmpf6->Fill(ptgam, mpf, w);
	  }
	}
	if (iGam!=-1 && Photon_seedGain[iGam]==12) {
	  h2phoj12->Fill(ptgam, footprint, w);
	  pphoj12->Fill(ptgam, footprint, w);
	  if (pass_jeteta && pass_alpha100) {
	    prbal12->Fill(ptgam, bal, w);
	    prmpf12->Fill(ptgam, mpf, w);
	  }
	}

	if (pass_jeteta && pass_alpha100) {
	  prbal->Fill(ptgam, bal, w);
	  prmpf->Fill(ptgam, mpf, w);
	  prbal0->Fill(ptgam, bal, w);
	  prmpf0->Fill(ptgam, mpf, w);
	  
	  // PF composition plots
	  h2pteta->Fill(ptgam, Jet_eta[iJet], w);
	  pabseta->Fill(ptgam, fabs(Jet_eta[iJet]), w);

	  // 1D composition and response
	  pdb->Fill(ptgam, bal, w);
	  pmpf->Fill(ptgam, mpf, w);
	  pchf->Fill(ptgam, Jet_chHEF[iJet], w);
	  pnhf->Fill(ptgam, Jet_neHEF[iJet], w);
	  pnef->Fill(ptgam, Jet_neEmEF[iJet], w);
	  pcef->Fill(ptgam, Jet_chEmEF[iJet], w);
	  pmuf->Fill(ptgam, Jet_muEF[iJet], w);
	  ppuf->Fill(ptgam, Jet_chFPV0EF[iJet], w);
	  
	  // 2D composition and response
	  if (ptgam>230) {
	    double eta = Jet_eta[iJet];
	    double phi = Jet_phi[iJet];
	    p2db->Fill(eta, phi, bal, w);
	    p2mpf->Fill(eta, phi, mpf, w);
	    p2chf->Fill(eta, phi, Jet_chHEF[iJet], w);
	    p2nhf->Fill(eta, phi, Jet_neHEF[iJet], w);
	    p2nef->Fill(eta, phi, Jet_neEmEF[iJet], w);
	    p2cef->Fill(eta, phi, Jet_chEmEF[iJet], w);
	    p2muf->Fill(eta, phi, Jet_muEF[iJet], w);
	    p2puf->Fill(eta, phi, Jet_chFPV0EF[iJet], w);
	  }

	  if (isMC) {
	    if (ptgam>=105 && ptgam<230)
	      hmus->Fill(Pileup_nTrueInt, w);
	    h2mus->Fill(ptgam, Pileup_nTrueInt, w);
	  }
	  else {
	    for (int i=0; i!=100; ++i) {
	      double mu = gRandom->Gaus(Pileup_nTrueInt,TruePUrms);
	      if (ptgam>=105 && ptgam<230)
		hmus->Fill(mu, 0.01*w);
	      h2mus->Fill(ptgam, mu, 0.01*w);
	    } // for i in 100
	  } // is MC
	  //if (ptgam>=130 && ptgam<175) {
	  //if ((is16 && ptgam>175) ||
	  //  (is17 && ptgam>230) ||
	  //  (is18 && ptgam>130)) {
	  if (ptgam>230 && iGam!=-1) {
	    pgainvsmu->Fill(Pileup_nTrueInt, Photon_seedGain[iGam], w);
	    if (b_Photon_eCorr) // safety for 2016
	      pcorrvsmu->Fill(Pileup_nTrueInt, Photon_eCorr[iGam], w);
	    perrvsmu->Fill(Pileup_nTrueInt, Photon_energyErr[iGam], w);
	    phoevsmu->Fill(Pileup_nTrueInt, Photon_hoe[iGam], w);
	    pr9vsmu->Fill(Pileup_nTrueInt, Photon_r9[iGam], w);
	  }
	  if (ptgam>230) {
	    pmuvsmu->Fill(Pileup_nTrueInt, Pileup_nTrueInt, w);
	    prhovsmu->Fill(Pileup_nTrueInt, fixedGridRhoFastjetAll, w);
	    pnpvgoodvsmu->Fill(Pileup_nTrueInt, PV_npvsGood, w);
	    pnpvallvsmu->Fill(Pileup_nTrueInt, PV_npvs, w);
	  } // high pT range
	  
	  if (iGam!=-1) {
	    pgain1vspt->Fill(ptgam, Photon_seedGain[iGam]==1 ? 1 : 0, w);
	    pgain6vspt->Fill(ptgam, Photon_seedGain[iGam]==6 ? 1 : 0, w);
	    pgain12vspt->Fill(ptgam, Photon_seedGain[iGam]==12 ? 1 : 0, w);
	    pgainvspt->Fill(ptgam, Photon_seedGain[iGam], w);
	    if (b_Photon_eCorr) // safety for 2016
	      pcorrvspt->Fill(ptgam, Photon_eCorr[iGam], w);
	    perrvspt->Fill(ptgam, Photon_energyErr[iGam], w);
	    h2hoevspt->Fill(ptgam, Photon_hoe[iGam], w);
	    phoevspt->Fill(ptgam, Photon_hoe[iGam], w);
	    h2r9vspt->Fill(ptgam, Photon_r9[iGam], w);
	    pr9vspt->Fill(ptgam, Photon_r9[iGam], w);
	  }
	  pmuvspt->Fill(ptgam, Pileup_nTrueInt, w);
	  prhovspt->Fill(ptgam, fixedGridRhoFastjetAll, w);
	  pnpvgoodvspt->Fill(ptgam, PV_npvsGood, w);
	  pnpvallvspt->Fill(ptgam, PV_npvs, w);
	} // barrel
      } // basic_ext cuts
      
      // Specific event selection for alpha and eta bins
      for (unsigned int ialpha = 0; ialpha != alphas.size(); ++ialpha) {
	for (unsigned int ieta = 0; ieta != etas.size(); ++ieta) { 
	for (unsigned int ips = 0; ips != nPSWeight+1; ++ips) { 

	  double alpha = alphas[ialpha];
	  double ymin = etas[ieta].first;
	  double ymax = etas[ieta].second;
	  double wps = (ips==0 ? 1. : PSWeight[ips-1]);
	  
	  // Specific event selection
	  bool pass_alpha = (pt2 < alpha*ptgam || pt2 < pt2min);
	  bool pass_eta = (abseta >= ymin && abseta < ymax);
	  bool pass = (pass_basic_ext && pass_alpha && pass_eta);
	  
	  if (pass) {
	    
	    // Retrieve pointers to correct set of histograms/profiles
	    int ia = int(100*alpha);
	    int iy = 100*int(10*ymin) + int(10*ymax);
	    
	    // Get reference instead of pointer so can use . and not ->
	    BasicHistos *pmh = mBasicHistos[iy][ia][ips]; assert(pmh);
	    BasicHistos& mh = (*pmh); assert(mh.hn);
	    
	    // Fill histograms (h*) and profiles (p*)
	    //assert(fabs(mpf1+mpfn+mpfu-mpf)<1e-4);
	    mh.hn->Fill(ptgam);
	    mh.hxsec->Fill(ptgam, w*wps);
	    mh.prpt->Fill(ptgam, ptgam, w*wps);
	    mh.prbal->Fill(ptgam, bal, w*wps);
	    mh.prdb->Fill(ptgam, mpf1, w*wps);
	    mh.prmpf->Fill(ptgam, mpf, w*wps);
	    mh.prmpf1->Fill(ptgam, mpf1, w*wps);
	    mh.prmpfn->Fill(ptgam, mpfn, w*wps);
	    mh.prmpfu->Fill(ptgam, mpfu, w*wps);
	    mh.prho->Fill(ptgam, fixedGridRhoFastjetAll, w);
	  } // pass
	} // for ips in PSWeight
	} // for ieta in etas
      } // for ialpha in alphas
      
    } // for jentry in nentries
    cout << endl << "Finished loop, writing file." << endl << flush;
    cout << "Processed " << _nevents << " events\n";
    cout << "Skipped " << _nbadevents_json << " events due to JSON ("
	 << (100.*_nbadevents_json/_nevents) << "%) \n";
    cout << "Skipped " << _nbadevents_trigger << " events due to trigger ("
      	 << (100.*_nbadevents_trigger/_ntot) << "%) \n";
    cout << "Skipped " << _nbadevents_veto << " events due to veto ("
      	 << (100.*_nbadevents_veto/_nevents) << "%) \n";

    // Add extra plot for jet response vs photon pT
    if (isMC) {
      fout->cd("control");
      TH1D *hrgenvgen = prgen->ProjectionX("hrgenvgen");
      TH1D *hrgenvgam = prjet->ProjectionX("hrgenvgam");
      hrgenvgam->Divide(pgjet);
      TH1D *hrgen2vgen = prgen2->ProjectionX("hrgen2vgen");
      TH1D *hrgen2vgam = prjet2->ProjectionX("hrgen2vgam");
      hrgen2vgam->Divide(pgjet2);
      curdir->cd();
    }

    fout->Write();
    cout << "File written." << endl << flush;
    
    fout->Close();
    cout << "File closed." << endl << flush;

} // GamHistosFill::Loop()


// Code originally from jetphys/HistosFill.C
void GamHistosFill::PrintInfo(string info, bool printcout)
{
  //*ferr << info << endl << flush;
  if (printcout) cout << info << endl << flush;
}

// Code originally from jetphys/HistosFill.C
bool GamHistosFill::LoadJSON(string json)
{
  PrintInfo(string("Processing LoadJSON() with ") + json + " ...",true);
  ifstream file(json, ios::in);
  if (!file.is_open()) { assert(false); return false; }
  char c;
  string s, s2, s3;
  char s1[256];
  int rn(0), ls1(0), ls2(0), nrun(0), nls(0);
  file.get(c);
  if (c!='{') return false;
  while (file >> s and sscanf(s.c_str(),"\"%d\":",&rn)==1) {
    if (_gh_debug) PrintInfo(Form("\"%d\": ",rn),true);

    while (file.get(c) and c==' ') {};
    if (_gh_debug) { PrintInfo(Form("%c",c),true); assert(c=='['); }
    ++nrun;

    bool endrun = false;
    while (!endrun and file >> s >> s2 and (sscanf((s+s2).c_str(),"[%d,%d]%s",&ls1,&ls2,s1)==3 or (file >> s3 and sscanf((s+s2+s3).c_str(),"[%d,%d]%s",&ls1,&ls2,s1)==3))) {

      s2 = s1;
      if (s2=="]") { file >> s3; s2 += s3; }

      if (_gh_debug) PrintInfo(Form("[%d,%d,'%s']",ls1,ls2,s1),true);

      for (int ls = ls1; ls != ls2+1; ++ls) {
        _json[rn][ls] = 1;
        ++nls;
      }

      endrun = (s2=="]," || s2=="]}");
      if (_gh_debug and !endrun and s2!=",") { PrintInfo(string("s1: ")+s2,true); assert(s2==","); }
    } // while ls
    if (_gh_debug) PrintInfo("",true);

    if (s2=="]}") continue;
    else if (_gh_debug and s2!="],") PrintInfo(string("s2: ")+s2,true); assert(s2=="],");
  } // while run
  if (s2!="]}") { PrintInfo(string("s3: ")+s2,true); return false; }

  PrintInfo(string("Called LoadJSON() with ") + json + ":",true);
  PrintInfo(Form("Loaded %d good runs and %d good lumi sections",nrun,nls),true);
  return true;
} // LoadJSON

void GamHistosFill::LoadPU() {

  cout << endl << "GamHistosFill::LoadPU" << endl << flush;
  TDirectory *curdir = gDirectory;

  string eras[] = {"2016P8","2016APVP8","2017P8", "2018P8",
		   "2016QCD","2016APVQCD","2017QCD", "2018QCD",
		   "2016APV","2016FGH","2017","2018"};
		   //"2016BCD","2016EF","2016FGH",
		   //"2017B","2017C","2017D","2017E","2017F",
		   //"2018A","2018B","2018C","2018D"};
  const int neras = sizeof(eras)/sizeof(eras[0]);
  map<string, vector<string> > trigs;
  trigs["2016P8"].push_back("mc");
  trigs["2016APVP8"] = trigs["2017P8"] = trigs["2018P8"] = 
    trigs["2016QCD"] =  trigs["2016APVQCD"] = trigs["2017QCD"] =
    trigs["2018QCD"] = trigs["2018P8"] = trigs["2016P8"];

  trigs["2016APV"].push_back("HLT_Photon22_R9Id90_HE10_IsoM");
  trigs["2016APV"].push_back("HLT_Photon30_R9Id90_HE10_IsoM");
  trigs["2016APV"].push_back("HLT_Photon36_R9Id90_HE10_IsoM");
  trigs["2016APV"].push_back("HLT_Photon50_R9Id90_HE10_IsoM");
  trigs["2016APV"].push_back("HLT_Photon75_R9Id90_HE10_IsoM");
  trigs["2016APV"].push_back("HLT_Photon90_R9Id90_HE10_IsoM");
  trigs["2016APV"].push_back("HLT_Photon120_R9Id90_HE10_IsoM");
  trigs["2016APV"].push_back("HLT_Photon165_R9Id90_HE10_IsoM");
  //trigs["2016APV"].push_back("HLT_Photon165_HE10");
  trigs["2016APV"].push_back("HLT_Photon175");
  trigs["2016FGH"] = trigs["2016APV"];

  trigs["2017"].push_back("HLT_Photon20_HoverELoose");
  trigs["2017"].push_back("HLT_Photon30_HoverELoose");
  trigs["2017"].push_back("HLT_Photon50_R9Id90_HE10_IsoM");
  trigs["2017"].push_back("HLT_Photon75_R9Id90_HE10_IsoM");
  trigs["2017"].push_back("HLT_Photon90_R9Id90_HE10_IsoM");
  trigs["2017"].push_back("HLT_Photon120_R9Id90_HE10_IsoM");
  trigs["2017"].push_back("HLT_Photon165_R9Id90_HE10_IsoM");
  trigs["2017"].push_back("HLT_Photon200");

  trigs["2018"].push_back("HLT_Photon20_HoverELoose");
  trigs["2018"].push_back("HLT_Photon30_HoverELoose");
  trigs["2018"].push_back("HLT_Photon50_R9Id90_HE10_IsoM");
  trigs["2018"].push_back("HLT_Photon75_R9Id90_HE10_IsoM");
  trigs["2018"].push_back("HLT_Photon90_R9Id90_HE10_IsoM");
  trigs["2018"].push_back("HLT_Photon100EB_TightID_TightIso");
  trigs["2018"].push_back("HLT_Photon110EB_TightID_TightIso");
  trigs["2018"].push_back("HLT_Photon200");

  // files/pileup.root updated with tchain.C on Hefaistos
  TFile *fmc = new TFile("files/pileup.root","READ");
  assert(fmc && !fmc->IsZombie());

  for (int i = 0; i != neras; ++i) {
    string se = eras[i]; const char *ce = se.c_str();
    for (unsigned int j = 0; j != trigs[se].size(); ++j) {
      string st = trigs[se][j]; const char *ct = st.c_str();

      // Read trigger threshold from trigger name
      int itrg(0);
      if (st=="mc") itrg = 1;
      else sscanf(ct,"HLT_Photon%d*",&itrg);
      
      TFile *fdt(0);
      TH1D *h(0);
      if (st=="mc") {
	h = (TH1D*)fmc->Get(Form("pileup_%s",ce));
	assert(h);
      }
      else {
	// data files from Laura (on CERNbox)
	fdt = new TFile(Form("pileup/%s/pu_%s.root",ce,ct),"READ");
	assert(fdt && !fdt->IsZombie());
	h = (TH1D*)fdt->Get("pileup");
	assert(h);
      }
      assert(h);

      curdir->cd();
      h = (TH1D*)h->Clone(Form("pileup_%s_%s",ce,ct));
      double lumi = h->Integral();
      h->Scale(1./lumi);
      _pu[se][itrg] = h;
      _lumi[se][itrg] = lumi;

      cout << Form("%s/%s (%d): %1.3g %s\n",ce,ct,itrg,
		   lumi,st=="mc" ? "events" : "fb-1");

      if (fdt) fdt->Close();
    } // for j in trigs
  } // for i in eras
  fmc->Close();
  cout << endl << flush;

  return;
} // LoadPU
