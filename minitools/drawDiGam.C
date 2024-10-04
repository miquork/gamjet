// Purpose: Draw DiGamHistos.Fill:/DiGam/hmgg* to find eta_t->gamma+gamma
// Date: 4 Oct 2024
// Author: miquork
#include "TFile.h"
#include "TH1D.h"
#include "TF1.h"

#include "../tdrstyle_mod22.C"

void drawDiGam() {

  setTDRStyle();
  TDirectory *curdir = gDirectory;

  TFile *f = new TFile("rootfiles/DiGamHistosFill_data_2024FG.root","READ");
  assert(f && !f->IsZombie());

  //2024F:   29.558801123      | 27.757490090 
  //2024G:  40.399886502      | 37.772631997 
  double lumi = (27.8+37.8)*1000; // 2024F+G, 1/fb->1/pb
  //double lumi = (20+30)*1000; // 2024F+G, 1/fb->1/pb

  TH1D *hmgg = (TH1D*)f->Get("DiGam/hmgg1_13"); assert(hmgg);
  //TH1D *hmgg = (TH1D*)f->Get("DiGam/hmgg1"); assert(hmgg);
  TH1D *hmggn = (TH1D*)hmgg->Clone("hmggn");
  hmggn->Rebin(2.);
  hmggn->Scale(1./lumi,"width");

  //TF1 *f1 = new TF1("f1","[0]*pow(x,[1]+[2]*log(x))",120,2000);
  TF1 *f1 = new TF1("f1","[0]*pow(x,[1]+[2]*log(0.01*x))",120,2000);
  f1->SetParameters(1e5,-5,0);
  //hmggn->Fit(f1,"QRN");
  hmggn->Fit(f1,"RN");

  double m_eta_t = 2*172.5; // GeV
  double gamma_total = 7.06; // GeV;
  double resol = 0.02*m_eta_t;
  //double resol = 0.01*m_eta_t;
  //double resol = 0.03*m_eta_t; // vertex smearing?
  double br_gg = 1.33e-4;
  double width_eta_t = sqrt(pow(gamma_total,2) + pow(resol,2));
  //TF1 *f2 = new TF1("f2","[0]*pow(x,[1]+[2]*log(x))+fabs([3])*TMath::Gaus(x,[4],[5],1)",120,2000);
  TF1 *f2 = new TF1("f2","[0]*pow(x,[1]+[2]*log(0.01*x))+fabs([3])*TMath::Gaus(x,[4],[5],1)",120,2000);
  f2->SetParameters(f1->GetParameter(0),f1->GetParameter(1),f1->GetParameter(2),0.,m_eta_t,width_eta_t);
  //f2->SetParLimits(3,0,1e10);
  f2->FixParameter(4,m_eta_t);
  f2->FixParameter(5,width_eta_t);
  //hmggn->Fit(f2,"QRN");
  hmggn->Fit(f2,"RN");

  TH1D *hmggd = (TH1D*)hmggn->Clone("hmggd");
  TH1D *hmggd2 = (TH1D*)hmggn->Clone("hmggd2");
  TH1D *hmggr = (TH1D*)hmggn->Clone("hmggr");
  TH1D *hmggr2 = (TH1D*)hmggn->Clone("hmggr2");
  double chi2(0), chi22(0);
  for (int i = 1; i != hmggd->GetNbinsX()+1; ++i) {

    double pt = hmggd->GetBinCenter(i);
    double data = hmggd->GetBinContent(i); 
    double sigma = hmggd->GetBinError(i);
    double fit = f1->Eval(pt);
    double fit2 = f2->Eval(pt);

    if (sigma!=0) {

      hmggd->SetBinContent(i, (data-fit)/sigma);
      hmggd->SetBinError(i, 0.);
      hmggd2->SetBinContent(i, (data-fit2)/sigma);
      hmggd2->SetBinError(i, 0.);

      if (pt>200. && pt<1000.) {
	chi2 += (data-fit)/sigma;
	chi22 += (data-fit2)/sigma;
      }
      
      hmggr->SetBinContent(i, (data-fit)*20. * lumi);
      hmggr->SetBinError(i, 0.);
      hmggr2->SetBinContent(i, (data-fit2)*20. * lumi);
      hmggr2->SetBinError(i, 0.);
    }
  }
  
  //TH1D *h = tdrHist("h","N_{event} / GeV",0.02,1e6,"m_{#gamma#gamma} (GeV)",50,2400);
  TH1D *h = tdrHist("h","Cross section (pb / GeV)",2e-7,1e2,"m_{#gamma#gamma} (GeV)",50,2400);
  TH1D *hd = tdrHist("hd","(Data-fit)/#sigma",-5,5,"m_{#gamma#gamma} (GeV)",50,2400);
  lumi_136TeV = Form("2024FG, %1.1f fb^{-1}",lumi/1000.);
  extraText = "Private";
  TCanvas *c1 = tdrDiCanvas("c1",h,hd,8,11);

  TLatex *tex = new TLatex();
  tex->SetNDC(); tex->SetTextSize(0.035);

  
  c1->cd(1);
  gPad->SetLogx();
  gPad->SetLogy();

  tdrDraw(hmggn,"HE",kNone,kBlack,kSolid,-1,1001,kBlue-9);

  f2->SetLineColor(kRed);
  f2->Draw("SAME");
  f1->SetLineColor(kBlue+2);
  f1->Draw("SAME");

  gPad->RedrawAxis();

  tex->DrawLatex(0.40,0.85,"Tight cut-based photon ID, R_{9}>0.94");
  //tex->DrawLatex(0.40,0.81,"|#eta_{#gamma,1}|<1.3 or |#eta_{#gamma,2}|<1.3 and p_{T,#gamma}>50 GeV");
  tex->DrawLatex(0.40,0.81,"|#eta_{#gamma,1,2}|<1.3 and p_{T,#gamma,1}>50 GeV");

  tex->DrawLatex(0.40,0.77,"f(m_{#gamma#gamma})=N#timespow(m_{#gamma#gamma},#alpha+#betalog(m_{#gamma#gamma}))+S#timesgaus");
  tex->DrawLatex(0.45,0.73,"Fit Range 120<m_{#gamma#gamma}<2000 GeV");
  tex->DrawLatex(0.45,0.69,"#chi^{2} for 200<m_{#gamma#gamma}<1000 GeV");
  tex->SetTextColor(kBlue);
  tex->DrawLatex(0.45,0.64,Form("#chi^{2}(B) = %1.1f / %d",
				f1->GetChisquare(),f1->GetNDF()));
  tex->SetTextColor(kRed);
  tex->DrawLatex(0.45,0.60,Form("#chi^{2}(S+B) = %1.1f / %d",
				f2->GetChisquare(),f2->GetNDF()));
  tex->SetTextColor(kBlack);
  tex->DrawLatex(0.45,0.56,Form("m(#eta_{t}) #equiv 2m_{t} = %1.1f GeV",m_eta_t));
  //tex->DrawLatex(0.50,0.52,Form("#sigma(#eta_{t}) #equiv 7.06 GeV #oplus 2%%#timesm(#eta_{t})"));
  //tex->DrawLatex(0.50,0.48,Form("          = %1.1f GeV",width_eta_t));
  tex->DrawLatex(0.45,0.52,Form("#sigma(#eta_{t}) #equiv 7.06 GeV #oplus 2%%#timesm(#eta_{t}) = %1.1f GeV",width_eta_t));
  //tex->DrawLatex(0.45,0.52,Form("#sigma(#eta_{t}) #equiv 7.06 GeV #oplus 1%%#timesm(#eta_{t}) = %1.1f GeV",width_eta_t));
  //tex->DrawLatex(0.45,0.52,Form("#sigma(#eta_{t}) #equiv 7.06 GeV #oplus 3%%#timesm(#eta_{t}) = %1.1f GeV",width_eta_t));
  tex->DrawLatex(0.45,0.48,Form("xsec(#eta_{t}) #equiv fidX / BR(#eta_{t}#rightarrow#gamma#gamma)"));
  //tex->DrawLatex(0.60,0.30,Form("= N/1.33#times10^{-4} = %1.1f pb",fabs(f2->GetParameter(3))/1.33e-4));
  tex->DrawLatex(0.56,0.44,Form("= S/1.33#times10^{-4} = %1.1f#pm%1.1f pb",fabs(f2->GetParameter(3))/1.33e-4,f2->GetParError(3)/1.33e-4));

  
  c1->cd(2);
  gPad->SetLogx();

  tdrDraw(hmggd,"HISTE",kNone,kBlack,kSolid,-1,1001,kBlue-9);
  tdrDraw(hmggd2,"HISTE",kNone,kRed,kSolid,-1,kNone,kBlue-9);

  gPad->RedrawAxis();

  
  c1->SaveAs("pdf/drawDiGam/drawDiGam.pdf");


  TH1D *h_2 = tdrHist("h_2","(Data - B fit) / 20 GeV",-1200,2300,"m_{#gamma#gamma} (GeV)",200,1000);
  TH1D *h_2d = tdrHist("h_2d","(Data - B+S fit) / 20 GeV",-1200,2300,"m_{#gamma#gamma} (GeV)",200,1000);
  lumi_136TeV = Form("2024FG, %1.1f fb^{-1}",lumi/1000.);
  extraText = "Private";
  TCanvas *c2 = tdrDiCanvas("c2",h_2,h_2d,8,11);

  c2->cd(1);

  //hmggr->Draw();
  tdrDraw(hmggr,"HISTE",kNone,kBlack,kSolid,-1,1001,kBlue-9);
  tdrDraw(hmggr2,"HISTE",kNone,kRed,kSolid,-1,kNone,kBlue-9);
  
  c2->cd(2);

  tdrDraw(hmggr2,"HISTE",kNone,kRed,kSolid,-1,kNone,kBlue-9);

  c2->SaveAs("pdf/drawDiGam/drawDiGam_diff.pdf");
}
