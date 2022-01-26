// Purpose: parse prescale information from brilcalc out.csv file
#include "TFile.h"

#include <map>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>

// Store information in [run,LS,itrg] map
map<int, map<int, map<int, int> > > _prescale;

// Load presacles.txt pre-made with parsePrescaleCSV
int loadPrescaleTXT() {

  // Open file
  cout << "::loadPrescaleTXT(): pileup/prescales.txt" << flush;
  ifstream fin("pileup/prescales.txt");
  assert(fin.is_open());
  
  // Read in header
  const int nmax = 512;
  char cline[nmax];
  fin.getline(cline,nmax);

  // Parse triggers from header
  stringstream sh(cline);
  char crun[nmax], cls[nmax];
  sh >> crun >> cls;
  assert(string(crun)=="Run");
  assert(string(cls)=="LS");
  int itrig(0);
  vector<int> vtrig;
  while (sh >> itrig) {
    vtrig.push_back(itrig);
  }

  // Parse prescales from table
  int nls(0);
  while (fin.getline(cline,nmax)) {
  stringstream ss(cline);
    int run, ls, itrig(0), idx(0), pre(0);
    ss >> run >> ls;
    while (ss >> pre) {
      assert(idx<vtrig.size());
      itrig = vtrig[idx]; ++idx;
      _prescale[run][ls][itrig] = pre;
    }
    ++nls;
    if (nls%100000==0) cout << "." << flush;
  }
  cout << endl;
  
  cout << "  Read in " << nls << " lumisections in "
       << _prescale.size() << " runs for "
       << vtrig.size() << " triggers" << endl;

  return nls;
} // loadPrescaleTXT

// Parse .csv into prescales.txt
void parsePrescaleCSV() {

  bool debug = true;
  cout << "parsePrescaleCVS()" << endl;

  int nls = loadPrescaleTXT();
  if (nls>0) return; // pre-made file was available already

  string years[] = {"2016APV", "2016FGH", "2017", "2018"};
  const int nyears = sizeof(years)/sizeof(years[0]);
  string trigs[] = 
    {"200",
     "175",
     "20_HoverELoose",
     "30_HoverELoose",
     "22_R9Id90_HE10_IsoM",
     "30_R9Id90_HE10_IsoM",
     "36_R9Id90_HE10_IsoM",
     "50_R9Id90_HE10_IsoM",
     "75_R9Id90_HE10_IsoM",
     "90_R9Id90_HE10_IsoM",
     "120_R9Id90_HE10_IsoM",
     "165_R9Id90_HE10_IsoM",
     //"165_HE10",
     "100EB_TightID_TightIso",
     "110EB_TightID_TightIso",
    };
  const int ntrigs = sizeof(trigs)/sizeof(trigs[0]);
  set<int> itrigs;
  map<string, int> ref;
  ref["2016APV"] = 175;
  ref["2016FGH"] = 175;
  ref["2017"] = 200;
  ref["2018"] = 200;

  // Read in effective recorded luminosity
  map<int, map<int, map<int, double> > > lum;

  const int nmax = 512;
  char cline[nmax];
  int run, store, ls, ls2, itrg;//, month, day, year, hour, min, sec;
  double del(0), rec(0), pu(0);
  char trig[nmax], trg[nmax], source[nmax], date[nmax], time[nmax];
  int icnt(0);
  
  for (int iyear = 0; iyear != nyears; ++iyear) {
    for (int itrig = 0; itrig != ntrigs; ++itrig) {
    
      // Open file
      //string filename = "pileup/2018/HLT_Photon200_out.csv";
      string filename = Form("pileup/%s/HLT_Photon%s_out.csv",
			     years[iyear].c_str(),trigs[itrig].c_str());
      cout << "Opening " << filename << "...";
      ifstream fin(filename.c_str(), ios::in);
      if (fin.is_open()) cout << "ok" << endl;
      //else { cout << "failure" << endl; return; }
      else { cout << "skip" << endl; continue; }

      // Read header information and check that it is ok
      fin.getline(cline, nmax);
      if (debug && icnt<10) cout << "Read: '" << cline << "'" << endl;
      fin.getline(cline, nmax);
      if (debug && icnt<10) cout << "Read: '" << cline << "'" << endl;
      assert(string(cline)=="#run:fill,ls,time,hltpath,delivered(/fb),recorded(/fb),avgpu,source");

      while (fin.getline(cline,nmax)) {
	/*
	  int n = sscanf(cline,"%d:%d,%d:%d,%d/%d/%d %d:%d:%d,%[^,]s,%lf,%lf%lf,%s",
	  &run,&store,&ls1,&ls2, &month,&day,&year, &hour,&min,&sec,
	  trig, &del,&rec,&pu, source);
	  if (debug && ++icnt<10)
	  cout << run<<" : "<<store<<", "<<ls1<<":"<<ls2<<", "
	  << day << "/" << month << "/" << year << " "
	  << hour << ":" << min << ":" << sec << ", "
	   << trig << ": "
	   << del << ", " << rec << ", " << pu << ", " << source
	   << "(read "<<n<<" inputs)" << endl;
	*/
	
	int n = sscanf(cline,"%d:%d,%d:%d,%s %[^,],%[^,],%lf,%lf,%lf,%s",
		       &run,&store,&ls,&ls2, date, time,
		       trig, &del,&rec,&pu, source);
	int m = sscanf(trig,"HLT_Photon%d%s",&itrg,trg);
	if (debug && !(rec>0) && ++icnt<10)
	  cout << run<<":"<<store<<","<<ls<<":"<<ls2<<", "
	       << date << "  " << time << ", "<< trig << ": "
	       << del << ", " << rec << ", " << pu << ", " << source
	       << " (read "<<n<<" inputs);" //<< endl;
	   << " itrg=" << itrg << endl;
	itrigs.insert(itrg);
	assert(ls2==ls);
	//assert(rec>0);
	lum[run][ls][itrg] = rec;
	double lumref = lum[run][ls][ref[years[iyear]]];
	//assert(lumref>0);
	if (rec>0) {
	  assert(lumref>0);
	  _prescale[run][ls][itrg] = int(lumref / rec + 0.5);
	}
	else if (itrg == years[iyear])
	  _prescale[run][ls][itrg] = 1;
	else
	  _prescale[run][ls][itrg] = 0;
      } // while
    } // for itrig in trigs
  } // for iyear in years

  ofstream fout("pileup/prescales.txt",ios::out);
  typedef map<int, map<int, map<int, int> > >::iterator IT;
  typedef map<int, map<int, int> >::iterator JT;
  //typedef map<int, int>::const_iterator KT;
  typedef set<int>::const_iterator KT;
  for (IT it = _prescale.begin(); it != _prescale.end(); ++it) {
    for (JT jt = it->second.begin(); jt != it->second.end(); ++jt) {
      
      // Print header
      if (it == _prescale.begin() && jt == it->second.begin()) {
	fout << "Run      LS";
	//for (KT kt = jt- >second.begin(); kt != jt->second.end(); ++kt) {
	//fout << Form(" %4d",kt->first);
	//}
	//for (int itrig = 0; itrig != ntrigs; ++itrig) {
	for (KT kt = itrigs.begin(); kt != itrigs.end(); ++kt) {
	  if      (*kt<30) fout << Form(" %5d",*kt);
	  else if (*kt>90) fout << Form(" %3d",*kt);
	  else             fout << Form(" %4d",*kt);
	} // for itrig
	fout << endl;
      }

      // Print prescale list
      fout << Form("%6d %4d",it->first,jt->first);
      //for (KT kt = jt->second.begin(); kt != jt->second.end(); ++kt) {
      //fout << Form(" %4d",kt->second);
      //} // for kt
      //for (int itrig = 0; itrig != ntrigs; ++itrig) {
      //fout << Form(" %4d",jt->second[trigs[itrig]]);
      for (KT kt = itrigs.begin(); kt != itrigs.end(); ++kt) {
	if      (*kt<30) fout << Form(" %5d",jt->second[*kt]);
	else if (*kt>90) fout << Form(" %3d",jt->second[*kt]);
	else             fout << Form(" %4d",jt->second[*kt]);
      } // for itrig
      fout << endl;
    } // for jt
  } // for it in _prescales

} // parsePrescaleCSV
