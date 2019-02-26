#include "RecoveryDerive.h"
#include "MyCanvas.h"
#include <iostream>
#include <string>
#include <fstream>
#include <TGraphErrors.h>
#include <TCanvas.h>
#include <TStyle.h>
#include <TFile.h>
#include <TTree.h>
#include <TLeaf.h>
#include <TApplication.h>
#include <TText.h>

using namespace std;

namespace{
  extern const int MAXCYCLE;
  const int MAXCH = 64;
}

class Analizer{
public:
  Analizer(string input_filename, string output_filename, string canvas_header, const bool save_canvas=false);
  ~Analizer();

  void drawCanvas();

private:
  string m_input_filename;
  string m_output_filename;
  string m_canvas_header;
  bool m_save_canvas;
   
  //==== branch for integral_header ====//
  int m_reference_ch=7;

  //==== branch for integral_tree ====//
  int m_max_ch;
  double m_integral_mean[MAXCH][MAXCYCLE] = {};
  double m_integral_error[MAXCH][MAXCYCLE] = {};
  double m_dark_mean[MAXCH][MAXCYCLE] = {};
  double m_dark_error[MAXCH][MAXCYCLE] = {};

  //==== branch for recovery_header ====//
  string m_gain_filename = "mppc_gain.txt";
  double m_fit_range_min = 0.0;
  double m_fit_range_max = 1.0e5;

  //==== branch for recovery_tree ====//
  int m_max_cycle;
  int m_ch;
  double m_mppc_adc_gain;
  double m_onepar_tau;
  double m_onepar_slope;
  double m_twopar_tau1;
  double m_twopar_tau2;
  double m_twopar_alpha;
  double m_twopar_beta;
  double m_calib_integral_mean[MAXCYCLE] = {};
  double m_ref_integral_mean[MAXCYCLE] = {};
  double m_calib_integral_error[MAXCYCLE] = {};
  double m_ref_integral_error[MAXCYCLE] = {};
  double m_nobs_mean[MAXCYCLE] = {};
  double m_nref_mean[MAXCYCLE] = {};
  double m_nobs_error[MAXCYCLE] = {};
  double m_nref_error[MAXCYCLE] = {};

  //---- openMppcGainFile ----//
  bool openMppcGainFile();
  double m_input_mppc_gain[MAXCH] = {};

  //---- recovery time analysis ----//
  RecoveryOnepar *rec_onepar[MAXCH];
  RecoveryTwopar *rec_twopar[MAXCH];
  
  TGraphErrors *g_adc[MAXCH];
  TF1 *f_linear[MAXCH];
  TGraphErrors *g_linear_residual[MAXCH];
  TGraphErrors *g_photon[MAXCH];
  TF1 *f_recovery_onepar[MAXCH];
  TF1 *f_recovery_twopar[MAXCH];
  TGraphErrors *g_onepar_residual[MAXCH];
  TGraphErrors *g_twopar_residual[MAXCH];
  

};
  
  
Analizer::Analizer(string input_filename, string output_filename, string canvas_header, const bool save_canvas){
  
  m_input_filename = input_filename;
  m_output_filename = output_filename;
  m_canvas_header = canvas_header;
  m_save_canvas = save_canvas;

  TFile *ifile = new TFile(m_input_filename.c_str());
  TFile *ofile = new TFile(m_output_filename.c_str(), "recreate");
  TTree *integral_event_header = ((TTree*)ifile -> Get("integral_event_header")) -> CloneTree();
  TTree *integral_header = ((TTree*)ifile -> Get("integral_header")) -> CloneTree();
  TTree *integral_tree = ((TTree*)ifile -> Get("integral_tree"));
  integral_event_header -> Write();
  integral_header -> Write();
  m_max_ch = integral_tree -> GetMaximum("ch");
  m_max_cycle = integral_tree -> GetMaximum("max_cycle");
  
  for(int ch=0; ch<m_max_ch; ch++){
    for(int cy=0; cy<m_max_cycle; cy++){
      integral_tree -> GetEntry(ch);
      m_integral_mean[ch][cy] = integral_tree -> GetLeaf("integral_mean") -> GetValue(cy);
      m_integral_error[ch][cy] = integral_tree -> GetLeaf("integral_error") -> GetValue(cy);
      m_dark_mean[ch][cy] = integral_tree -> GetLeaf("dark_mean") -> GetValue(cy);
      m_dark_error[ch][cy] = integral_tree -> GetLeaf("dark_error") -> GetValue(cy);
    }
  }
  
  TTree *recovery_header = new TTree("recovery_header", "recovery_header");
  TTree *recovery_tree = new TTree("recovery_tree", "recovery_tree");
  recovery_header -> Branch("max_ch", &m_max_ch);
  recovery_header -> Branch("gain_filename", &m_gain_filename);
  recovery_header -> Fill();
  recovery_header -> Write();

  recovery_tree -> Branch("max_cycle", &m_max_cycle);
  recovery_tree -> Branch("ch", &m_ch);
  recovery_tree -> Branch("mppc_adc_gain", &m_mppc_adc_gain);
  recovery_tree -> Branch("onepar_tau", &m_onepar_tau);
  recovery_tree -> Branch("onepar_slope", &m_onepar_slope);
  recovery_tree -> Branch("twopar_tau1", &m_twopar_tau1);
  recovery_tree -> Branch("twopar_tau2", &m_twopar_tau2);
  recovery_tree -> Branch("twopar_alpha", &m_twopar_alpha);
  recovery_tree -> Branch("twopar_beta", &m_twopar_beta);
  recovery_tree -> Branch("calib_integral_mean", 
			  m_calib_integral_mean, "calib_integral_mean[max_cycle]/D");
  recovery_tree -> Branch("ref_integral_mean", 
			  m_ref_integral_mean, "ref_integral_mean[max_cycle]/D");
  recovery_tree -> Branch("calib_integral_error", 
			  m_calib_integral_error, "calib_integral_error[max_cycle]/D");
  recovery_tree -> Branch("ref_integral_error", 
			  m_ref_integral_error, "ref_integral_error[max_cycle]/D");
  recovery_tree -> Branch("nobs_mean", 
			  m_nobs_mean, "nobs_mean[max_cycle]/D");
  recovery_tree -> Branch("nref_mean", 
			  m_nref_mean, "nref_mean[max_cycle]/D");
  recovery_tree -> Branch("nobs_error", 
			  m_nobs_error, "nobs_error[max_cycle]/D");
  recovery_tree -> Branch("nref_error", 
			  m_nref_error, "nref_error[max_cycle]/D");
  
  
  if(!openMppcGainFile()){
    cout << "# Message: "
	 << "default adc gain M=16.0 is used"<< endl;
    for(int ch=0; ch<m_max_ch; ch++){
      m_input_mppc_gain[ch] = 16.0;
    }
  }
  
  for(int cy=0; cy<m_max_cycle; cy++){
   m_ref_integral_mean[cy] 
     = m_integral_mean[m_reference_ch][cy] - m_dark_mean[m_reference_ch][cy];    
   m_ref_integral_error[cy] = m_integral_error[m_reference_ch][cy];
  }

  for(int ch=0; ch<m_max_ch; ch++){
    m_ch = ch;
    cout << "ch: " << ch << endl;
    for(int cy=0; cy<m_max_cycle; cy++){
      m_calib_integral_mean[cy] = m_integral_mean[ch][cy] - m_dark_mean[ch][cy];
      m_calib_integral_error[cy] = m_integral_error[ch][cy];
    }
        
    rec_onepar[ch] = new RecoveryOnepar(m_ch, m_input_mppc_gain[ch], m_max_cycle,
					m_calib_integral_mean, m_ref_integral_mean,
					m_calib_integral_error, m_ref_integral_error,
					m_fit_range_min, m_fit_range_max);
    rec_twopar[ch] = new RecoveryTwopar(m_ch, m_input_mppc_gain[ch], m_max_cycle,
					m_calib_integral_mean, m_ref_integral_mean,
					m_calib_integral_error, m_ref_integral_error,
					m_fit_range_min, m_fit_range_max);
    
       
    m_mppc_adc_gain = m_input_mppc_gain[ch];
    m_onepar_tau = rec_onepar[ch] -> getTau();
    m_onepar_slope = rec_onepar[ch] -> getSlope();
    m_twopar_tau1 = rec_twopar[ch] -> getTau1();
    m_twopar_tau2 = rec_twopar[ch] -> getTau2();
    m_twopar_alpha = rec_twopar[ch] -> getAlpha();
    m_twopar_beta = rec_twopar[ch] -> getBeta();
    double* nobs_mean;
    double* nref_mean;
    double* nobs_error;
    double* nref_error;
    nobs_mean = rec_onepar[ch] -> getNobsMean();
    nref_mean = rec_onepar[ch] -> getNrefMean();
    nobs_error = rec_onepar[ch] -> getNobsError();
    nref_error = rec_onepar[ch] -> getNrefError();
    for(int cy=0; cy<m_max_cycle; cy++){
      m_nobs_mean[cy] = nobs_mean[cy];
      m_nref_mean[cy] = nref_mean[cy];
      m_nobs_error[cy] = nobs_error[cy];
      m_nref_error[cy] = nref_error[cy];
    }
    recovery_tree -> Fill();
  }
  
  recovery_tree -> Write();
  ofile -> Close();
}

Analizer::~Analizer(){
  for(int ch=0; ch<m_max_ch; ch++){
    delete rec_onepar[ch];
    delete rec_twopar[ch];
  }
  
}

bool Analizer::openMppcGainFile(){
  ifstream f_gain(m_gain_filename.c_str());
  if(!f_gain.good()){
    cerr << "# Error: failed to open "
	 << "\"" << m_gain_filename << "\"" << endl;
    return false;
  }
  
  for(int ch=0; ch<m_max_ch; ch++){
    f_gain >> m_input_mppc_gain[ch];
  }
  return true;
}

void Analizer::drawCanvas(){
  for(int ch=0; ch<m_max_ch; ch++){
    g_adc[ch] = rec_onepar[ch] -> getAdcPlot();
    g_adc[ch] -> SetTitle(";;");
    f_linear[ch] = rec_onepar[ch] -> getLinearFit();
    g_linear_residual[ch] = rec_onepar[ch] -> getLinearResidualPlot();
    g_photon[ch] = rec_onepar[ch] -> getPhotonPlot();
    g_photon[ch] -> SetTitle(";;");
    f_recovery_onepar[ch] = rec_onepar[ch] -> getRecoveryFit();
    f_recovery_twopar[ch] = rec_twopar[ch] -> getRecoveryFit();
    g_onepar_residual[ch] = rec_onepar[ch] -> getRecoveryResidualPlot();
    g_twopar_residual[ch] = rec_twopar[ch] -> getRecoveryResidualPlot();
  }
  //  TApplication app("app",0,0,0,0);
  
  TText *t_ch[m_max_ch];
  TText *t_x_adc = new TText(0.6, 0.01,"ADC count");
  TText *t_y_adc = new TText(0.07, 0.5, "ADC count");
  t_x_adc -> SetTextSize(0.08);
  t_y_adc -> SetTextSize(0.08);
  t_x_adc -> SetNDC(1);
  t_y_adc -> SetNDC(1);
  t_y_adc -> SetTextAngle(90);
  for(int ch=0; ch<m_max_ch; ch++){
    t_ch[ch] = new TText(0.15, 0.75, Form("ch%d",ch));
    t_ch[ch] -> SetNDC(1);
    t_ch[ch] -> SetTextSize(0.15);
  }
  MyCanvas *myc_adc = new MyCanvas("adc_plot",100,100);
  TCanvas *c_adc = myc_adc -> cloneCanvas();
  for(int ch=0; ch<m_max_ch; ch++){
    c_adc->cd(ch+1);
    g_adc[ch] -> Draw("ap");
    f_linear[ch] -> Draw("same");
    t_ch[ch] -> Draw();
    t_x_adc -> Draw();
    t_y_adc -> Draw();
  }

  TText *t_nref = new TText(0.5, 0.01,"Nref (photons/us)");
  TText *t_nobs = new TText(0.07, 0.2,"Nobs (photons/us)");
  TText *t_residual = new TText(0.07, 0.6,"Data/Fit");
  t_nref -> SetTextSize(0.08);
  t_nobs -> SetTextSize(0.08);
  t_residual -> SetTextSize(0.08);
  t_nref -> SetNDC(1);
  t_nobs -> SetNDC(1);
  t_residual -> SetNDC(1);
  t_nobs -> SetTextAngle(90);
  t_residual -> SetTextAngle(90);
  MyCanvas *myc_onepar = new MyCanvas("onepar_plot",150,150);
  TCanvas *c_onepar = myc_onepar -> cloneCanvas();
  for(int ch=0; ch<m_max_ch; ch++){
    c_onepar->cd(ch+1);
    g_photon[ch] -> Draw("ap");
    f_recovery_onepar[ch] -> Draw("same");
    t_ch[ch] -> Draw();
    t_nref -> Draw();
    t_nobs -> Draw();
  }
  MyCanvas *myc_onepar_residual = new MyCanvas("onepar_residual_plot",200,200);
  TCanvas *c_onepar_residual = myc_onepar_residual -> cloneCanvas();
  TF1 *f_base = new TF1("f_base","1",0,1.0e5);
  f_base -> SetLineStyle(7);
  f_base -> SetLineColor(1);
  for(int ch=0; ch<m_max_ch; ch++){
    c_onepar_residual->cd(ch+1)->DrawFrame(0,0.9,9000,1.1);
    g_onepar_residual[ch] -> Draw("psame");
    f_base -> Draw("same");
    t_ch[ch] -> Draw();
    t_nref -> Draw();
    t_residual -> Draw();
  }
  
  TCanvas *c_onepar_overwrite = new TCanvas("c_onepar_overwrite","c_onepar_overwrite",700,500);
  c_onepar_overwrite -> cd() -> DrawFrame(0,0.9,9000,1.1,"one parameter residual;Nref (photons/us);Data/Fit");
  f_base -> Draw("same");
  for(int ch=0; ch<m_max_ch; ch++){
    TGraphErrors *g_residual = (TGraphErrors*)g_onepar_residual[ch] -> Clone(); 
    g_residual -> SetMarkerStyle(7);
    g_residual -> SetMarkerColor(ch+1);
    g_residual -> Draw("psame");
  }
  
  if(m_save_canvas){
    c_adc -> Print(Form("%s_adc.png",m_canvas_header.c_str()));
    c_onepar -> Print(Form("%s_onepar.png",m_canvas_header.c_str()));
    c_onepar_residual -> Print(Form("%s_onepar_residual.png",m_canvas_header.c_str()));
    c_onepar_overwrite -> Print(Form("%s_onepar_overwrite.png",m_canvas_header.c_str()));
  }
  //  app.Run();

}

int main(int argc, char* argv[]){
  if(argc!=4){
    cout << "# Usage: " << argv[0] << " "
         << "[path to ana_sum_integ_***.root] [output_filename] [canvas header]"
         << endl;
    return -1;
  }

  const string input_filename = argv[1];
  const string output_filename = argv[2];
  const string canvas_header = argv[3];
  const bool save_canvas = true;

  gStyle -> SetMarkerStyle(20);

  Analizer *anal = new Analizer(input_filename, output_filename, canvas_header, save_canvas);

  anal->drawCanvas();
  
  delete anal;

}
