#define GamHistosFill_cxx
#include "GamHistosFill.h"
#include <TH2.h>
#include <TStyle.h>
#include <TCanvas.h>

#include "TH1D.h"
#include "TProfile.h"
#include "TLorentzVector.h"
#include "TStopwatch.h"

#include <iostream>
using namespace std;
//using namespace GamHistosFill;

// Classes to structure sets of histograms and profiles
struct BasicHistos {
  TH1D *hn;
  TProfile *prpt;
  TProfile *prbal;
  TProfile *prmpf;
  TProfile *prmpf1;
  TProfile *prmpfn;
  TProfile *prmpfu;
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
  
  if (true) { // ProcessFast
    fChain->SetBranchStatus("*",0);  // disable all branches
    
    // Baseline triggers with very high prescale except Photon200
    fChain->SetBranchStatus("HLT_Photon20",1);
    fChain->SetBranchStatus("HLT_Photon33",1);
    fChain->SetBranchStatus("HLT_Photon50",1);
    fChain->SetBranchStatus("HLT_Photon75",1);
    fChain->SetBranchStatus("HLT_Photon90",1);
    fChain->SetBranchStatus("HLT_Photon120",1);
    fChain->SetBranchStatus("HLT_Photon150",1);
    fChain->SetBranchStatus("HLT_Photon175",1);
    fChain->SetBranchStatus("HLT_Photon200",1);
    
    // Presumably backup trigger for unprescaled Photon200
    fChain->SetBranchStatus("HLT_Photon300_NoHE",1);
    
    // Triggers to recover pT=105-230 GeV range. Efficient, low prescale
    // Not active in some of 2018 data (~10%)?
    fChain->SetBranchStatus("HLT_Photon100EB_TightID_TightIso",1); // ok
    fChain->SetBranchStatus("HLT_Photon110EB_TightID_TightIso",1); // best
    fChain->SetBranchStatus("HLT_Photon120EB_TightID_TightIso",1); // backup
    
    // Triggers to recover 60-105 GeV range. However, inefficient up to high pT
    // Possibly medium HLT cuts not fully consistent with tight ID?
    fChain->SetBranchStatus("HLT_Photon50_R9Id90_HE10_IsoM",1);
    fChain->SetBranchStatus("HLT_Photon75_R9Id90_HE10_IsoM",1);
    fChain->SetBranchStatus("HLT_Photon90_R9Id90_HE10_IsoM",1);
    fChain->SetBranchStatus("HLT_Photon120_R9Id90_HE10_IsoM",1);
    fChain->SetBranchStatus("HLT_Photon165_R9Id90_HE10_IsoM",1);
    
    // Triggers to recover 30-60 GeV range. Efficient above 30 GeV
    fChain->SetBranchStatus("HLT_Photon20_HoverELoose",1);
    fChain->SetBranchStatus("HLT_Photon30_HoverELoose",1);
    
    // JSON filtering
    fChain->SetBranchStatus("run",1);
    if (!isMC) fChain->SetBranchStatus("luminosityBlock",1);
    if (!isMC) fChain->SetBranchStatus("event",1);
    
    // Event filters listing from Sami:
    // twiki.cern.ch/twiki/bin/viewauth/CMS/MissingETOptionalFiltersRun2
    // #2018_2017_data_and_MC_UL
    fChain->SetBranchStatus("Flag_goodVertices",1);
    fChain->SetBranchStatus("Flag_globalSuperTightHalo2016Filter",1);
    fChain->SetBranchStatus("Flag_HBHENoiseFilter",1);
    fChain->SetBranchStatus("Flag_HBHENoiseIsoFilter",1);
    fChain->SetBranchStatus("Flag_EcalDeadCellTriggerPrimitiveFilter",1);
    fChain->SetBranchStatus("Flag_BadPFMuonFilter",1);
    //inputList.("Flag_BadPFMuonDzFilter",1); // new in UL, but part of above?
    //inputList.("Flag_BadChargedCandidateFilter",0); // not recommended
    //inputList.("Flag_globalTightHalo2016Filter",0); // obsolete?
    //inputList.("Flag_CSCTightHaloFilter",0); // obsolete?
    //if dataset.isData:
    if (!isMC) {
      fChain->SetBranchStatus("Flag_eeBadScFilter",1); // data only
    }
    
    
    // MC weights
    if (isMC)  fChain->SetBranchStatus("genWeight",1);
    
    fChain->SetBranchStatus("fixedGridRhoFastjetAll",1);
    
    fChain->SetBranchStatus("ChsMET_pt",1);
    fChain->SetBranchStatus("ChsMET_phi",1);
    
    fChain->SetBranchStatus("nPhoton",1);
    fChain->SetBranchStatus("Photon_pt",1);
    fChain->SetBranchStatus("Photon_eta",1);
    fChain->SetBranchStatus("Photon_phi",1);
    fChain->SetBranchStatus("Photon_mass",1);
    fChain->SetBranchStatus("Photon_cutBased",1);
    fChain->SetBranchStatus("Photon_hoe",1);
    fChain->SetBranchStatus("Photon_jetIdx",1);
    
    fChain->SetBranchStatus("nJet",1);
    fChain->SetBranchStatus("Jet_pt",1);
    fChain->SetBranchStatus("Jet_eta",1);
    fChain->SetBranchStatus("Jet_phi",1);
    fChain->SetBranchStatus("Jet_mass",1);
    fChain->SetBranchStatus("Jet_rawFactor",1);
    fChain->SetBranchStatus("Jet_area",1);
    fChain->SetBranchStatus("Jet_jetId",1);
    
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
      fChain->SetBranchStatus("nGenIsolatedPhoton",1);
      fChain->SetBranchStatus("GenIsolatedPhoton_pt",1);
      fChain->SetBranchStatus("GenIsolatedPhoton_eta",1);
      fChain->SetBranchStatus("GenIsolatedPhoton_phi",1);
      fChain->SetBranchStatus("GenIsolatedPhoton_mass",1);
    }
  }
  
  // Select appropriate L1RC for type-I MET L1L2L3-RC calculation
  FactorizedJetCorrector *jecl1rc(0);
  if (dataset=="2018A")  jecl1rc = getFJC("Summer19UL18_RunA_V5_DATA_L1RC");
  if (dataset=="2018B")  jecl1rc = getFJC("Summer19UL18_RunB_V5_DATA_L1RC");
  if (dataset=="2018C")  jecl1rc = getFJC("Summer19UL18_RunC_V5_DATA_L1RC");
  if (dataset=="2018D")  jecl1rc = getFJC("Summer19UL18_RunD_V5_DATA_L1RC");
  if (dataset=="2018P8") jecl1rc = getFJC("Summer19UL18_V5_MC_L1RC");
  assert(jecl1rc);
  
  // Create histograms. Copy format from existing files from Lyon
  // Keep only histograms actually used by global fit (reprocess.C)
  TDirectory *curdir = gDirectory;
  TFile *fout = new TFile(Form("files/GamHistosFill_%s_%s.root",
			       isMC ? "mc" : "data", dataset.c_str()),
			  "RECREATE");
  assert(fout && !fout->IsZombie());
  
  double vx[] = {15, 20, 25, 30, 35, 40, 50, 60, 70, 85, 105, 130, 175, 230,
		 300, 400, 500, 600, 700, 850, 1000, 1200, 1450, 1750};
  const int nx = sizeof(vx)/sizeof(vx[0])-1;
  
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
  
  // Control plots stored in a separate directory
  fout->mkdir("control");
  fout->cd("control");
  
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

  // Plots for photon properties
  TH2D *h2ngam = new TH2D("h2ngam","",nx,vx,5,0,5);
  TH1D *hgen = new TH1D("hgen","",nx,vx);
  TH1D *hgam = new TH1D("hgam","",nx,vx);
  TProfile *peffgr = new TProfile("peffgr","",nx,vx);
  TProfile *peffid = new TProfile("peffid","",nx,vx);
  TProfile *pfake = new TProfile("pfake","",nx,vx);
  TProfile *prgam = new TProfile("prgam","",nx,vx);
  
  // Plots for photon trigger efficiencies
  // TBD: need to create these more systematically with a loop
  TH1D *hgam0_data = new TH1D("hgam0_data","",197,15,1000);
  TH1D *hgam0_mc = new TH1D("hgam0_mc","",197,15,1000);
  TH1D *hgam0 = new TH1D("hgam0","",197,15,1000);
  TH1D *hgam20 = new TH1D("hgam20","",197,15,1000);
  TH1D *hgam33 = new TH1D("hgam33","",197,15,1000);
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
  TH1D *hgam50m = new TH1D("hgam50m","",197,15,1000);
  TH1D *hgam75m = new TH1D("hgam75m","",197,15,1000);
  TH1D *hgam90m = new TH1D("hgam90m","",197,15,1000);
  TH1D *hgam120m = new TH1D("hgam120m","",197,15,1000);
  TH1D *hgam165m = new TH1D("hgam165m","",197,15,1000);
  TH1D *hgam20l = new TH1D("hgam20l","",197,15,1000);
  TH1D *hgam30l = new TH1D("hgam30l","",197,15,1000);
  TH1D *hgamtrig = new TH1D("hgamtrig","",197,15,1000);
  TH1D *hgamtrig_data = new TH1D("hgamtrig_data","",197,15,1000);
  TH1D *hgamtrig_mc = new TH1D("hgamtrig_mc","",197,15,1000);
  
  fout->cd();
  
  // Loop to create histograms and profiles
  // Match ordering to Lyon files (alpha->eta->data/MC) when creating
  // Although otherwise ordering is data/MC->eta->alpha
  map<int, map<int, BasicHistos*> > mBasicHistos;
  for (unsigned int ialpha = 0; ialpha != alphas.size(); ++ialpha) {
    for (unsigned int ieta = 0; ieta != etas.size(); ++ieta) { 
      
      // Select data/MC, alpha and eta bin
      const char *cd = dir.c_str();
      int ia = int(100*alphas[ialpha]);
      int iy1 = int(10*etas[ieta].first);
      int iy2 = int(10*etas[ieta].second);
      int iy = 100*int(iy1) + int(iy2);
      
      // Counts
      TH1D *hn = new TH1D(Form("resp_MPFchs_%s_a%d_eta%02d_%02d_RawNEvents"
			       "_data_vs_pt",cd,ia,iy1,iy2),"",nx,vx);
      
      // Response profiles
      string name = Form("resp_%%s_%s_a%d_eta%02d_%02d",cd,ia,iy1,iy2);
      const char *cname = name.c_str();
      TProfile *prpt  =  new TProfile(Form(cname,"PtGam"),"",nx,vx);
      TProfile *prbal =  new TProfile(Form(cname,"PtBalchs"),"",nx,vx);
      TProfile *prmpf =  new TProfile(Form(cname,"MPFchs"),"",nx,vx);
      TProfile *prmpf1 = new TProfile(Form(cname,"MPFR1chs"),"",nx,vx);
      TProfile *prmpfn = new TProfile(Form(cname,"MPFRnchs"),"",nx,vx);
      TProfile *prmpfu = new TProfile(Form(cname,"MpfRuchs"),"",nx,vx);
      
      // Store links to histograms and profiles into maps
      BasicHistos *pmh = new BasicHistos();
      BasicHistos& mh = (*pmh);
      mh.hn = hn;
      mh.prpt = prpt;
      mh.prbal = prbal;
      mh.prmpf = prmpf;
      mh.prmpf1 = prmpf1;
      mh.prmpfn = prmpfn;
      mh.prmpfu = prmpfu;
      mBasicHistos[iy][ia] = pmh;
    } // for ieta in etas
  } // for ialpha in alphas
  
  curdir->cd();
  
  TLorentzVector gam, gami, lhe, gen, phoj, jet, jet2, jetn;
  TLorentzVector met, met1, metn, metu, rawmet, corrmet;
  TLorentzVector jeti, corrjets, rawjet, rawjets, rcjet, rcjets, rcoffsets;
  
  //Long64_t nentries = fChain->GetEntriesFast();
  Long64_t nentries = fChain->GetEntries();
  cout << "\nStarting loop over " << dataset << " with "
       << nentries << " entries" << endl;
  
  Long64_t nbytes = 0, nb = 0;
  for (Long64_t jentry=0; jentry<nentries;jentry++) {
    Long64_t ientry = LoadTree(jentry);
    if (ientry < 0) break;
    
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

    // Safety resets for triggers that are partly missing in 2018A
    HLT_Photon100EB_TightID_TightIso =  HLT_Photon110EB_TightID_TightIso =
      HLT_Photon120EB_TightID_TightIso = kFALSE;
    
    if (true) { // Fast trigger filtering (useful for data)
      b_HLT_Photon200->GetEntry(ientry);
      b_HLT_Photon110EB_TightID_TightIso->GetEntry(ientry);
      b_HLT_Photon100EB_TightID_TightIso->GetEntry(ientry);
      b_HLT_Photon75_R9Id90_HE10_IsoM->GetEntry(ientry);
      b_HLT_Photon50_R9Id90_HE10_IsoM->GetEntry(ientry);
      b_HLT_Photon30_HoverELoose->GetEntry(ientry);
      b_HLT_Photon20_HoverELoose->GetEntry(ientry);
      
      if (!(HLT_Photon200 ||
	    HLT_Photon110EB_TightID_TightIso ||
	    HLT_Photon100EB_TightID_TightIso ||
	    HLT_Photon75_R9Id90_HE10_IsoM ||
	    HLT_Photon50_R9Id90_HE10_IsoM ||
	    HLT_Photon30_HoverELoose ||
	    HLT_Photon20_HoverELoose))
	continue;
    }

    nb = fChain->GetEntry(jentry);   nbytes += nb;
    // if (Cut(ientry) < 0) continue;

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
      if (Photon_pt[i]>15 && Photon_cutBased[i]==3 && Photon_hoe[i]<0.02148) {
	++nGam;
	if (iGam==-1) {
	  iGam = i;
	  gam = gami;
	}
      } // tight photon
    } // for i in nPhoton

    // Photon-jet: uncorrected jet minus (uncorr.) photon minus L1RC
    if (iGam!=-1 && Photon_jetIdx[iGam]!=-1) {
      unsigned int idx = Photon_jetIdx[iGam]; assert(idx<nJet);
      phoj.SetPtEtaPhiM(Jet_pt[idx], Jet_eta[idx], Jet_phi[idx], Jet_mass[idx]);
      phoj *= (1-Jet_rawFactor[idx]);
      phoj -= gam;

      // Calculate L1RC correction
      jecl1rc->setJetPt(phoj.Pt());
      jecl1rc->setJetEta(phoj.Eta());
      jecl1rc->setJetA(Jet_area[idx]);
      jecl1rc->setRho(fixedGridRhoFastjetAll);
      double corrl1rc = jecl1rc->getCorrection();
      phoj *= corrl1rc;
    }
  
    // Event weights (1 for MadGraph)
    //bool isMC = (run==1);
    assert((isMC && run==1) || (!isMC && run!=1));
    double w = (isMC ? genWeight : 1);
    
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
      if (Jet_pt[i]>15 && (iGam==-1 || (int)i != Photon_jetIdx[iGam])) {
	
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
    
    // Set MET vectors
    rawmet.SetPtEtaPhiM(ChsMET_pt, 0, ChsMET_phi, 0);
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
    double ptgam = gam.Pt();
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
      cout << "HDM input error: mpf=" << mpf << " mpf1=" << mpf1
	   << " mpfn=" << mpfn << " mpfu=" << mpfu << endl;
      cout << "Difference = " << mpf1+mpfn+mpfu-mpf << endl << flush;
      assert(false);
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
      if (iGam==iGamGen && gen.Pt()>0)
	prgam->Fill(gen.Pt(), ptgam / gen.Pt(), w);
      
      // Plots for photon trigger efficiencies
      if (isMC)  hgam0_mc->Fill(ptgam, w);
      if (!isMC) hgam0_data->Fill(ptgam, w);
      
      hgam0 ->Fill(ptgam, w);
      // Backup high pT
      if (HLT_Photon300_NoHE) hgam300->Fill(ptgam, w);
      // Main unprescaled trigger
      if (HLT_Photon200) hgam200->Fill(ptgam, w);
      // Run 1 style prescaled triggers
      if (HLT_Photon175) hgam175->Fill(ptgam, w);
      if (HLT_Photon150) hgam150->Fill(ptgam, w);
      if (HLT_Photon120) hgam120->Fill(ptgam, w);
      if (HLT_Photon90)  hgam90 ->Fill(ptgam, w);
      if (HLT_Photon75)  hgam75 ->Fill(ptgam, w);
      if (HLT_Photon50)  hgam50 ->Fill(ptgam, w);
      if (HLT_Photon33)  hgam33 ->Fill(ptgam, w);
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
      // 20-60 GeV with loose triggers
      if (HLT_Photon30_HoverELoose) hgam30l->Fill(ptgam, w);
      if (HLT_Photon20_HoverELoose) hgam20l->Fill(ptgam, w);
    } // barrel photon

    // Basic event selection. Take care to match pT bin edges
    // vx: {15, 20, 25, 30, 35, 40, 50, 60, 85, 105, 130, 175, 230, 300,
    double pt = ptgam; // shorthand to make trigger selection more readable
    bool pass_trig = 
      (//isMC ||
       // Unprescaled trigger for pT>230
       (HLT_Photon200 && pt>=230) ||
       // Run 1 style triggers with very high prescale
       //(HLT_Photon175 && pt>230  && pt<300) ||
       //(HLT_Photon150 && pt>=175 && pt<230) ||
       //(HLT_Photon120 && pt>=130 && pt<175) ||
       //(HLT_Photon90  && pt>=105 && pt<130) ||
       //(HLT_Photon75  && pt>=85  && pt<105) ||
       //(HLT_Photon50  && pt>=60  && pt<85) ||
       //(HLT_Photon33  && pt>=35  && pt<60) ||
       //(HLT_Photon20  && pt>=20  && pt<35) ||
       // Tight triggers with low prescale for 105-230 GeV
       //(HLT_Photon120EB_TightID_TightIso && pt>=130 && pt<230) ||
       (HLT_Photon110EB_TightID_TightIso && pt>=130 && pt<230) ||
       (HLT_Photon100EB_TightID_TightIso && pt>=105 && pt<130) ||
       // Medium triggers with medium prescale for 60-85
       //(HLT_Photon165_R9Id90_HE10_IsoM   && pt>=175 && pt<230) ||
       //(HLT_Photon120_R9Id90_HE10_IsoM   && pt>=130 && pt<175) ||
       //(HLT_Photon90_R9Id90_HE10_IsoM    && pt>=105 && pt<130) ||
       (HLT_Photon75_R9Id90_HE10_IsoM    && pt>=85  && pt<105) ||
       (HLT_Photon50_R9Id90_HE10_IsoM    && pt>=60  && pt<85) ||
       // Loose triggers with high prescale for 20-60 GeV
       (HLT_Photon30_HoverELoose         && pt>=35  && pt<60) ||
       (HLT_Photon20_HoverELoose         && pt>=20  && pt<35)
       );

      // Summary of combined trigger efficiencies
      if (ptgam>0 && fabs(gam.Eta())<1.3 && pass_trig) {
	if (isMC)  hgamtrig_mc->Fill(ptgam, w);
	if (!isMC) hgamtrig_data->Fill(ptgam, w);
	hgamtrig->Fill(ptgam, w);
      }
      
      // Event filters
      bool pass_filt = 
	(Flag_goodVertices &&
	 Flag_globalSuperTightHalo2016Filter &&
	 Flag_HBHENoiseFilter &&
	 Flag_HBHENoiseIsoFilter &&
	 Flag_EcalDeadCellTriggerPrimitiveFilter &&
	 Flag_BadPFMuonFilter &&
	 //Flag_BadPFMuonDzFilter && // new in UL, part of above?
	 //Flag_BadChargedCandidateFilter && // not recommended
	 //Flag_globalTightHalo2016Filter && // obsolete?
	 //Flag_CSCTightHaloFilter // obsolete?
	 (isMC || Flag_eeBadScFilter) // data only
	 );
      
      bool pass_json = true;
      bool pass_ngam = (nGam>=1);
      bool pass_njet = (nJets>=1);
      bool pass_gameta = (fabs(gam.Eta()) < 1.3);
      bool pass_dphi = (gam.DeltaPhi(jet) > 2.7);
      bool pass_jetid = (iJet!=-1 && Jet_jetId[iJet]>=4); // tightLepVeto
      bool pass_veto = true;
      bool pass_basic = (pass_trig && pass_filt && pass_json && pass_ngam &&
			 pass_njet && pass_dphi && pass_jetid && pass_veto);
      
      // Control plots for jet response
      bool pass_bal = (fabs(1-bal)<0.7);
      bool pass_mpf = (fabs(1-mpf)<0.7);
      bool pass_jeteta = (abseta < 1.3);
      bool pass_alpha100 = (pt2 < ptgam || pt2 < pt2min);      
      if (pass_basic && pass_jeteta && pass_alpha100) {
	h2bal->Fill(ptgam, bal, w);
	h2mpf->Fill(ptgam, mpf, w);
	if (pass_mpf) h2balc->Fill(ptgam, bal, w);
	if (pass_bal) h2mpfc->Fill(ptgam, mpf, w);
	if (pass_mpf && pass_bal) h2balc2->Fill(ptgam, bal, w);
	if (pass_mpf && pass_bal) h2mpfc2->Fill(ptgam, mpf, w);
      }
      bool pass_basic_ext = (pass_basic && pass_bal && pass_mpf);

      // Control plots for photon-jet 
      if (phoj.Pt()>0 && pass_basic_ext) {
	h2phoj->Fill(ptgam, phoj.Pt()/ptgam, w);
	pphoj->Fill(ptgam, phoj.Pt()/ptgam, w);
      }
      
      // Specific event selection for alpha and eta bins
      for (unsigned int ialpha = 0; ialpha != alphas.size(); ++ialpha) {
	for (unsigned int ieta = 0; ieta != etas.size(); ++ieta) { 
	  
	  double alpha = alphas[ialpha];
	  double ymin = etas[ieta].first;
	  double ymax = etas[ieta].second;
	  
	  // Specific event selection
	  bool pass_alpha = (pt2 < alpha*ptgam || pt2 < pt2min);
	  bool pass_eta = (abseta >= ymin && abseta < ymax);
	  bool pass = (pass_basic_ext && pass_alpha && pass_eta);
	  
	  if (pass) {
	    
	    // Retrieve pointers to correct set of histograms/profiles
	    int ia = int(100*alpha);
	    int iy = 100*int(10*ymin) + int(10*ymax);
	    
	    // Get reference instead of pointer so can use . and not ->
	    BasicHistos *pmh = mBasicHistos[iy][ia]; assert(pmh);
	    BasicHistos& mh = (*pmh); assert(mh.hn);
	    
	    // Fill histograms (h*) and profiles (p*)
	    mh.hn->Fill(ptgam, w);
	    mh.prpt->Fill(ptgam, ptgam, w);
	    mh.prbal->Fill(ptgam, bal, w);
	    mh.prmpf->Fill(ptgam, mpf, w);
	    mh.prmpf1->Fill(ptgam, mpf1, w);
	    mh.prmpfn->Fill(ptgam, mpfn, w);
	    mh.prmpfu->Fill(ptgam, mpfu, w);
	  } // pass
	} // for ieta in etas
      } // for ialpha in alphas
      
    } // for jentry in nentries
    cout << endl << "Finished loop, writing file." << endl << flush;
    
    fout->Write();
    cout << "File written." << endl << flush;
    
    fout->Close();
    cout << "File closed." << endl << flush;

} // GamHistosFill::Loop()
