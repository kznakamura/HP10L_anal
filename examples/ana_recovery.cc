#include "RecoveryAnalizer.h"
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

using namespace std;

namespace{
  extern const int MAXCYCLE;
  const int MAXCH = 64;
}

class Analizer{
public:
  Analizer(string input_filename, string output_filename);
  ~Analizer();

  void drawCanvas();

private:
  string m_input_filename;
  string m_output_filename;

   
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
  RecoveryAnalizer *rec[MAXCH];
  TGraphErrors *g_adc[MAXCH];
  TF1 *f_linear[MAXCH];
  TGraphErrors *g_onepar_residual[MAXCH];
  

};
  
  
Analizer::Analizer(string input_filename, string output_filename){
  
  m_input_filename = input_filename;
  m_output_filename = output_filename;

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
    
    rec[ch] = new RecoveryAnalizer(m_ch, m_input_mppc_gain[ch], m_max_cycle,
				   m_calib_integral_mean, m_ref_integral_mean,
				   m_calib_integral_error, m_ref_integral_error,
				   m_fit_range_min, m_fit_range_max);
    
    m_mppc_adc_gain = m_input_mppc_gain[ch];
    m_onepar_tau = rec[ch] -> getOneparTau();
    m_onepar_slope = rec[ch] -> getOneparSlope();
    m_twopar_tau1 = rec[ch] -> getTwoparTau1();
    m_twopar_tau2 = rec[ch] -> getTwoparTau2();
    m_twopar_alpha = rec[ch] -> getTwoparAlpha();
    m_twopar_beta = rec[ch] -> getTwoparBeta();
    recovery_tree -> Fill();
  }
  
  recovery_tree -> Write();
  ofile -> Close();
}

Analizer::~Analizer(){
  
  
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
    g_adc[ch] = rec[ch] -> getAdcPlot();
    f_linear[ch] = rec[ch] -> getLinearFit();
    g_onepar_residual[ch] = rec[ch] -> getOneparResidual();
  }
  TApplication app("app",0,0,0,0);    
  TCanvas *c = new TCanvas("c", "c", 700, 500);
  c->cd();
  /*  g_adc[8] -> Draw("ap");
      f_linear[8] -> Draw("same");*/
  g_onepar_residual[9] -> Draw("ap");
  app.Run();
  
}

int main(int argc, char* argv[]){
  if(argc!=3){
    cout << "# Usage: " << argv[0] << " "
         << "[path to ana_sum_integ_***.root] [output_filename]"
         << endl;
    return -1;
  }

  const string input_filename = argv[1];
  const string output_filename = argv[2];

  gStyle -> SetMarkerStyle(20);

  Analizer *anal = new Analizer(input_filename, output_filename);


  anal->drawCanvas();
  
  delete anal;

}
