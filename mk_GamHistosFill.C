// Purpose: Fill gamma+jet analysis histograms
// Author:  mikko.voutilainen@cern.ch
// Created: June 6, 2021
//#include "CondFormats/JetMETObjects/src/Utilities.cc"
#include "CondFormats/JetMETObjects/interface/JetCorrectorParameters.h"
#include "CondFormats/JetMETObjects/interface/SimpleJetCorrector.h"
#include "CondFormats/JetMETObjects/interface/FactorizedJetCorrector.h"

#include "CondFormats/JetMETObjects/interface/SimpleJetCorrectionUncertainty.h"
#include "CondFormats/JetMETObjects/interface/JetCorrectionUncertainty.h"

#include "GamHistosFill.h"

#include "TSystem.h"

#include <fstream>
#include <string>

#define LOCAL

#ifdef LOCAL
// Compile these libraries into *.so first with root -l -b -q mk_CondFormats.C
R__LOAD_LIBRARY(CondFormats/JetMETObjects/src/JetCorrectorParameters.cc+)
R__LOAD_LIBRARY(CondFormats/JetMETObjects/src/SimpleJetCorrector.cc+)
R__LOAD_LIBRARY(CondFormats/JetMETObjects/src/FactorizedJetCorrector.cc+)

R__LOAD_LIBRARY(CondFormats/JetMETObjects/src/SimpleJetCorrectionUncertainty.cc+)
R__LOAD_LIBRARY(CondFormats/JetMETObjects/src/JetCorrectionUncertainty.cc+)

R__LOAD_LIBRARY(GamHistosFill.C+g)
#endif

void mk_GamHistosFill(string dataset = "2018P8") {

  // Settings
  bool addData = (dataset=="2016B"  || dataset=="2016C" || dataset=="2016D" || 
		  dataset=="2016E"  || dataset=="2016F" || 
		  dataset=="2016FG" || dataset=="2016H" || 
		  dataset=="2016BCD"|| dataset=="2016EF"|| dataset=="2016FGH" ||
		  //dataset=="2016BCDEF" ||
		  dataset=="2017B" || dataset=="2017C" || dataset=="2017D" || 
		  dataset=="2017E" || dataset=="2017F" ||
		  //dataset=="2017BCDEF" || 
		  dataset=="2018A"  || dataset=="2018B" ||
		  dataset=="2018C"  || dataset=="2018D" ||
		  dataset=="2018D1" || dataset=="2018D2");
                  //dataset=="2018ABCD");
  bool addMC = (dataset=="2016P8" || dataset=="2017P8" || dataset=="2018P8");

  //cout << "Clean old shared objects and link files" << endl << flush;
  //gSystem->Exec("rm *.d");
  //gSystem->Exec("rm *.so");
  //gSystem->Exec("rm *.pcm");	

  string path = gSystem->pwd();
  /*
  gSystem->AddIncludePath(Form("-I%s",path.c_str()));
  gSystem->AddIncludePath(Form("-I%s/CondFormats/JetMETObjects/interface",path.c_str()));
  */
#ifdef GPU
  // Compile these libraries into *.so first with root -l -b -q mk_CondFormats.C
  gROOT->ProcessLine(".L CondFormats/JetMETObjects/src/Utilities.cc+");
  gROOT->ProcessLine(".L CondFormats/JetMETObjects/src/JetCorrectorParameters.cc+");
  gROOT->ProcessLine(".L CondFormats/JetMETObjects/src/SimpleJetCorrector.cc+");
  gROOT->ProcessLine(".L CondFormats/JetMETObjects/src/FactorizedJetCorrector.cc+");
  
  gROOT->ProcessLine(".L CondFormats/JetMETObjects/src/SimpleJetCorrectionUncertainty.cc+");
  gROOT->ProcessLine(".L CondFormats/JetMETObjects/src/JetCorrectionUncertainty.cc+");

  gROOT->ProcessLine(".L GamHistosFill.C+g");
#endif


  TChain *c = new TChain("Events");
  //c->AddFile("files/mc-ht-200to400.root");
  //c->AddFile("files/mc-ht-600toInf.root");
  //c->AddFile("files/mc-pt-15to6000.root");
  //c->AddFile("files/data-2018a.root");
  
  // Automatically figure out where we are running the job
  bool runGPU = (path=="/media/storage/gamjet");
  bool runLocal = (path=="/Users/voutila/Dropbox/Cern/gamjet");
  if (!runLocal) assert(runGPU);
  
  if (addData) {
    ifstream fin(runLocal ? Form("dataFiles_local_%s.txt",dataset.c_str()) : 
		 Form("dataFiles_Run%s.txt",dataset.c_str()), ios::in);
    string filename;
    cout << "Chaining data files:" << endl << flush;
    int nFiles(0), nFilesMax(827);//9999);
    while (fin >> filename && nFiles<nFilesMax) {
      ++nFiles;
      c->AddFile(filename.c_str());
    }
    cout << "Chained " << nFiles <<  " files" << endl << flush;
    
    GamHistosFill filler(c,0,dataset);
    filler.Loop();
  }
  
  if (addMC) {
    ifstream fin(runLocal ? "mcFiles_local.txt" :
		 Form("mcFiles_%s.txt",dataset.c_str()), ios::in);
    string filename;
    cout << "Chaining MC files:" << endl << flush;
    int nFiles(0), nFilesMax(8);
    while (fin >> filename && nFiles<nFilesMax) {
      ++nFiles;
      c->AddFile(filename.c_str());
    }
    cout << "Chained " << nFiles <<  " files" << endl << flush;
  
    GamHistosFill filler(c,1,dataset);
    filler.Loop();
  }
}
